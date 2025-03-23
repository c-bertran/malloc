#!/bin/bash
# Format all C files in the project

# Set to your clang-format style preference
STYLE="file"  # Or: llvm, google, webkit, mozilla, gnu

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Starting C code formatting with style: ${STYLE}${NC}"

# Count total files
TOTAL=$(find . -type f \( -name "*.c" -o -name "*.h" \) -not -path "./lib/*" -not -path "./build/*" | wc -l)
echo -e "Found ${TOTAL} files to format"

# Process each file
COUNT=0
find . -type f \( -name "*.c" -o -name "*.h" \) -not -path "./lib/*" -not -path "./build/*" | while read file; do
  COUNT=$((COUNT+1))
  echo -e "${GREEN}[$COUNT/$TOTAL] Formatting:${NC} $file"
  clang-format -i -style=$STYLE "$file"
done

echo -e "${GREEN}Formatting complete!${NC}"
