use std::time::Instant;
use std::time::Duration;
use std::thread::sleep;

fn main(){
    println!("Zeno Engine Core started");

    run_engine_loop();

    println!("Zeno Engine Core shutdown");
}


fn run_engine_loop(){
    println!("Engine loop started");

    let target_frame_time = Duration::from_secs_f64(1.0 / 60.0);

    let mut previous_frame_time = Instant::now();
    for n in 1..61{
        let frame_start_time = Instant::now();

        let delta_time_duration = frame_start_time - previous_frame_time;

        //Convert Duration to seconds as f64
        //as_secs_f64()を使ってDuration型を秒単位の小数に変換
        let delta_time = delta_time_duration.as_secs_f64();

        println!("Frame {} update, delta_time = {}", n ,delta_time);
        previous_frame_time = frame_start_time;

        let frame_end_time = Instant::now();
        let frame_work_time = frame_end_time - frame_start_time;

        if frame_work_time < target_frame_time{
            let sleep_time = target_frame_time - frame_work_time;
            sleep(sleep_time);
        }
    }

    println!("Engine loop finished");
}