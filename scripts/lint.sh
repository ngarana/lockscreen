#!/usr/bin/env bash
# scripts/lint.sh — Run clang-format and clang-tidy checks for qypr.
#
# Modes:
#   ./scripts/lint.sh                 # check whole src/ + tests/ tree
#   ./scripts/lint.sh --fix           # auto-format + apply safe tidy fixits
#   ./scripts/lint.sh --format-only   # only clang-format check
#   ./scripts/lint.sh --tidy-only     # only clang-tidy
#   ./scripts/lint.sh --staged        # check files listed in $QYPR_LINT_FILES
#                                     # (called by the pre-commit hook)
#
# The pre-commit hook calls:
#   QYPR_LINT_FILES="file1.cpp file2.hpp …" ./scripts/lint.sh --staged
#
# Exit codes:  0 = clean,  1 = violations or missing tool
#
# Prerequisites:
#   clang-format and clang-tidy must be on PATH.
#   clang-tidy also needs compile_commands.json.  The script looks for it at
#   <root>/compile_commands.json or <root>/build/compile_commands.json and
#   auto-generates it into ./build if neither exists.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FIX=0
FORMAT_ONLY=0
TIDY_ONLY=0
STAGED=0
JOBS="${NPROC:-$(nproc 2>/dev/null || echo 4)}"

# ---- argument parsing -------------------------------------------------------
for arg in "$@"; do
    case "$arg" in
        --fix)          FIX=1 ;;
        --format-only)  FORMAT_ONLY=1 ;;
        --tidy-only)    TIDY_ONLY=1 ;;
        --staged)       STAGED=1 ;;
        -j*)            JOBS="${arg#-j}" ;;
        --jobs=*)       JOBS="${arg#--jobs=}" ;;
        -h|--help)
            sed -n '2,18p' "$0" | sed 's/^# \?//'
            exit 0 ;;
        *)
            echo "Unknown option: $arg" >&2; exit 1 ;;
    esac
done

# ---- colours ----------------------------------------------------------------
if [[ -t 1 || -t 2 ]]; then
    RED='\033[0;31m'; YELLOW='\033[0;33m'; GREEN='\033[0;32m'; RESET='\033[0m'
else
    RED=''; YELLOW=''; GREEN=''; RESET=''
fi
red()    { printf "${RED}%s${RESET}\n"    "$*"; }
yellow() { printf "${YELLOW}%s${RESET}\n" "$*"; }
green()  { printf "${GREEN}%s${RESET}\n"  "$*"; }
header() { printf "\n${YELLOW}=== %s ===${RESET}\n" "$*"; }

PASS=0; FAIL=0

# ---- collect files ----------------------------------------------------------
if [[ "$STAGED" -eq 1 ]]; then
    # File list injected by the pre-commit hook via environment variable
    read -ra ALL_FILES <<< "${QYPR_LINT_FILES:-}"
    if [[ ${#ALL_FILES[@]} -eq 0 ]]; then
        green "lint: no staged C++ files to check."
        exit 0
    fi
    MODE_LABEL="staged (${#ALL_FILES[@]} file(s))"
else
    # Whole-tree scan
    mapfile -t ALL_FILES < <(
        find "$ROOT/src" "$ROOT/tests" \
            \( -name '*.cpp' -o -name '*.hpp' \) \
            -not -path '*/wayland-generated/*' \
            -not -path '*/build*/*' \
            | sort
    )
    MODE_LABEL="tree (${#ALL_FILES[@]} file(s))"
fi

echo "qypr lint  |  mode=$MODE_LABEL  |  jobs=$JOBS  |  fix=$FIX"

# ---- 1. clang-format --------------------------------------------------------
if [[ "$TIDY_ONLY" -eq 0 ]]; then
    header "clang-format"

    if ! command -v clang-format &>/dev/null; then
        red "ERROR: clang-format not found."
        exit 1
    fi

    FORMAT_FAIL=()
    for f in "${ALL_FILES[@]}"; do
        if [[ "$FIX" -eq 1 ]]; then
            clang-format -i "$f"
        else
            if ! clang-format --dry-run --Werror "$f" 2>/dev/null; then
                FORMAT_FAIL+=("${f#"$ROOT/"}")
            fi
        fi
    done

    if [[ "$FIX" -eq 1 ]]; then
        green "✓ clang-format: reformatted in-place"
        PASS=$((PASS + 1))
    elif [[ ${#FORMAT_FAIL[@]} -eq 0 ]]; then
        green "✓ clang-format: all files properly formatted"
        PASS=$((PASS + 1))
    else
        red "✗ clang-format: ${#FORMAT_FAIL[@]} file(s) need reformatting:"
        for f in "${FORMAT_FAIL[@]}"; do printf '    %s\n' "$f"; done
        yellow "  Fix with:  $0 --fix"
        FAIL=$((FAIL + 1))
    fi
fi

# ---- 2. clang-tidy ----------------------------------------------------------
if [[ "$FORMAT_ONLY" -eq 0 ]]; then
    header "clang-tidy"

    if ! command -v clang-tidy &>/dev/null; then
        red "ERROR: clang-tidy not found."
        exit 1
    fi

    # Locate or auto-generate compile_commands.json
    DB=""
    for candidate in "$ROOT/compile_commands.json" "$ROOT/build/compile_commands.json"; do
        if [[ -f "$candidate" ]]; then
            DB="$(dirname "$(realpath "$candidate")")"
            break
        fi
    done

    if [[ -z "$DB" ]]; then
        yellow "compile_commands.json not found – generating into ./build …"
        cmake -S "$ROOT" -B "$ROOT/build" -G Ninja \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_COMPILER=clang++ \
            >/dev/null
        DB="$ROOT/build"
        green "Generated $DB/compile_commands.json"
    fi

    # clang-tidy processes .cpp translation units; headers are analysed through them.
    CPP_ONLY=()
    for f in "${ALL_FILES[@]}"; do
        [[ "$f" == *.cpp ]] || continue
        # Skip files not yet in the compile DB (newly created, not yet built).
        # clang-tidy prints its --help page for unknown files, which is noise.
        if ! grep -qF "$(basename "$f")" "$DB/compile_commands.json" 2>/dev/null; then
            yellow "  ⚠ skipping $(basename "$f") (not in compile DB — rebuild first)"
            continue
        fi
        CPP_ONLY+=("$f")
    done

    if [[ ${#CPP_ONLY[@]} -eq 0 ]]; then
        yellow "  ⚠ no .cpp files to analyse (all skipped or none staged)"
        PASS=$((PASS + 1))
    else
        TIDY_EXTRA=()
        # --fix only (never --fix-errors): clang-tidy must refuse to rewrite a
        # translation unit that does not compile. --fix-errors applies fixits on
        # top of a broken AST, which produces garbage edits (bogus `static`,
        # corrupted literals) — the exact failure that motivated this guard.
        [[ "$FIX" -eq 1 ]] && TIDY_EXTRA+=(--fix)

        TIDY_FAIL=0
        for f in "${CPP_ONLY[@]}"; do
            if ! clang-tidy -p "$DB" --quiet "${TIDY_EXTRA[@]}" "$f"; then
                TIDY_FAIL=1
            fi
        done

        if [[ "$TIDY_FAIL" -eq 0 ]]; then
            green "✓ clang-tidy: no errors in ${#CPP_ONLY[@]} translation unit(s)"
            PASS=$((PASS + 1))
        else
            red "✗ clang-tidy: errors found (see output above)"
            yellow "  Auto-fix safe issues with:  $0 --fix"
            FAIL=$((FAIL + 1))
        fi
    fi
fi

# ---- summary ----------------------------------------------------------------
echo ""
echo "────────────────────────────────────────"
if [[ "$FAIL" -gt 0 ]]; then
    red "FAIL  $PASS passed / $FAIL failed"
    exit 1
fi
green "PASS  $PASS check(s) passed, 0 failed"
exit 0
