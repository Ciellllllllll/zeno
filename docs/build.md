# Build Guide

## Overview
Zeno uses Rust for the engine core and C++ for the native backend, SDK, and GameModule.
The first build environment targets Windows with Visual Studio 2022, MSVC, CMake, and the Rust stable toolchain.

## Supported Environment
The first supported environment is:
- Windows 10 or Windows 11
- Visual Studio 2022
- MSVC toolchain
- Windows SDK
- Rust stable toolchain

Linux, macOS, Vulkan, DirectX 12, and cross-platform builds are not included in the first milestone.

## Required Tools
Required tools:
- Visual Studio 2022
- Desktop development with C++
- MSVC v143
- Windows SDK
- CMake
- Rust stable
- Cargo
- VS Code
- rust-analyzer
- CMake Tools

## Repository Layout

The initial repository layout is:

zeno/
  engine_core/          Rust engine core
  engine_backend_dx11/  C++ DirectX 11 native backend
  engine_api/           C ABI headers shared between Rust and C++
  engine_cpp_sdk/       C++ wrapper SDK for game code
  samples/              Sample games built on Zeno
  docs/                 Project documentation
  tools/                Helper tools and scripts

The layout separates the Rust engine core, C++ native backend, C ABI layer, C++ SDK, and sample game code.
This keeps each responsibility clear and makes the project easier to explain as a portfolio.

## Rust Build Policy
The Rust engine core is built with Cargo.
Rust owns the high-level engine runtime, game loop, module loading, and engine-side lifecycle control.

## C++ Build Policy
C++ components are built with MSVC and CMake.
The C++ side owns the native backend, DirectX 11 renderer, C++ SDK, and GameModule projects.

## GameModule Build Policy
The GameModule is built as a dynamic library.
The Rust engine core loads the GameModule DLL at runtime and resolves required exported functions.

## Output Directory Policy

For MVP 0.1, the engine executable and GameModule DLL should be placed in the same runtime output directory.

The expected output layout is:

build/
  bin/
    Debug/
      zeno.exe
      aim_training_game.dll
    Release/
      zeno.exe
      aim_training_game.dll

Keeping the executable and GameModule DLL in the same directory makes runtime loading simple.
The first MVP should avoid complex search paths, asset packaging, or installer-style output layouts.

Later versions may introduce a more structured runtime directory, but MVP 0.1 should prioritize simplicity and debuggability.

## Debug and Release Builds
MVP 0.1 prioritizes Debug builds.
Release builds should be supported later, but initial development focuses on correctness, logging, and debuggability.

## VS Code Usage
VS Code is used as the main editor for Rust, C++, documentation, and repository-wide editing.

## Visual Studio 2022 Usage
Visual Studio 2022 is used for MSVC-based C++ builds, native debugging, DirectX 11 backend debugging, and CMake project inspection.

## MVP 0.1 Build Scope
MVP 0.1 build setup includes:
- Build Rust engine_core executable
- Build C++ GameModule DLL
- Place executable and DLL in the same runtime directory
- Run the engine executable on Windows
- Load the GameModule DLL at runtime

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# ビルドガイド

## 概要
Zeno は、エンジンコアに Rust、ネイティブバックエンド、SDK、GameModule に C++ を使用します。
最初のビルド環境は、Windows、Visual Studio 2022、MSVC、CMake、Rust stable toolchain を対象とします。

## 対応環境
最初に対応する環境は以下です。
- Windows 10 または Windows 11
- Visual Studio 2022
- MSVC toolchain
- Windows SDK
- Rust stable toolchain

Linux、macOS、Vulkan、DirectX 12、クロスプラットフォームビルドは、最初のマイルストーンには含めません。

## 必要なツール
必要なツールは以下です。
- Visual Studio 2022
- Desktop development with C++
- MSVC v143
- Windows SDK
- CMake
- Rust stable
- Cargo
- VS Code
- rust-analyzer
- CMake Tools

## リポジトリ構成

初期リポジトリ構成は以下です。

zeno/
  engine_core/          Rust 製エンジンコア
  engine_backend_dx11/  C++ DirectX 11 ネイティブバックエンド
  engine_api/           Rust と C++ で共有する C ABI ヘッダー
  engine_cpp_sdk/       ゲームコード向け C++ ラッパー SDK
  samples/              Zeno 上で動作するサンプルゲーム
  docs/                 プロジェクトドキュメント
  tools/                補助ツールとスクリプト

この構成では、Rust 製エンジンコア、C++ 製ネイティブバックエンド、C ABI レイヤー、C++ SDK、サンプルゲームコードを分離します。
これにより、それぞれの責務が明確になり、ポートフォリオとしても説明しやすくなります。

## Rust ビルドポリシー
Rust 製エンジンコアは Cargo でビルドします。
Rust は、高レベルなエンジンランタイム、ゲームループ、モジュール読み込み、エンジン側ライフサイクル制御を担当します。

## C++ ビルドポリシー
C++ コンポーネントは MSVC と CMake でビルドします。
C++ 側は、ネイティブバックエンド、DirectX 11 レンダラー、C++ SDK、GameModule プロジェクトを担当します。

## GameModule ビルドポリシー
GameModule は、動的ライブラリとしてビルドします。
Rust 製エンジンコアは、実行時に GameModule DLL を読み込み、必須エクスポート関数を解決します。

## 出力ディレクトリポリシー

MVP 0.1 では、エンジン実行ファイルと GameModule DLL を同じランタイム出力ディレクトリに配置します。

想定する出力構成は以下です。

build/
  bin/
    Debug/
      zeno.exe
      aim_training_game.dll
    Release/
      zeno.exe
      aim_training_game.dll

実行ファイルと GameModule DLL を同じディレクトリに置くことで、実行時の読み込みをシンプルにします。
最初の MVP では、複雑な検索パス、アセットパッケージング、インストーラー形式の出力構成は避けます。

将来的には、より構造化されたランタイムディレクトリを導入する可能性がありますが、MVP 0.1 ではシンプルさとデバッグしやすさを優先します。

## Debug と Release ビルド
MVP 0.1 では Debug ビルドを優先します。
Release ビルドは後から対応するべきですが、初期開発では正確性、ログ出力、デバッグしやすさを重視します。

## VS Code の使用
VS Code は、Rust、C++、ドキュメント、リポジトリ全体の編集に使用するメインエディタです。

## Visual Studio 2022 の使用
Visual Studio 2022 は、MSVC ベースの C++ ビルド、ネイティブデバッグ、DirectX 11 バックエンドのデバッグ、CMake プロジェクトの確認に使用します。

## MVP 0.1 ビルドスコープ
MVP 0.1 のビルド設定には、以下を含めます。
- Rust engine_core 実行ファイルをビルドする
- C++ GameModule DLL をビルドする
- 実行ファイルと DLL を同じランタイムディレクトリに配置する
- Windows 上でエンジン実行ファイルを実行する
- 実行時に GameModule DLL を読み込む