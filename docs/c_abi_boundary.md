# C ABI Boundary Rules

## Purpose

This document defines the rules for the C ABI boundary used by Zeno.

Zeno uses Rust for the engine core and C++ for the native backend and game modules.
The C ABI boundary keeps the connection between Rust and C++ stable, explicit, and language-neutral.

The goal is to avoid exposing language-specific types or ownership models across the boundary.

## Allowed Types
Allowed:
- Integer types
- Floating point types
- Boolean-like integer values
- Plain data structures
- Handles
- IDs
- Raw pointers only when ownership is clear

## Forbidden Types
Forbidden:
- C++ classes
- C++ references
- std::string
- std::vector
- std::shared_ptr
- std::unique_ptr
- C++ exceptions
- Rust Vec
- Rust String
- Rust references
- Rust internal structs

## Handle Policy
Resource objects should be accessed through handles.
The C++ SDK should not own the actual engine resource directly.
The Rust core or native backend owns the real resource.
Game code only receives a handle.

## String Policy

Strings crossing the C ABI boundary must use C-compatible representations.

The boundary should use:
- const char* for read-only null-terminated strings
- pointer + length pairs when binary-safe or non-null-terminated strings are needed

The boundary must not use:
- std::string
- Rust String
- Rust &str
- C++ string references

The owner of the string memory must always be clear.
Game modules must not store string pointers unless the API explicitly defines the lifetime.
If the engine returns a string allocated internally, a matching release function must be provided.

## Memory Ownership
Every object crossing the boundary must have a clear owner.
The side that allocates memory should generally be responsible for freeing it.
The API should avoid requiring the game module to free memory allocated by Rust unless a specific release function exists.

## Error Handling
Errors should be returned as explicit result codes.
C++ exceptions must not cross the C ABI boundary.
Rust panics must not cross the C ABI boundary.

## Game Module Exports

Game modules are loaded as dynamic libraries by the Rust engine core.

Exported functions must use C ABI compatible signatures.
The first required exports are:
- GameStart
- GameUpdate
- GameDraw
- GameEnd

Exported functions must not expose:
- C++ classes
- C++ references
- std::string
- std::vector
- C++ exceptions

Game module exports should return explicit result codes when failure is possible.
The engine must handle missing required exports as a module load failure.

## API Versioning

The C API should have an explicit version.

The engine and game module should be able to check whether their API versions are compatible.
Breaking changes to the C ABI should require a version update.

The API should avoid changing existing function signatures after they are published.
When possible, new functionality should be added through new functions or new structures with version fields.

## Safety Rules

The C ABI boundary must be small, explicit, and stable.

Rust internal data structures must not be exposed directly to C++.
C++ implementation details must not be required by Rust.
All ownership rules must be documented before adding new boundary functions.
All failure cases must be represented without crossing language exceptions or panics.

If a new API requires pointers, arrays, strings, or callbacks, its lifetime and ownership rules must be documented before implementation.

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# C ABI 境界ルール

## 目的

このドキュメントは、Zeno で使用する C ABI 境界のルールを定義します。

Zeno は、エンジンコアに Rust、ネイティブバックエンドとゲームモジュールに C++ を使用します。
C ABI 境界は、Rust と C++ の接続を安定させ、明示的かつ言語非依存に保つためのものです。

目的は、言語固有の型や所有権モデルを境界の外へ公開しないことです。

## 許可される型
許可:
- 整数型
- 浮動小数点型
- 真偽値として扱う整数値
- プレーンなデータ構造
- ハンドル
- ID
- 所有権が明確な場合のみ、生ポインタ

## 禁止される型
禁止:
- C++ クラス
- C++ 参照
- std::string
- std::vector
- std::shared_ptr
- std::unique_ptr
- C++ 例外
- Rust Vec
- Rust String
- Rust 参照
- Rust の内部構造体

## ハンドルポリシー
リソースオブジェクトには、ハンドルを通してアクセスします。
C++ SDK は、実際のエンジンリソースを直接所有しません。
実体のリソースは、Rust コアまたはネイティブバックエンドが所有します。
ゲームコードは、ハンドルのみを受け取ります。

## 文字列ポリシー

C ABI 境界を越える文字列は、C 互換の表現を使用する必要があります。

境界では、以下を使用します。
- 読み取り専用の null 終端文字列には const char*
- バイナリセーフ、または null 終端ではない文字列が必要な場合は pointer + length の組み合わせ

境界では、以下を使用してはいけません。
- std::string
- Rust String
- Rust &str
- C++ の文字列参照

文字列メモリの所有者は、常に明確でなければなりません。
API がライフタイムを明示的に定義していない限り、ゲームモジュールは文字列ポインタを保持してはいけません。
エンジンが内部で確保した文字列を返す場合は、対応する解放関数を提供する必要があります。

## メモリ所有権
境界を越えるすべてのオブジェクトは、所有者を明確にする必要があります。
メモリを確保した側が、基本的にはそのメモリの解放も担当します。
API は、専用の解放関数が存在しない限り、Rust が確保したメモリをゲームモジュール側に解放させる設計を避けるべきです。

## エラーハンドリング
エラーは、明示的な結果コードとして返します。
C++ 例外を C ABI 境界の外へ越えさせてはいけません。
Rust panic を C ABI 境界の外へ越えさせてはいけません。

## ゲームモジュールのエクスポート

ゲームモジュールは、Rust 製エンジンコアによって動的ライブラリとして読み込まれます。

エクスポート関数は、C ABI 互換のシグネチャを使用する必要があります。
最初に必要なエクスポート関数は以下です。
- GameStart
- GameUpdate
- GameDraw
- GameEnd

エクスポート関数では、以下を公開してはいけません。
- C++ クラス
- C++ 参照
- std::string
- std::vector
- C++ 例外

失敗する可能性がある場合、ゲームモジュールのエクスポート関数は明示的な結果コードを返すべきです。
必須エクスポート関数が見つからない場合、エンジンはモジュール読み込み失敗として扱う必要があります。

## API バージョニング

C API には、明示的なバージョンを持たせるべきです。

エンジンとゲームモジュールは、互いの API バージョンに互換性があるか確認できるようにします。
C ABI に破壊的変更を加える場合は、バージョンの更新を必要とします。

公開後の既存関数シグネチャは、できる限り変更しないようにします。
可能な場合、新しい機能は新しい関数、またはバージョンフィールドを持つ新しい構造体として追加します。

## 安全性ルール

C ABI 境界は、小さく、明示的で、安定している必要があります。

Rust の内部データ構造を C++ に直接公開してはいけません。
Rust 側が C++ の実装詳細に依存する設計にしてはいけません。
新しい境界関数を追加する前に、すべての所有権ルールを文書化する必要があります。
すべての失敗ケースは、言語間で例外や panic を越えさせずに表現する必要があります。

新しい API がポインタ、配列、文字列、コールバックを必要とする場合は、実装前にライフタイムと所有権ルールを文書化する必要があります。