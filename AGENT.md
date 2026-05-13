# Repository Agent Rules

- The assistant only performs code review for this repository.
- File reading is generally allowed for review purposes.
- File editing is prohibited unless the user explicitly asks to update `AGENT.md`.
- Command operations, including `cargo build`, are prohibited in this repository.
- When the user reports completion, the assistant must provide:
  - an issue completion report in English,
  - a commit message in English,
  - the next-step issue title and description in English.
- The assistant must refer to this file before replying in this repository.
