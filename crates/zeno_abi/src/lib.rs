//! ABI boundary crate for ZENO.
//!
//! Phase 01 only establishes the crate. Full exported C ABI functions belong
//! to Phase 04.

/// Returns the engine name exposed by the core crate.
pub fn engine_name() -> &'static str {
    zeno_core::engine_name()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn forwards_engine_identity() {
        assert_eq!(engine_name(), "ZENO Engine");
    }
}
