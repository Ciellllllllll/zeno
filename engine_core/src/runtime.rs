use std::thread::sleep;
use std::time::Duration;
use std::time::Instant;

const TARGET_FPS: f64 = 60.0;
const TEST_FRAME_COUNT: usize = 60;

pub struct EngineRuntime {
    target_frame_time: Duration,
    previous_frame_time: Instant,
}

impl EngineRuntime {
    pub fn new() -> Self {
        Self {
            target_frame_time: Duration::from_secs_f64(1.0 / TARGET_FPS),
            previous_frame_time: Instant::now(),
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

    pub fn run(&mut self) {
        println!("Engine loop started");

        for n in 1..=TEST_FRAME_COUNT {
            let (frame_start_time, delta_time) = self.frame_timing_method();
            println!("Frame {} update, delta_time = {}", n, delta_time);
            self.frame_pacing_method(frame_start_time);
        }

        println!("Engine loop finished");
    }
}
