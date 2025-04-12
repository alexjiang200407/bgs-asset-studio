#!/bin/bash

# Recursive clang-format script for C/C++ files
# Usage: ./format.sh [directory] [--check]

# Default values
DIRECTORY="${1:-.}"  # Default to current directory if none specified
CHECK_MODE=false
FORMAT="clang-format"

# Check for --check flag
if [[ "$@" == *"--check"* ]]; then
    CHECK_MODE=true
fi

# Find all C/C++ files (add more extensions if needed)
FILES=$(find "$DIRECTORY" -path "$DIRECTORY/out" -prune -o -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" -o -name "*.cc" -o -name "*.m" -o -name "*.mm" \) -print)

# Check if clang-format exists
if ! command -v "$FORMAT" &> /dev/null; then
    echo "Error: $FORMAT not found in PATH"
    exit 1
fi

# Format or check files
ERROR_COUNT=0
for file in $FILES; do
    if [ "$CHECK_MODE" = true ]; then
        # Check formatting without modifying files
        if ! "$FORMAT" --dry-run --Werror "$file" &> /dev/null; then
            echo "[ERROR] $file needs formatting"
            ((ERROR_COUNT++))
        fi
    else
        # Format the file in-place
        echo "Formatting $file"
        "$FORMAT" -i "$file"
    fi
done

# Exit with appropriate status
if [ "$CHECK_MODE" = true ]; then
    if [ "$ERROR_COUNT" -gt 0 ]; then
        echo "Found $ERROR_COUNT files that need formatting"
        exit 1
    else
        echo "All files are properly formatted"
        exit 0
    fi
fi

echo "Formatting complete"
exit 0