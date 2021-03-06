#!/bin/bash -e
executable=./build/main
echo "running tests..."
echo
for path in $(ls test/input/*.js); do
  name=$(basename "$path" .js)
  actual=$($executable $path)
  expected=$(cat "test/output/${name}.out")
  if [ "$expected" != "$actual" ]; then
    echo "test failed ($path):"
    echo "  expect: $expected"
    echo "  actual: $actual"
  else
    echo "test passed ($path)"
  fi
done
