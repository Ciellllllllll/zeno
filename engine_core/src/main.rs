mod runtime;
use runtime::EngineRuntime;

fn main() {
    println!("Zeno Engine Core started");

    let mut runtime = EngineRuntime::new();
    runtime.run();

    println!("Zeno Engine Core shutdown");
}
