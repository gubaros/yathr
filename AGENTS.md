# AGENTS.md

This file defines practical instructions for coding agents working in this repository.

## Repository facts

- This is a C codebase for an HTTP redirect server.
- Main modules in the repository root:
  - `server.c` (entry point/event loop setup)
  - `platform.c/.h` (event loop abstraction: epoll on Linux, kqueue on non-Linux)
  - `http.c/.h` (request handling and HTTP responses)
  - `routing.c/.h` (in-memory redirect table and lookup)
- Utility modules:
  - `utils/socket.c/.h`
  - `utils/config.c/.h`
  - `utils/logs.c/.h`
- Plugin modules:
  - `plugins/plugin.c/.h`
  - `plugins/pre_routing_plugin.c`
  - `plugins/post_routing_plugin.c`

## Build and run commands

- Build the server with:
  - `make`
  - or `make http_server`
- Clean build outputs with:
  - `make clean`
- Run the server binary:
  - `./http_server`

## Runtime/configuration facts

- Server port is read from `config.txt` via `SERVER_PORT=...`.
- Logging is configured with `zlog.conf`.
- `utils/logs.c` uses zlog (`<zlog.h>`), and the Makefile links with `-lzlog`.

## GitHub Actions present in this repo

- `.github/workflows/ci.yml`
  - Triggers on push/pull_request to `main`.
  - Installs build dependencies, tinycdb and zlog, then runs compile/run checks.
- `.github/workflows/cia.yml`
  - Triggers on pull requests.
  - Sets up Python and runs `python review_code.py`.

## Agent guidelines for changes

- Treat this as a C project first; do not introduce unrelated stack assumptions.
- Keep edits minimal and scoped to the requested task.
- When changing build/runtime behavior, verify commands and file references against:
  - `Makefile`
  - `.github/workflows/*.yml`
  - existing source files in root, `utils/`, and `plugins/`.
