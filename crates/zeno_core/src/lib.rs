//! Internal Rust engine core for ZENO.
//!
//! This crate owns the high-level runtime lifecycle and frame loop. Native
//! platform, rendering, and ABI concerns are introduced in later phases.

use std::error::Error;
use std::fmt;
use std::thread;
use std::time::{Duration, Instant};

/// Version string for the engine core crate.
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

const DEFAULT_TARGET_FPS: f64 = 60.0;

/// Returns the public engine name used in documentation and diagnostics.
pub fn engine_name() -> &'static str {
    "ZENO Engine"
}

/// Result type used by the Rust engine core.
pub type EngineResult<T> = Result<T, EngineError>;

/// High-level error category used to map internal failures to future ABI result codes.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EngineErrorCategory {
    /// Caller supplied invalid configuration or arguments.
    InvalidConfig,
    /// Runtime or backend was already initialized.
    AlreadyInitialized,
    /// Runtime or backend was not initialized.
    NotInitialized,
    /// Native backend is unavailable or failed.
    Backend,
    /// Unexpected internal engine failure.
    Internal,
}

/// Error type used inside the Rust engine core.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum EngineError {
    /// Runtime configuration is invalid.
    InvalidConfig(String),
    /// Runtime is already running.
    RuntimeAlreadyRunning,
    /// Runtime is not running.
    RuntimeNotRunning,
    /// Native backend is unavailable.
    BackendUnavailable,
    /// Unexpected internal engine failure.
    Internal(String),
}

impl EngineError {
    /// Returns the stable category this error should map to in ABI code later.
    pub fn category(&self) -> EngineErrorCategory {
        match self {
            Self::InvalidConfig(_) => EngineErrorCategory::InvalidConfig,
            Self::RuntimeAlreadyRunning => EngineErrorCategory::AlreadyInitialized,
            Self::RuntimeNotRunning => EngineErrorCategory::NotInitialized,
            Self::BackendUnavailable => EngineErrorCategory::Backend,
            Self::Internal(_) => EngineErrorCategory::Internal,
        }
    }
}

impl fmt::Display for EngineError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidConfig(message) => write!(formatter, "invalid engine config: {message}"),
            Self::RuntimeAlreadyRunning => write!(formatter, "engine runtime is already running"),
            Self::RuntimeNotRunning => write!(formatter, "engine runtime is not running"),
            Self::BackendUnavailable => write!(formatter, "native backend is unavailable"),
            Self::Internal(message) => write!(formatter, "internal engine error: {message}"),
        }
    }
}

impl Error for EngineError {}

/// Logging severity used by the engine core's minimal logging wrapper.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EngineLogLevel {
    /// Useful runtime details.
    Info,
    /// Recoverable issue that should be visible during development.
    Warning,
    /// Failure path that prevented requested work from completing.
    Error,
}

impl fmt::Display for EngineLogLevel {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Info => formatter.write_str("INFO"),
            Self::Warning => formatter.write_str("WARN"),
            Self::Error => formatter.write_str("ERROR"),
        }
    }
}

/// Emits a minimal engine log message.
pub fn log_engine_event(level: EngineLogLevel, message: impl AsRef<str>) {
    eprintln!("[ZENO][{level}] {}", message.as_ref());
}

/// Engine runtime configuration.
#[derive(Debug, Clone, PartialEq)]
pub struct EngineConfig {
    /// Desired frame rate used for frame pacing.
    pub target_fps: f64,
    /// Optional frame limit for deterministic test or smoke-test runs.
    pub max_test_frames: Option<usize>,
}

impl Default for EngineConfig {
    fn default() -> Self {
        Self {
            target_fps: DEFAULT_TARGET_FPS,
            max_test_frames: None,
        }
    }
}

impl EngineConfig {
    /// Validates that the config can be used to create a runtime.
    pub fn validate(&self) -> EngineResult<()> {
        if !self.target_fps.is_finite() || self.target_fps <= 0.0 {
            return Err(EngineError::InvalidConfig(format!(
                "target_fps must be finite and greater than zero, got {}",
                self.target_fps
            )));
        }

        Ok(())
    }

    fn target_frame_duration(&self) -> Duration {
        Duration::from_secs_f64(1.0 / self.target_fps)
    }
}

/// Information produced for one completed engine frame.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct EngineFrameInfo {
    /// Zero-based frame index.
    pub frame_index: u64,
    /// Seconds elapsed since the previous frame step.
    pub delta_time_seconds: f64,
}

#[derive(Debug, Clone, Copy, PartialEq)]
struct ActiveFrame {
    started_at: Instant,
    info: EngineFrameInfo,
}

/// Current runtime lifecycle state.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EngineRuntimeState {
    /// The runtime should continue stepping frames.
    Running,
    /// Shutdown was requested and the frame loop should stop.
    ShutdownRequested,
    /// The frame loop has exited.
    Stopped,
}

/// High-level Rust engine runtime.
#[derive(Debug)]
pub struct EngineRuntime {
    config: EngineConfig,
    state: EngineRuntimeState,
    frame_count: u64,
    last_frame_time: Instant,
    active_frame: Option<ActiveFrame>,
}

impl EngineRuntime {
    /// Creates a running runtime from the provided configuration.
    pub fn new(config: EngineConfig) -> EngineResult<Self> {
        if let Err(error) = config.validate() {
            log_engine_event(EngineLogLevel::Error, error.to_string());
            return Err(error);
        }

        log_engine_event(EngineLogLevel::Info, "engine runtime initialized");

        Ok(Self {
            config,
            state: EngineRuntimeState::Running,
            frame_count: 0,
            last_frame_time: Instant::now(),
            active_frame: None,
        })
    }

    /// Returns the runtime configuration.
    pub fn config(&self) -> &EngineConfig {
        &self.config
    }

    /// Returns the current runtime state.
    pub fn state(&self) -> EngineRuntimeState {
        self.state
    }

    /// Returns whether the runtime is still running.
    pub fn is_running(&self) -> bool {
        self.state == EngineRuntimeState::Running
    }

    /// Returns how many frames have completed.
    pub fn frame_count(&self) -> u64 {
        self.frame_count
    }

    /// Requests shutdown of the runtime.
    pub fn request_shutdown(&mut self) {
        if self.state == EngineRuntimeState::Running {
            self.state = EngineRuntimeState::ShutdownRequested;
        }
    }

    /// Runs the frame loop until shutdown or the configured test frame limit.
    pub fn run(&mut self) -> EngineResult<()> {
        if !self.is_running() {
            log_engine_event(
                EngineLogLevel::Error,
                "cannot run engine runtime because it is not running",
            );
            return Err(EngineError::RuntimeNotRunning);
        }

        log_engine_event(EngineLogLevel::Info, "engine runtime frame loop started");

        while self.is_running() {
            self.stop_if_test_limit_reached();
            if !self.is_running() {
                break;
            }

            let _frame_info = self.begin_frame()?;
            self.end_frame()?;
        }

        self.state = EngineRuntimeState::Stopped;
        log_engine_event(EngineLogLevel::Info, "engine runtime stopped");
        Ok(())
    }

    /// Advances the runtime by one frame if it is running.
    pub fn step_frame(&mut self) -> EngineResult<EngineFrameInfo> {
        let frame_info = self.begin_frame()?;
        self.end_frame()?;

        Ok(frame_info)
    }

    /// Starts a frame and returns frame timing information for update/render work.
    pub fn begin_frame(&mut self) -> EngineResult<EngineFrameInfo> {
        if !self.is_running() {
            log_engine_event(
                EngineLogLevel::Error,
                "cannot begin engine frame because runtime is not running",
            );
            return Err(EngineError::RuntimeNotRunning);
        }

        if self.active_frame.is_some() {
            return Err(EngineError::Internal(
                "cannot begin a new engine frame before ending the active frame".to_string(),
            ));
        }

        let frame_start = Instant::now();
        let delta_time_seconds = frame_start
            .saturating_duration_since(self.last_frame_time)
            .as_secs_f64();
        self.last_frame_time = frame_start;
        let frame_info = EngineFrameInfo {
            frame_index: self.frame_count,
            delta_time_seconds,
        };
        self.active_frame = Some(ActiveFrame {
            started_at: frame_start,
            info: frame_info,
        });

        Ok(frame_info)
    }

    /// Ends the active frame, applies frame pacing, and updates frame-limit state.
    pub fn end_frame(&mut self) -> EngineResult<()> {
        let Some(active_frame) = self.active_frame.take() else {
            return Err(EngineError::Internal(
                "cannot end engine frame because no frame is active".to_string(),
            ));
        };

        self.frame_count += 1;
        self.run_frame_step(active_frame.info);
        self.pace_frame(active_frame.started_at);
        self.stop_if_test_limit_reached();

        Ok(())
    }

    fn run_frame_step(&mut self, _frame_info: EngineFrameInfo) {
        // Later phases will orchestrate game update/render work here.
    }

    fn pace_frame(&self, frame_start: Instant) {
        let elapsed = frame_start.elapsed();
        let target_frame_duration = self.config.target_frame_duration();
        if elapsed < target_frame_duration {
            thread::sleep(target_frame_duration - elapsed);
        }
    }

    fn stop_if_test_limit_reached(&mut self) {
        let Some(max_test_frames) = self.config.max_test_frames else {
            return;
        };

        if self.frame_count >= max_test_frames as u64 {
            self.request_shutdown();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn exposes_engine_identity() {
        assert_eq!(engine_name(), "ZENO Engine");
        assert_eq!(VERSION, "0.1.0");
    }

    #[test]
    fn default_config_uses_60_fps_without_frame_limit() {
        let config = EngineConfig::default();

        assert_eq!(config.target_fps, 60.0);
        assert_eq!(config.max_test_frames, None);
    }

    #[test]
    fn runtime_starts_in_running_state() {
        let runtime = EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        assert!(runtime.is_running());
        assert_eq!(runtime.state(), EngineRuntimeState::Running);
        assert_eq!(runtime.frame_count(), 0);
    }

    #[test]
    fn shutdown_request_changes_state() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        runtime.request_shutdown();

        assert!(!runtime.is_running());
        assert_eq!(runtime.state(), EngineRuntimeState::ShutdownRequested);
    }

    #[test]
    fn test_limited_run_exits() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(Some(3))).expect("runtime should be valid");

        runtime.run().expect("runtime should complete");

        assert_eq!(runtime.frame_count(), 3);
        assert_eq!(runtime.state(), EngineRuntimeState::Stopped);
    }

    #[test]
    fn zero_frame_test_limited_run_exits_without_stepping() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(Some(0))).expect("runtime should be valid");

        runtime.run().expect("runtime should complete");

        assert_eq!(runtime.frame_count(), 0);
        assert_eq!(runtime.state(), EngineRuntimeState::Stopped);
    }

    #[test]
    fn frame_index_increments() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        let first = runtime.step_frame().expect("first frame should run");
        let second = runtime.step_frame().expect("second frame should run");

        assert_eq!(first.frame_index, 0);
        assert_eq!(second.frame_index, 1);
        assert_eq!(runtime.frame_count(), 2);
    }

    #[test]
    fn delta_time_is_non_negative() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        let frame = runtime.step_frame().expect("frame should run");

        assert!(frame.delta_time_seconds >= 0.0);
    }

    #[test]
    fn invalid_config_rejects_non_positive_target_fps() {
        let config = EngineConfig {
            target_fps: 0.0,
            max_test_frames: None,
        };

        let error = config.validate().expect_err("config should be invalid");

        assert_eq!(error.category(), EngineErrorCategory::InvalidConfig);
    }

    #[test]
    fn invalid_config_rejects_non_finite_target_fps() {
        let config = EngineConfig {
            target_fps: f64::INFINITY,
            max_test_frames: None,
        };

        let error = EngineRuntime::new(config).expect_err("runtime creation should fail");

        assert_eq!(error.category(), EngineErrorCategory::InvalidConfig);
    }

    #[test]
    fn error_formatting_includes_actionable_message() {
        let error = EngineError::InvalidConfig("target_fps must be positive".to_string());

        assert_eq!(
            error.to_string(),
            "invalid engine config: target_fps must be positive"
        );
    }

    #[test]
    fn stopped_runtime_step_returns_error() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(Some(0))).expect("runtime should be valid");

        runtime.run().expect("runtime should complete");
        let error = runtime
            .step_frame()
            .expect_err("stopped runtime should not step");

        assert_eq!(error, EngineError::RuntimeNotRunning);
        assert_eq!(error.category(), EngineErrorCategory::NotInitialized);
    }

    #[test]
    fn begin_frame_computes_delta_without_incrementing_frame_count() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        let frame = runtime.begin_frame().expect("frame should begin");

        assert_eq!(frame.frame_index, 0);
        assert!(frame.delta_time_seconds >= 0.0);
        assert_eq!(runtime.frame_count(), 0);

        runtime.end_frame().expect("frame should end");
        assert_eq!(runtime.frame_count(), 1);
    }

    #[test]
    fn end_frame_without_active_frame_returns_error() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        let error = runtime
            .end_frame()
            .expect_err("end frame without begin should fail");

        assert_eq!(error.category(), EngineErrorCategory::Internal);
    }

    #[test]
    fn nested_begin_frame_returns_error() {
        let mut runtime =
            EngineRuntime::new(fast_test_config(None)).expect("runtime should be valid");

        runtime.begin_frame().expect("first begin should succeed");
        let error = runtime.begin_frame().expect_err("second begin should fail");

        assert_eq!(error.category(), EngineErrorCategory::Internal);
        runtime.end_frame().expect("active frame should still end");
    }

    fn fast_test_config(max_test_frames: Option<usize>) -> EngineConfig {
        EngineConfig {
            target_fps: 1_000_000.0,
            max_test_frames,
        }
    }
}
