# Dynamic Module Sample

This sample demonstrates the first Windows DLL-based game module path.

`zeno_dynamic_sample_module` exports the C ABI entry point named by
`ZEN_GAME_MODULE_ENTRY_POINT`. The exported function fills a
`ZenGameModuleDescriptor` containing only descriptor size, API version,
`ZenResultCode` returns, function pointers, and reserved opaque pointers.

`zeno_dynamic_module_sample` loads that DLL through `zeno::DynamicGameModule`,
validates the module API version, calls the lifecycle functions, and unloads
the DLL. The smoke executable is headless and does not replace the existing
static-linked sample game path.

The Phase 37 module ABI intentionally does not expose `zeno::GameContext`, SDK
classes, STL types, Win32 handles, DirectX types, exceptions, templates, or Rust
internals. It is not hot reload or scripting.
