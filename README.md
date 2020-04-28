## Toolchain

- C++-Standard: ISO-C++ 17
- Compiler Flags: `-Werror -Wall -Wextra -Wpedantic -O2`
- Build: CMake
- Style guide: [Webkit](https://webkit.org/code-style-guidelines/)
    - Umsetzung mit `clang-format`/`clang-tidy`
- Dokumentation: javadoc
- Tests: [doctest](https://github.com/onqtam/doctest)
- Git:
    - [Repo]
    - Gitflow Workflow
    - Rebase statt Mergen

## Repo-Aufbau

- extern/
- include/
- src/
- test/

## Todo

- Externe Builds aus CMake entfernen, falls möglich
- S. Präsentation 28.4.
- `tiling` durch `tiling_size` oder ähnliches ersetzen
