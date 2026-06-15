//! Internal Rust engine core for ZENO.
//!
//! Phase 01 keeps this crate intentionally small. Runtime behavior is added in
//! later phases after the repository layout is stable.

/// Version string for the engine core crate.
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

/// Returns the public engine name used in documentation and diagnostics.
pub fn engine_name() -> &'static str {
    "ZENO Engine"
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn exposes_engine_identity() {
        assert_eq!(engine_name(), "ZENO Engine");
        assert_eq!(VERSION, "0.1.0");
    }
}
