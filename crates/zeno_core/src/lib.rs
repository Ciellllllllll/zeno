//! Internal Rust engine core for ZENO.
//!
//! This crate owns the high-level runtime lifecycle and frame loop. Native
//! platform, rendering, and ABI concerns are introduced in later phases.

use std::thread;
use std::time::{Duration, Instant};

/// Version string for the engine core crate.
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

const DEFAULT_TARGET_FPS: f64 = 60.0;

/// Returns the public engine name used in documentation and diagnostics.
pub fn engine_name() -> &'static str {
    "ZENO Engine"
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
    fn target_frame_duration(&self) -> Option<Duration> {
        if self.target_fps.is_finite() && self.target_fps > 0.0 {
            Some(Duration::from_secs_f64(1.0 / self.target_fps))
        } else {
            None
        }
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
}

impl EngineRuntime {
    /// Creates a running runtime from the provided configuration.
    pub fn new(config: EngineConfig) -> Self {
        Self {
            config,
            state: EngineRuntimeState::Running,
            frame_count: 0,
            last_frame_time: Instant::now(),
        }
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
    pub fn run(&mut self) {
        while self.is_running() {
            self.stop_if_test_limit_reached();
            if !self.is_running() {
                break;
            }

            self.step_frame();
        }

        self.state = EngineRuntimeState::Stopped;
    }

    /// Advances the runtime by one frame if it is running.
    pub fn step_frame(&mut self) -> Option<EngineFrameInfo> {
        if !self.is_running() {
            return None;
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

        self.frame_count += 1;
        self.run_frame_step(frame_info);
        self.pace_frame(frame_start);
        self.stop_if_test_limit_reached();

        Some(frame_info)
    }

    fn run_frame_step(&mut self, _frame_info: EngineFrameInfo) {
        // Later phases will orchestrate game update/render work here.
    }

    fn pace_frame(&self, frame_start: Instant) {
        let Some(target_frame_duration) = self.config.target_frame_duration() else {
            return;
        };

        let elapsed = frame_start.elapsed();
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
        let runtime = EngineRuntime::new(EngineConfig::default());

        assert!(runtime.is_running());
        assert_eq!(runtime.state(), EngineRuntimeState::Running);
        assert_eq!(runtime.frame_count(), 0);
    }

    #[test]
    fn shutdown_request_changes_state() {
        let mut runtime = EngineRuntime::new(EngineConfig::default());

        runtime.request_shutdown();

        assert!(!runtime.is_running());
        assert_eq!(runtime.state(), EngineRuntimeState::ShutdownRequested);
    }

    #[test]
    fn test_limited_run_exits() {
        let mut runtime = EngineRuntime::new(EngineConfig {
            target_fps: 0.0,
            max_test_frames: Some(3),
        });

        runtime.run();

        assert_eq!(runtime.frame_count(), 3);
        assert_eq!(runtime.state(), EngineRuntimeState::Stopped);
    }

    #[test]
    fn zero_frame_test_limited_run_exits_without_stepping() {
        let mut runtime = EngineRuntime::new(EngineConfig {
            target_fps: 0.0,
            max_test_frames: Some(0),
        });

        runtime.run();

        assert_eq!(runtime.frame_count(), 0);
        assert_eq!(runtime.state(), EngineRuntimeState::Stopped);
    }

    #[test]
    fn frame_index_increments() {
        let mut runtime = EngineRuntime::new(EngineConfig {
            target_fps: 0.0,
            max_test_frames: None,
        });

        let first = runtime.step_frame().expect("first frame should run");
        let second = runtime.step_frame().expect("second frame should run");

        assert_eq!(first.frame_index, 0);
        assert_eq!(second.frame_index, 1);
        assert_eq!(runtime.frame_count(), 2);
    }

    #[test]
    fn delta_time_is_non_negative() {
        let mut runtime = EngineRuntime::new(EngineConfig {
            target_fps: 0.0,
            max_test_frames: None,
        });

        let frame = runtime.step_frame().expect("frame should run");

        assert!(frame.delta_time_seconds >= 0.0);
    }
}
