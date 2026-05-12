# Initial C API

## Overview
The initial C API defines the small ABI surface shared between the Rust engine core, C++ native backend, C++ SDK, and C++ GameModule.
It is intentionally small for MVP 0.1.

## Purpose of engine_api
engine_api contains C-compatible API definitions shared across Zeno components.
It is the stable boundary layer between Rust and C++.
It should not expose Rust internal structures or C++ implementation details.

## Design Principles
- Keep the API small
- Use C-compatible types
- Use handles instead of direct object pointers
- Use explicit result codes
- Avoid ownership ambiguity
- Do not expose C++ classes or Rust internal structures
- Do not allow exceptions or panics to cross the boundary

## Common Types
The initial API should use:
- Fixed-width integer types
- Floating point types
- Boolean-like integer values
- POD structures
- Opaque handles

## Result Codes
The C API should use explicit result codes for operations that can fail.

Initial result categories:
- Success
- Unknown error
- Invalid argument
- Invalid handle
- Module load failure
- Missing export
- Backend failure

## Handles
Handles represent engine-owned or backend-owned objects.
Game code should not directly own internal engine objects.

Initial handle candidates:
- Texture handle
- Model handle
- Sound handle
- Entity handle

## POD Structures
POD structures may be used for simple values such as:
- Color
- 2D vector
- Rectangle
- Keyboard state

## GameModule Lifecycle API
The first GameModule lifecycle functions are:
- GameStart
- GameUpdate
- GameDraw
- GameEnd

The engine loads the module, resolves these exports, and calls them in the documented lifecycle order.

## Runtime API
Runtime API candidates for MVP 0.1:
- Request application quit
- Set clear color
- Get delta time if needed

## Input API
Input API for MVP 0.1 should only support basic keyboard checks.

Initial input functions:
- Check whether a key is currently down
- Check whether a key was pressed this frame

## Drawing API
Drawing API for MVP 0.1 should only support:
- Set clear color
- Draw a simple rectangle or placeholder 2D object

## Excluded APIs
Excluded from the first C API:
- Texture loading
- Model loading
- Sound playback
- Text rendering
- SceneManager API
- ResourceManager API
- Collision API
- Advanced input API
- Hot reload API
- Editor API

## Ownership Rules
The initial C API should avoid transferring ownership across the boundary.
If ownership transfer becomes necessary, the API must provide explicit creation and release functions.

## Error Handling
All fallible C API operations should report errors through result codes.
C++ exceptions and Rust panics must not cross the C ABI boundary.

## MVP 0.1 API Scope
MVP 0.1 C API scope:
- GameModule lifecycle calls
- Basic result codes
- Basic POD value types
- Basic keyboard input
- Clear color control
- Simple rectangle drawing

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# 初期 C API

## 概要
初期 C API は、Rust 製エンジンコア、C++ 製ネイティブバックエンド、C++ SDK、C++ GameModule の間で共有する小さな ABI 境界を定義します。
MVP 0.1 のため、意図的に小さく保ちます。

## engine_api の目的
engine_api には、Zeno の各コンポーネント間で共有する C 互換の API 定義を配置します。
これは、Rust と C++ の間にある安定した境界レイヤーです。
Rust の内部構造体や C++ の実装詳細を公開してはいけません。

## 設計原則
- API を小さく保つ
- C 互換の型を使用する
- 直接のオブジェクトポインタではなくハンドルを使用する
- 明示的な結果コードを使用する
- 所有権の曖昧さを避ける
- C++ クラスや Rust の内部構造体を公開しない
- 例外や panic を境界越しに伝播させない

## 共通型
初期 API では、以下を使用します。
- 固定幅整数型
- 浮動小数点型
- 真偽値として扱う整数値
- POD 構造体
- 不透明ハンドル

## 結果コード
C API では、失敗する可能性がある操作に対して明示的な結果コードを使用するべきです。

初期の結果カテゴリは以下です。
- 成功
- 不明なエラー
- 不正な引数
- 不正なハンドル
- モジュール読み込み失敗
- エクスポート不足
- バックエンド失敗

## ハンドル
ハンドルは、エンジンまたはバックエンドが所有するオブジェクトを表します。
ゲームコードは、エンジン内部オブジェクトを直接所有してはいけません。

初期のハンドル候補は以下です。
- テクスチャハンドル
- モデルハンドル
- サウンドハンドル
- エンティティハンドル

## POD 構造体
POD 構造体は、以下のような単純な値に使用できます。
- 色
- 2D ベクトル
- 矩形
- キーボード状態

## GameModule ライフサイクル API
最初の GameModule ライフサイクル関数は以下です。
- GameStart
- GameUpdate
- GameDraw
- GameEnd

エンジンはモジュールを読み込み、これらのエクスポート関数を解決し、文書化されたライフサイクル順序で呼び出します。

## ランタイム API
MVP 0.1 のランタイム API 候補は以下です。
- アプリケーション終了要求
- クリアカラー設定
- 必要に応じたデルタタイム取得

## 入力 API
MVP 0.1 の入力 API は、基本的なキーボード確認のみをサポートするべきです。

初期の入力関数は以下です。
- キーが現在押されているか確認する
- キーがこのフレームで押されたか確認する

## 描画 API
MVP 0.1 の描画 API は、以下のみをサポートするべきです。
- クリアカラー設定
- 簡単な矩形、または仮の 2D オブジェクト描画

## 除外する API
最初の C API からは、以下を除外します。
- テクスチャ読み込み
- モデル読み込み
- サウンド再生
- テキスト描画
- SceneManager API
- ResourceManager API
- Collision API
- 高度な入力 API
- Hot reload API
- Editor API

## 所有権ルール
初期 C API では、境界を越えた所有権の移動を避けるべきです。
所有権の移動が必要になった場合、API は明示的な作成関数と解放関数を提供する必要があります。

## エラーハンドリング
失敗する可能性があるすべての C API 操作は、結果コードを通してエラーを報告するべきです。
C++ 例外や Rust panic を C ABI 境界の外へ越えさせてはいけません。

## MVP 0.1 API スコープ
MVP 0.1 の C API スコープは以下です。
- GameModule ライフサイクル呼び出し
- 基本的な結果コード
- 基本的な POD 値型
- 基本的なキーボード入力
- クリアカラー制御
- 簡単な矩形描画