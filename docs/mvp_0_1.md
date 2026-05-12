# MVP 0.1 Scope

## Overview
MVP 0.1 is the first working version of Zeno.
The goal is to prove that the Rust engine core can load and run a C++ GameModule DLL through a C ABI boundary.

## Goal
The goal of MVP 0.1 is to run a minimal game loop where the Rust engine core loads a C++ GameModule DLL and calls its lifecycle functions.

## Included Features
- Create a Windows application entry point
- Create a window
- Run a 60 FPS-style game loop
- Load a C++ GameModule DLL
- Resolve GameStart, GameUpdate, GameDraw, and GameEnd
- Call GameStart once
- Call GameUpdate every frame
- Call GameDraw every frame
- Call GameEnd once before shutdown
- Read basic keyboard input
- Change the clear color
- Draw a simple rectangle or placeholder 2D object

## Excluded Features
- 3D model rendering
- Texture resource management
- SceneManager
- Audio
- Text rendering
- Collision system
- Advanced input system
- Hot reload
- Editor
- Multi-platform support
- DirectX 12 or Vulkan support

## Runtime Flow
1. Start the Zeno executable.
2. Initialize the native backend.
3. Create a window.
4. Load the GameModule DLL.
5. Resolve required lifecycle functions.
6. Call GameStart.
7. Enter the game loop.
8. Poll input.
9. Call GameUpdate.
10. Clear the screen.
11. Call GameDraw.
12. Present the frame.
13. Exit the loop when requested.
14. Call GameEnd.
15. Unload the GameModule.
16. Shutdown the engine.

## Success Criteria
MVP 0.1 is complete when:
- The application starts on Windows.
- A window is created successfully.
- The engine runs a frame loop.
- A C++ GameModule DLL is loaded.
- GameStart, GameUpdate, GameDraw, and GameEnd are called in the correct order.
- Keyboard input can affect the running application.
- The screen clear color can be changed from game-side logic.
- A simple rectangle or placeholder object can be drawn.
- The application shuts down cleanly.

## Implementation Issues
- Create Rust engine_core application entry
- Define initial C API header
- Create C++ GameModule project
- Load GameModule DLL from Rust
- Resolve GameModule lifecycle functions
- Call GameStart / GameUpdate / GameDraw / GameEnd
- Create native window
- Add basic frame loop
- Add keyboard input test
- Add clear color command
- Add simple rectangle draw test
- Add shutdown sequence

## Notes

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# MVP 0.1 スコープ

## 概要
MVP 0.1 は、Zeno の最初に動作するバージョンです。
目的は、Rust 製エンジンコアが C ABI 境界を通して C++ GameModule DLL を読み込み、実行できることを証明することです。

## 目標
MVP 0.1 の目標は、Rust 製エンジンコアが C++ GameModule DLL を読み込み、そのライフサイクル関数を呼び出す最小限のゲームループを動作させることです。

## 含める機能
- Windows アプリケーションのエントリポイントを作成する
- ウィンドウを作成する
- 60 FPS 形式のゲームループを実行する
- C++ GameModule DLL を読み込む
- GameStart、GameUpdate、GameDraw、GameEnd を解決する
- GameStart を一度だけ呼び出す
- GameUpdate を毎フレーム呼び出す
- GameDraw を毎フレーム呼び出す
- シャットダウン前に GameEnd を一度だけ呼び出す
- 基本的なキーボード入力を読み取る
- クリアカラーを変更する
- 簡単な矩形、または仮の 2D オブジェクトを描画する

## 含めない機能
- 3D モデル描画
- テクスチャリソース管理
- SceneManager
- オーディオ
- テキスト描画
- コリジョンシステム
- 高度な入力システム
- ホットリロード
- エディタ
- マルチプラットフォーム対応
- DirectX 12 または Vulkan 対応

## ランタイムフロー
1. Zeno 実行ファイルを起動する。
2. ネイティブバックエンドを初期化する。
3. ウィンドウを作成する。
4. GameModule DLL を読み込む。
5. 必須ライフサイクル関数を解決する。
6. GameStart を呼び出す。
7. ゲームループに入る。
8. 入力をポーリングする。
9. GameUpdate を呼び出す。
10. 画面をクリアする。
11. GameDraw を呼び出す。
12. フレームを表示する。
13. 終了要求があったらループを抜ける。
14. GameEnd を呼び出す。
15. GameModule をアンロードする。
16. エンジンをシャットダウンする。

## 成功条件
MVP 0.1 は、以下を満たした時点で完了とします。
- アプリケーションが Windows 上で起動する。
- ウィンドウが正常に作成される。
- エンジンがフレームループを実行する。
- C++ GameModule DLL が読み込まれる。
- GameStart、GameUpdate、GameDraw、GameEnd が正しい順序で呼び出される。
- キーボード入力が実行中のアプリケーションに影響を与えられる。
- ゲーム側ロジックから画面のクリアカラーを変更できる。
- 簡単な矩形、または仮のオブジェクトを描画できる。
- アプリケーションが正常にシャットダウンする。

## 実装タスク
- Rust engine_core のアプリケーションエントリを作成する
- 初期 C API ヘッダーを定義する
- C++ GameModule プロジェクトを作成する
- Rust から GameModule DLL を読み込む
- GameModule のライフサイクル関数を解決する
- GameStart / GameUpdate / GameDraw / GameEnd を呼び出す
- ネイティブウィンドウを作成する
- 基本的なフレームループを追加する
- キーボード入力テストを追加する
- クリアカラー変更コマンドを追加する
- 簡単な矩形描画テストを追加する
- シャットダウンシーケンスを追加する

## メモ