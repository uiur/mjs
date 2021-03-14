#!/bin/bash
executable=./build/main

GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m'

fail() {
  echo -e "${RED}test failed ($1):${RESET}"
}

echo "running tests..."
echo
for path in $(ls test/input/*.js); do
  name=$(basename "$path" .js)
  actual=$($executable $path)
  exit_code=$?
  if [[ exit_code -ne 0 ]]; then
    fail $path
    echo "  program exited with $exit_code"
    echo "  $executable $path"
    continue
  fi

  expected=$(cat "test/output/${name}.out")
  if [ "$expected" != "$actual" ]; then
    fail $path
    echo "  expect: $expected"
    echo "  actual: $actual"
  else
    echo -e "${GREEN}test passed ($path)${RESET}"
  fi
done
