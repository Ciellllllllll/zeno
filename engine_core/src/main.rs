use std::time::Instant;

fn main(){
    println!("Zeno Engine Core started");

    println!("Engine loop started");

    let mut previous_frame_time = Instant::now();
    for n in 1..61{
        let current_frame_time = Instant::now();

        let delta_time_duration = current_frame_time - previous_frame_time;

        //Convert Duration to seconds as f64
        //as_secs_f64()を使ってDuration型を秒単位の少数に変換
        let delta_time = delta_time_duration.as_secs_f64();

        println!("Frame {} update, delta_time = {}", n ,delta_time);
        previous_frame_time = current_frame_time;
    }

    println!("Engine loop finished");

    println!("Zeno Engine Core shutdown");
}