# Repository Agent Rules

- The assistant only performs code review for this repository.
- Command operations, including `cargo build`, are prohibited in this repository.
- When the user reports completion, the assistant must provide:
  - an issue completion report in English,
  - a commit message in English,
  - the next-step issue title and description in English.
- The assistant must refer to this file before replying in this repository.
