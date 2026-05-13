mod runtime;

fn main(){
    println!("Zeno Engine Core started");

    runtime::run_engine_loop();

    println!("Zeno Engine Core shutdown");
}