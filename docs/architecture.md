# Zeno Architecture

## Overview
    Zeno is a small game engine project built for learning and portfolio purposes.
    It uses a Rust engine core and a C++ native backend.
    The engine allows C++ game modules to run on top of the Rust core through a C ABI boundary.

## Goals
    - Build a small but working game engine.
    - Use Rust for the engine core.
    - Use C++ for the Windows native backend.
    - Provide a C++ SDK.
    - Run a playable sample game on the engine.
    - Keep the architecture simple enough to complete within the project timeline.

## Non-goals
    - Zeno is not intended to replace Unity or Unreal Engine.
    - The first target platform is Windows only.
    - The first graphics API target is DirectX 11.
    - A visual editor is not included in the first milestone.
    - Vulkan, DirectX 12, and multi-platform support are not initial goals.
    - Full custom physics engine development is not included in the first milestone.

## Repository Structure
    zeno/
    　engine_core/          Rust engine core
    　engine_backend_dx11/  C++ DirectX 11 backend
    　engine_api/           C ABI headers
    　engine_cpp_sdk/       C++ wrapper SDK
    　samples/              Sample games
    　docs/                 Documentation

## Module Responsibilities
    The Rust engine core owns high-level engine systems such as the game loop, scene management, entity management, resource handles, time management, and game module lifecycle.

    The C++ native backend owns platform-specific systems such as Win32 window creation, DirectX 11 rendering, input, audio, and debug drawing.

    The C ABI layer connects Rust and C++ without exposing Rust internal structures or C++ classes across the boundary.

    The C++ SDK provides a simple API for game developers.

## Rust Engine Core
    The Rust engine core is responsible for:
    - Game loop control
    - Scene management
    - Entity and handle management
    - Resource management
    - Time management
    - Collision management
    - Loading and calling C++ game modules

## C++ Native Backend
    The C++ native backend is responsible for:
    - Win32 window management
    - DirectX 11 rendering
    - Keyboard and mouse input
    - Texture rendering
    - 3D model rendering
    - Text rendering
    - Audio playback
    - Debug drawing

## C ABI Boundary
    The C ABI boundary is used to keep the Rust and C++ connection stable and explicit.

    The boundary does not expose:
    - C++ classes
    - std::string
    - std::vector
    - C++ exceptions
    - Rust internal structures

    The boundary uses:
    - Handles
    - IDs
    - Plain data structures
    - Explicit result codes

## C++ Game SDK
    The C++ Game SDK wraps the low-level C API and provides a simple interface for game code.

    The SDK should feel similar to a lightweight learning framework.
    Game developers should be able to write game logic without directly touching the C ABI layer.

## Game Module Lifecycle
    A game module is a C++ dynamic library loaded by the Rust engine core.

    The first lifecycle functions are:
    - GameStart
    - GameUpdate
    - GameDraw
    - GameEnd

    The engine loads the module, resolves these functions, and calls them during the game loop.

## Initial MVP
    The first MVP focuses on proving that the engine can load and run a C++ game module.

    The MVP includes:
    - Create a Windows window
    - Run a 60 FPS-style game loop
    - Load a C++ GameModule DLL
    - Call GameStart, GameUpdate, GameDraw, and GameEnd
    - Read keyboard input
    - Change the clear color
    - Draw a rectangle or simple 2D sprite

## Future Direction
    After the first MVP, Zeno will gradually add:
    - 2D texture rendering
    - SceneManager
    - C++ SDK improvements
    - 3D rendering
    - Camera and lighting
    - Ray and sphere collision
    - Sound
    - ResourceManager
    - DebugDraw
    - A playable 3D aim training sample game
  
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# Zeno アーキテクチャ

## 概要
    Zeno は、学習とポートフォリオ制作を目的とした小規模なゲームエンジンプロジェクトです。
    Rust 製のエンジンコアと、C++ 製のネイティブバックエンドで構成されます。
    エンジンは C ABI 境界を通して、Rust コア上で C++ のゲームモジュールを実行できるようにします。

## 目標
    - 小規模だが実際に動作するゲームエンジンを作る。
    - エンジンコアには Rust を使用する。
    - Windows 向けネイティブバックエンドには C++ を使用する。
    - C++ SDK を提供する。
    - エンジン上で動作するプレイ可能なサンプルゲームを作る。
    - プロジェクト期間内に完成できるよう、アーキテクチャを十分にシンプルに保つ。

## 非目標
    - Zeno は Unity や Unreal Engine の代替を目的としない。
    - 最初の対象プラットフォームは Windows のみとする。
    - 最初のグラフィックス API は DirectX 11 とする。
    - 最初のマイルストーンにはビジュアルエディタを含めない。
    - Vulkan、DirectX 12、マルチプラットフォーム対応は初期目標に含めない。
    - フルスクラッチの物理エンジン開発は最初のマイルストーンに含めない。

## リポジトリ構成
    zeno/
    　engine_core/          Rust 製エンジンコア
    　engine_backend_dx11/  C++ DirectX 11 バックエンド
    　engine_api/           C ABI ヘッダー
    　engine_cpp_sdk/       C++ ラッパー SDK
    　samples/              サンプルゲーム
    　docs/                 ドキュメント

## モジュールの責務
    Rust 製エンジンコアは、ゲームループ、シーン管理、エンティティ管理、リソースハンドル、時間管理、ゲームモジュールのライフサイクルなど、エンジンの高レベルなシステムを担当します。

    C++ 製ネイティブバックエンドは、Win32 ウィンドウ作成、DirectX 11 レンダリング、入力、オーディオ、デバッグ描画など、プラットフォーム固有のシステムを担当します。

    C ABI レイヤーは、Rust と C++ を接続します。この境界では、Rust の内部構造体や C++ のクラスを直接公開しません。

    C++ SDK は、ゲーム開発者向けに扱いやすいシンプルな API を提供します。

## Rust 製エンジンコア
    Rust 製エンジンコアは、以下を担当します。
    - ゲームループ制御
    - シーン管理
    - エンティティとハンドル管理
    - リソース管理
    - 時間管理
    - コリジョン管理
    - C++ ゲームモジュールの読み込みと呼び出し

## C++ 製ネイティブバックエンド
    C++ 製ネイティブバックエンドは、以下を担当します。
    - Win32 ウィンドウ管理
    - DirectX 11 レンダリング
    - キーボードとマウス入力
    - テクスチャ描画
    - 3D モデル描画
    - テキスト描画
    - オーディオ再生
    - デバッグ描画

## C ABI 境界
    C ABI 境界は、Rust と C++ の接続を安定かつ明示的に保つために使用します。

    この境界では、以下を公開しません。
    - C++ クラス
    - std::string
    - std::vector
    - C++ 例外
    - Rust の内部構造体

    この境界では、以下を使用します。
    - ハンドル
    - ID
    - プレーンなデータ構造
    - 明示的な結果コード

## C++ Game SDK
    C++ Game SDK は、低レベルな C API をラップし、ゲームコード向けにシンプルなインターフェースを提供します。

    SDK は、軽量な学習用フレームワークに近い使い心地を目指します。
    ゲーム開発者は、C ABI レイヤーを直接触らずにゲームロジックを書けるようにします。

## ゲームモジュールのライフサイクル
    ゲームモジュールは、Rust 製エンジンコアによって読み込まれる C++ の動的ライブラリです。

    最初に用意するライフサイクル関数は以下です。
    - GameStart
    - GameUpdate
    - GameDraw
    - GameEnd

    エンジンはモジュールを読み込み、これらの関数を解決し、ゲームループ中に呼び出します。

## 初期 MVP
    最初の MVP では、エンジンが C++ ゲームモジュールを読み込んで実行できることを証明します。

    MVP に含める内容は以下です。
    - Windows ウィンドウを作成する
    - 60 FPS 形式のゲームループを実行する
    - C++ GameModule DLL を読み込む
    - GameStart、GameUpdate、GameDraw、GameEnd を呼び出す
    - キーボード入力を読み取る
    - クリアカラーを変更する
    - 矩形または簡単な 2D スプライトを描画する

## 今後の方向性
    最初の MVP 後、Zeno には段階的に以下を追加していきます。
    - 2D テクスチャ描画
    - SceneManager
    - C++ SDK の改善
    - 3D レンダリング
    - カメラとライティング
    - レイとスフィアのコリジョン
    - サウンド
    - ResourceManager
    - DebugDraw
    - プレイ可能な 3D エイムトレーニングのサンプルゲーム