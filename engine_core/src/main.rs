mod runtime;

use runtime::EngineRuntime;

fn main() {
    println!("Zeno Engine Core started");

    let mut runtime = EngineRuntime::new();
    match runtime.run() {
        Ok(()) => {}
        Err(err) => eprintln!("Runtime error: {}", err),
    }

    println!("Zeno Engine Core shutdown");
}
