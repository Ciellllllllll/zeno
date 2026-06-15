//! C ABI boundary for ZENO.
//!
//! This crate exposes handle-based C-compatible functions and keeps Rust engine
//! internals behind an internal registry.

use std::collections::HashMap;
use std::ptr;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Mutex, OnceLock};

use zeno_core::{EngineConfig, EngineError, EngineFrameInfo, EngineRuntime};

const ZEN_ENGINE_CONFIG_API_VERSION: u32 = 1;
const ZEN_ENGINE_CONFIG_SIZE: u32 = std::mem::size_of::<ZenEngineConfig>() as u32;
const ZEN_ENGINE_HANDLE_INVALID: u64 = 0;
const ZEN_MAX_TEST_FRAMES_UNLIMITED: u64 = u64::MAX;

static NEXT_ENGINE_HANDLE: AtomicU64 = AtomicU64::new(1);
static ENGINE_REGISTRY: OnceLock<Mutex<HashMap<u64, EngineRuntime>>> = OnceLock::new();

/// C-compatible result code returned by fallible ABI functions.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ZenResultCode {
    /// Operation completed successfully.
    Ok = 0,
    /// Caller supplied a null pointer, invalid handle, or invalid configuration.
    InvalidArgument = 1,
    /// Requested object has not been initialized or no longer exists.
    NotInitialized = 2,
    /// Requested object is already initialized.
    AlreadyInitialized = 3,
    /// Native backend failure.
    BackendError = 4,
    /// Unexpected engine failure.
    InternalError = 100,
}

/// Opaque engine runtime handle.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ZenEngineHandle {
    /// Non-zero registry ID. Zero is always invalid.
    pub value: u64,
}

/// Engine runtime configuration passed through the C ABI.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ZenEngineConfig {
    /// Size of this struct in bytes. Must be `sizeof(ZenEngineConfig)`.
    pub size: u32,
    /// ABI config version. Must be `ZEN_ENGINE_CONFIG_API_VERSION`.
    pub api_version: u32,
    /// Desired target frame rate. Must be finite and greater than zero.
    pub target_fps: f64,
    /// Maximum test frames, or `ZEN_MAX_TEST_FRAMES_UNLIMITED` for no limit.
    pub max_test_frames: u64,
}

impl Default for ZenEngineConfig {
    fn default() -> Self {
        Self {
            size: ZEN_ENGINE_CONFIG_SIZE,
            api_version: ZEN_ENGINE_CONFIG_API_VERSION,
            target_fps: 60.0,
            max_test_frames: ZEN_MAX_TEST_FRAMES_UNLIMITED,
        }
    }
}

/// Returns the engine name exposed by the core crate.
pub fn engine_name() -> &'static str {
    zeno_core::engine_name()
}

/// Returns a default ABI config value for Rust-side tests and tools.
pub fn default_zen_engine_config() -> ZenEngineConfig {
    ZenEngineConfig::default()
}

/// Creates an engine runtime and writes its non-zero handle to `out_engine`.
///
/// Ownership: the caller owns the returned handle and must pass it to
/// `zen_engine_destroy` exactly once when done.
#[no_mangle]
pub extern "C" fn zen_engine_create(
    config: *const ZenEngineConfig,
    out_engine: *mut ZenEngineHandle,
) -> ZenResultCode {
    if config.is_null() || out_engine.is_null() {
        return ZenResultCode::InvalidArgument;
    }

    // SAFETY: null was checked above, and the function only copies the POD value.
    let config = unsafe { ptr::read(config) };
    let Ok(core_config) = convert_config(config) else {
        return ZenResultCode::InvalidArgument;
    };

    let runtime = match EngineRuntime::new(core_config) {
        Ok(runtime) => runtime,
        Err(error) => return map_engine_error(&error),
    };

    let Some(handle) = allocate_handle() else {
        return ZenResultCode::InternalError;
    };
    let mut registry = registry()
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner());
    if registry.insert(handle, runtime).is_some() {
        return ZenResultCode::AlreadyInitialized;
    }

    // SAFETY: null was checked above, and the caller owns this out pointer for the call.
    unsafe {
        ptr::write(out_engine, ZenEngineHandle { value: handle });
    }

    ZenResultCode::Ok
}

/// Destroys an engine runtime handle.
///
/// Ownership: after success, the handle value is invalid and must not be used again.
#[no_mangle]
pub extern "C" fn zen_engine_destroy(engine: ZenEngineHandle) -> ZenResultCode {
    if engine.value == ZEN_ENGINE_HANDLE_INVALID {
        return ZenResultCode::InvalidArgument;
    }

    let mut registry = registry()
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner());
    if registry.remove(&engine.value).is_some() {
        ZenResultCode::Ok
    } else {
        ZenResultCode::NotInitialized
    }
}

/// Advances the engine runtime by one frame.
#[no_mangle]
pub extern "C" fn zen_engine_step(engine: ZenEngineHandle) -> ZenResultCode {
    with_runtime(engine, |runtime| runtime.step_frame().map(|_| ()))
}

/// Advances the engine runtime by one frame and optionally writes frame data.
#[no_mangle]
pub extern "C" fn zen_engine_step_frame(
    engine: ZenEngineHandle,
    out_frame_info: *mut ZenEngineFrameInfo,
) -> ZenResultCode {
    with_runtime(engine, |runtime| {
        let frame_info = runtime.step_frame()?;
        if !out_frame_info.is_null() {
            // SAFETY: null was checked above, and the caller owns this out pointer for the call.
            unsafe {
                ptr::write(out_frame_info, frame_info.into());
            }
        }

        Ok(())
    })
}

/// Requests runtime shutdown. The runtime remains owned by the caller until destroyed.
#[no_mangle]
pub extern "C" fn zen_engine_request_shutdown(engine: ZenEngineHandle) -> ZenResultCode {
    with_runtime(engine, |runtime| {
        runtime.request_shutdown();
        Ok(())
    })
}

/// Returns a static string for a result code.
#[no_mangle]
pub extern "C" fn zen_result_to_string(code: u32) -> *const std::ffi::c_char {
    match code {
        0 => c"ok".as_ptr(),
        1 => c"invalid argument".as_ptr(),
        2 => c"not initialized".as_ptr(),
        3 => c"already initialized".as_ptr(),
        4 => c"backend error".as_ptr(),
        100 => c"internal error".as_ptr(),
        _ => c"unknown result code".as_ptr(),
    }
}

/// C-compatible frame information returned by `zen_engine_step_frame`.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ZenEngineFrameInfo {
    /// Zero-based frame index.
    pub frame_index: u64,
    /// Seconds elapsed since the previous frame step.
    pub delta_time_seconds: f64,
}

impl From<EngineFrameInfo> for ZenEngineFrameInfo {
    fn from(value: EngineFrameInfo) -> Self {
        Self {
            frame_index: value.frame_index,
            delta_time_seconds: value.delta_time_seconds,
        }
    }
}

fn registry() -> &'static Mutex<HashMap<u64, EngineRuntime>> {
    ENGINE_REGISTRY.get_or_init(|| Mutex::new(HashMap::new()))
}

fn allocate_handle() -> Option<u64> {
    NEXT_ENGINE_HANDLE
        .fetch_update(Ordering::Relaxed, Ordering::Relaxed, |current| {
            let next = current.checked_add(1)?;
            if next == ZEN_ENGINE_HANDLE_INVALID {
                None
            } else {
                Some(next)
            }
        })
        .ok()
}

fn convert_config(config: ZenEngineConfig) -> Result<EngineConfig, ZenResultCode> {
    if config.size != ZEN_ENGINE_CONFIG_SIZE {
        return Err(ZenResultCode::InvalidArgument);
    }

    if config.api_version != ZEN_ENGINE_CONFIG_API_VERSION {
        return Err(ZenResultCode::InvalidArgument);
    }

    let max_test_frames = if config.max_test_frames == ZEN_MAX_TEST_FRAMES_UNLIMITED {
        None
    } else {
        Some(usize::try_from(config.max_test_frames).map_err(|_| ZenResultCode::InvalidArgument)?)
    };

    let engine_config = EngineConfig {
        target_fps: config.target_fps,
        max_test_frames,
    };
    engine_config
        .validate()
        .map_err(|error| map_engine_error(&error))?;

    Ok(engine_config)
}

fn with_runtime(
    engine: ZenEngineHandle,
    action: impl FnOnce(&mut EngineRuntime) -> Result<(), EngineError>,
) -> ZenResultCode {
    if engine.value == ZEN_ENGINE_HANDLE_INVALID {
        return ZenResultCode::InvalidArgument;
    }

    let mut registry = registry()
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner());
    let Some(runtime) = registry.get_mut(&engine.value) else {
        return ZenResultCode::NotInitialized;
    };

    match action(runtime) {
        Ok(()) => ZenResultCode::Ok,
        Err(error) => map_engine_error(&error),
    }
}

fn map_engine_error(error: &EngineError) -> ZenResultCode {
    match error {
        EngineError::InvalidConfig(_) => ZenResultCode::InvalidArgument,
        EngineError::RuntimeAlreadyRunning => ZenResultCode::AlreadyInitialized,
        EngineError::RuntimeNotRunning => ZenResultCode::NotInitialized,
        EngineError::BackendUnavailable => ZenResultCode::BackendError,
        EngineError::Internal(_) => ZenResultCode::InternalError,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CStr;

    #[test]
    fn forwards_engine_identity() {
        assert_eq!(engine_name(), "ZENO Engine");
    }

    #[test]
    fn default_config_converts_to_core_config() {
        let config = ZenEngineConfig {
            target_fps: 120.0,
            max_test_frames: 3,
            ..ZenEngineConfig::default()
        };

        let core_config = convert_config(config).expect("config should convert");

        assert_eq!(core_config.target_fps, 120.0);
        assert_eq!(core_config.max_test_frames, Some(3));
    }

    #[test]
    fn unlimited_frame_sentinel_converts_to_none() {
        let core_config =
            convert_config(ZenEngineConfig::default()).expect("config should convert");

        assert_eq!(core_config.max_test_frames, None);
    }

    #[test]
    fn create_rejects_null_config_pointer() {
        let mut handle = ZenEngineHandle { value: 999 };

        let result = zen_engine_create(ptr::null(), &mut handle);

        assert_eq!(result, ZenResultCode::InvalidArgument);
        assert_eq!(handle.value, 999);
    }

    #[test]
    fn create_rejects_null_output_pointer() {
        let config = ZenEngineConfig::default();

        let result = zen_engine_create(&config, ptr::null_mut());

        assert_eq!(result, ZenResultCode::InvalidArgument);
    }

    #[test]
    fn create_rejects_invalid_config_size() {
        let config = ZenEngineConfig {
            size: 0,
            ..ZenEngineConfig::default()
        };
        let mut handle = ZenEngineHandle { value: 0 };

        let result = zen_engine_create(&config, &mut handle);

        assert_eq!(result, ZenResultCode::InvalidArgument);
        assert_eq!(handle.value, 0);
    }

    #[test]
    fn invalid_handle_returns_invalid_argument() {
        let result = zen_engine_step(ZenEngineHandle { value: 0 });

        assert_eq!(result, ZenResultCode::InvalidArgument);
    }

    #[test]
    fn create_step_and_destroy_lifecycle_succeeds() {
        let config = ZenEngineConfig {
            target_fps: 1_000_000.0,
            max_test_frames: 2,
            ..ZenEngineConfig::default()
        };
        let mut handle = ZenEngineHandle { value: 0 };

        assert_eq!(zen_engine_create(&config, &mut handle), ZenResultCode::Ok);
        assert_ne!(handle.value, 0);

        let mut frame_info = ZenEngineFrameInfo {
            frame_index: u64::MAX,
            delta_time_seconds: -1.0,
        };
        assert_eq!(
            zen_engine_step_frame(handle, &mut frame_info),
            ZenResultCode::Ok
        );
        assert_eq!(frame_info.frame_index, 0);
        assert!(frame_info.delta_time_seconds >= 0.0);

        assert_eq!(zen_engine_destroy(handle), ZenResultCode::Ok);
    }

    #[test]
    fn double_destroy_returns_not_initialized() {
        let config = ZenEngineConfig::default();
        let mut handle = ZenEngineHandle { value: 0 };

        assert_eq!(zen_engine_create(&config, &mut handle), ZenResultCode::Ok);
        assert_eq!(zen_engine_destroy(handle), ZenResultCode::Ok);
        assert_eq!(zen_engine_destroy(handle), ZenResultCode::NotInitialized);
    }

    #[test]
    fn shutdown_request_stops_future_steps() {
        let config = ZenEngineConfig::default();
        let mut handle = ZenEngineHandle { value: 0 };

        assert_eq!(zen_engine_create(&config, &mut handle), ZenResultCode::Ok);
        assert_eq!(zen_engine_request_shutdown(handle), ZenResultCode::Ok);
        assert_eq!(zen_engine_step(handle), ZenResultCode::NotInitialized);
        assert_eq!(zen_engine_destroy(handle), ZenResultCode::Ok);
    }

    #[test]
    fn result_to_string_returns_static_message() {
        let message = zen_result_to_string(ZenResultCode::InvalidArgument as u32);
        assert!(!message.is_null());

        // SAFETY: zen_result_to_string returns a pointer to a static nul-terminated string.
        let message = unsafe { CStr::from_ptr(message) };

        assert_eq!(message.to_str().unwrap(), "invalid argument");
    }

    #[test]
    fn result_to_string_handles_unknown_code() {
        let message = zen_result_to_string(999);
        assert!(!message.is_null());

        // SAFETY: zen_result_to_string returns a pointer to a static nul-terminated string.
        let message = unsafe { CStr::from_ptr(message) };

        assert_eq!(message.to_str().unwrap(), "unknown result code");
    }
}
