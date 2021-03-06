#!/bin/bash
executable=./build/main

GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m'

echo "running tests..."
echo
for path in $(ls test/input/*.js); do
  name=$(basename "$path" .js)
  actual=$($executable $path)
  expected=$(cat "test/output/${name}.out")
  if [ "$expected" != "$actual" ]; then
    echo -e "${RED}test failed ($path):${RESET}"
    echo "  expect: $expected"
    echo "  actual: $actual"
  else
    echo -e "${GREEN}test passed ($path)${RESET}"
  fi
done
