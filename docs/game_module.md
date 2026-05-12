# Game Module
A GameModule is a C++ dynamic library loaded by the Zeno engine core.
It contains game-specific logic and exposes a small set of C ABI functions.

## Overview
A GameModule is a C++ dynamic library loaded by the Zeno engine core.
It contains game-specific logic and exposes a small set of C ABI functions.

## Role of GameModule
The GameModule owns game-specific behavior such as initialization, update logic, drawing commands, and shutdown logic.
It should not own the engine core or native backend directly.

## Required Exports
The first required exported functions are:
- GameStart
- GameUpdate
- GameDraw
- GameEnd

## Lifecycle Order
1. The engine loads the GameModule DLL.
2. The engine resolves required exported functions.
3. The engine calls GameStart once.
4. The engine calls GameUpdate every frame.
5. The engine calls GameDraw every frame.
6. The engine calls GameEnd once before shutdown.
7. The engine unloads the GameModule.

## GameStart
GameStart is called once after the GameModule is loaded.
It is used for game initialization, resource loading requests, and initial state setup.

## GameUpdate
GameUpdate is called once per frame.
It is used for input handling, game logic, scene updates, entity updates, and collision-related logic.

## GameDraw
GameDraw is called once per frame after GameUpdate.
It is used to submit drawing commands through the C++ SDK or engine API.

## GameEnd
GameEnd is called once before the GameModule is unloaded.
It is used to release game-side resources and perform shutdown logic.

## Access to Engine API
The GameModule should access engine functionality through the C++ Game SDK.
The SDK wraps the lower-level C API.
The GameModule must not access Rust internal structures directly.

## Failure Handling
If the DLL cannot be loaded, the engine should report a module load failure.
If a required export is missing, the engine should reject the module.
If a lifecycle function returns an error, the engine should handle it without allowing exceptions or panics to cross the boundary.

## Unload Policy
The engine should call GameEnd before unloading the GameModule.
The GameModule should not keep references to engine-owned memory after GameEnd.

## MVP Scope
For the first MVP, the GameModule only needs to:
- Start successfully
- Update every frame
- Draw every frame
- End cleanly
- Change the clear color or submit a simple draw command

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# ゲームモジュール
GameModule は、Zeno エンジンコアによって読み込まれる C++ の動的ライブラリです。
ゲーム固有のロジックを含み、小さな C ABI 関数群を公開します。

## 概要
GameModule は、Zeno エンジンコアによって読み込まれる C++ の動的ライブラリです。
ゲーム固有のロジックを含み、小さな C ABI 関数群を公開します。

## GameModule の役割
GameModule は、初期化、更新ロジック、描画コマンド、終了処理など、ゲーム固有の振る舞いを担当します。
エンジンコアやネイティブバックエンドを直接所有してはいけません。

## 必須エクスポート
最初に必要なエクスポート関数は以下です。
- GameStart
- GameUpdate
- GameDraw
- GameEnd

## ライフサイクル順序
1. エンジンが GameModule DLL を読み込む。
2. エンジンが必須エクスポート関数を解決する。
3. エンジンが GameStart を一度だけ呼び出す。
4. エンジンが GameUpdate を毎フレーム呼び出す。
5. エンジンが GameDraw を毎フレーム呼び出す。
6. エンジンがシャットダウン前に GameEnd を一度だけ呼び出す。
7. エンジンが GameModule をアンロードする。

## GameStart
GameStart は、GameModule が読み込まれた後に一度だけ呼び出されます。
ゲームの初期化、リソース読み込み要求、初期状態の設定に使用します。

## GameUpdate
GameUpdate は、1 フレームに一度呼び出されます。
入力処理、ゲームロジック、シーン更新、エンティティ更新、コリジョン関連の処理に使用します。

## GameDraw
GameDraw は、GameUpdate の後に 1 フレームに一度呼び出されます。
C++ SDK またはエンジン API を通して、描画コマンドを送信するために使用します。

## GameEnd
GameEnd は、GameModule がアンロードされる前に一度だけ呼び出されます。
ゲーム側リソースの解放や終了処理に使用します。

## エンジン API へのアクセス
GameModule は、C++ Game SDK を通してエンジン機能にアクセスするべきです。
SDK は、低レベルな C API をラップします。
GameModule は、Rust の内部構造体へ直接アクセスしてはいけません。

## 失敗時の処理
DLL を読み込めない場合、エンジンはモジュール読み込み失敗を報告するべきです。
必須エクスポート関数が見つからない場合、エンジンはそのモジュールを拒否するべきです。
ライフサイクル関数がエラーを返した場合、エンジンは例外や panic を境界越しに伝播させずに処理するべきです。

## アンロードポリシー
エンジンは、GameModule をアンロードする前に GameEnd を呼び出すべきです。
GameModule は、GameEnd 後にエンジン所有メモリへの参照を保持してはいけません。

## MVP スコープ
最初の MVP では、GameModule は以下のみを満たせば十分です。
- 正常に開始できる
- 毎フレーム更新できる
- 毎フレーム描画できる
- 正常に終了できる
- クリアカラーを変更する、または簡単な描画コマンドを送信する