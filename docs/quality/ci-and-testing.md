# Quality, CI, and Testing

This page records the Phase44 quality baseline. It is intentionally small: the goal is a stable public portfolio workflow, not a full production policy.

## CMake Build and CTest

Use the canonical Debug preset for local C++ work:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug-test
```

`windows-msvc-debug-test` is the headless CTest preset used by CI. It excludes tests whose names contain `window`, `sample`, or `manual`.

The full Debug test preset still exists:

```powershell
ctest --preset windows-msvc-debug
```

That preset may run window-capable smoke tests and should be treated as a local/manual validation path.

## GoogleTest

GoogleTest is introduced only under `tests/cpp` through CMake `FetchContent`.

- Test target: `zeno_cpp_smoke_tests`
- CTest entries: `ZenoCppSmoke.*`
- GoogleTest is not included from public SDK headers, native headers, samples, or `zeno_sdk_cpp`.

## Visual Studio 2022

Open the repository folder or the CMake project and select the existing `windows-msvc-debug` configure/build preset. The same target graph is used by CLI, VS Code, and CI.

The clang-tidy option is off by default. Visual Studio's own analysis integrations may be used manually, but the repository CMake graph does not force clang-tidy during normal builds.

## VS Code

Use the CMake Tools extension with the existing `windows-msvc-debug` preset. The default workflow is the same as CLI:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug-test
```

`.editorconfig` and `.clang-format` are provided so VS Code and Codex edits produce fewer whitespace-only diffs.

## GitHub Actions CI

The `CI` workflow runs on `develop` and `main` for push and pull request events.

It contains:

- `Rust checks`: `cargo fmt --all -- --check`, `cargo clippy --all-targets --all-features -- -D warnings`, and `cargo test --all-targets --all-features` when `Cargo.toml` exists.
- `Windows MSVC CMake`: builds the Rust ABI artifact when `Cargo.toml` exists, then runs `cmake --preset windows-msvc-debug`, `cmake --build --preset windows-msvc-debug`, and `ctest --preset windows-msvc-debug-test`.

The older `Windows CI` workflow remains present for the broader repository baseline scripts.

## Dependabot

Dependabot targets `develop` and runs weekly in the `Asia/Tokyo` timezone.

Configured ecosystems:

- `github-actions`
- `cargo`
- `rust-toolchain`

GitHub Actions and Cargo updates are grouped to reduce pull request noise. Pull request limits are set in `.github/dependabot.yml`.

## CodeQL

The `CodeQL` workflow analyzes C/C++ only:

- Language: `c-cpp`
- Build mode: `manual`
- Configure preset: `windows-msvc-debug`
- Build preset: `windows-msvc-debug`

Rust is not included in CodeQL for this phase. Rust remains covered by the CI Rust checks.

## clang-format

`.clang-format` defines the C/C++ formatting baseline for new and touched files. Do not run repository-wide reformatting as part of normal feature work. Format only files you intentionally edit.

## EditorConfig

`.editorconfig` defines UTF-8, final newlines, LF as the preferred line ending, and indentation rules for C/C++, CMake, Rust, Markdown, YAML, and JSON.

Do not perform a repository-wide line-ending conversion. Existing files should keep their current line endings unless they are intentionally edited for a scoped change.

## clang-tidy

clang-tidy is available through an explicit CMake option:

```powershell
cmake --preset windows-msvc-debug -D ZENO_ENABLE_CLANG_TIDY=ON
```

The option defaults to `OFF`. When it is `ON`, CMake searches for `clang-tidy` on `PATH`. If it cannot be found, configure fails with a clear error.

Normal builds do not set `CMAKE_CXX_CLANG_TIDY`.

## Not Included

Phase44 intentionally does not add:

- vcpkg manifest
- Conan
- AddressSanitizer
- Doxygen
- Google Benchmark
- fuzzing
- release/deploy automation
- automatic merge automation
