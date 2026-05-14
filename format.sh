#!/usr/bin/env sh

# Taken from https://github.com/iDontHaveTime/Inertia

EXCLUDE_DIRS="build"

find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.inc" \) \
  ! -regex ".*\($EXCLUDE_DIRS\).*" \
  -exec clang-format -i {} \;