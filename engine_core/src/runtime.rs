use std::fmt;
use std::thread::sleep;
use std::time::Duration;
use std::time::Instant;

const TARGET_FPS: f64 = 60.0;
const TEST_FRAME_COUNT: usize = 60;

pub struct EngineRuntime {
    target_frame_time: Duration,
    previous_frame_time: Instant,
    is_running: bool,
}

pub struct EngineFrameInfo {
    frame_index: usize,
    delta_time: f64,
}

#[derive(Debug)]
pub struct EngineRuntimeError {}

impl fmt::Display for EngineRuntimeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // Formatterに文字列を書き込む
        write!(f, "Engine runtime error")
    }
}

impl EngineRuntime {
    pub fn new() -> Self {
        Self {
            target_frame_time: Duration::from_secs_f64(1.0 / TARGET_FPS),
            previous_frame_time: Instant::now(),
            is_running: true,
        }
    }

    fn frame_timing_method(&mut self) -> (Instant, f64) {
        let frame_start_time = Instant::now();

        let delta_time_duration = frame_start_time - self.previous_frame_time;

        //as_secs_f64()を使ってDuration型を秒単位の小数に変換
        let delta_time = delta_time_duration.as_secs_f64();
        self.previous_frame_time = frame_start_time;

        return (frame_start_time, delta_time);
    }

    fn frame_pacing_method(&self, frame_start_time: Instant) {
        let frame_end_time = Instant::now();
        let frame_work_time = frame_end_time - frame_start_time;
        if frame_work_time < self.target_frame_time {
            let sleep_time = self.target_frame_time - frame_work_time;
            sleep(sleep_time);
        }
    }

    pub fn run(&mut self) -> Result<(), EngineRuntimeError> {
        println!("Engine loop started");
        let mut frame_index = 1;
        while self.is_running && frame_index <= TEST_FRAME_COUNT {
            let (frame_start_time, delta_time) = self.frame_timing_method();
            let frame_info = EngineFrameInfo {
                frame_index,
                delta_time,
            };
            println!(
                "Frame {} update, delta_time = {}",
                frame_info.frame_index, frame_info.delta_time
            );
            self.frame_pacing_method(frame_start_time);
            frame_index += 1;
        }
        self.request_shutdown();
        println!("Engine loop finished");
        return Ok(());
    }

    fn request_shutdown(&mut self) {
        self.is_running = false;
    }
}
