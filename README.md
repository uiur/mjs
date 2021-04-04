# mjs

A minimal (toy) JavaScript interpreter.

The goal is that it can be used as a good subset of ES5.

The implementation is from scratch. It doesn't have any library dependencies except for standard C library.
The parser and lexer are written as a recursive descendant parser.

## motivation

> What I cannot create, I do not understand - Richard Feynman

The motivation is to:

- learn C language 
- learn algorithms and data structures such as hash table
- learn how to build an interpreter of a dynamic-typed language
- know more deeply about JavaScript

## status

- It can run sort algorithms 
- It has very basic object system with prototype chains

## development

Here's a note for myself.

### build

```sh
make
./build/main test/input/8-array-sort.js
```

### test

This runs unit tests in C and end-to-end tests in shell scripts.

```sh
make test_run
```

## examples

see `test/input`

```js
function sort(items) {
  for (var i = 0; i < items.length; i = i + 1) {
    var j = 0;
    var stop = items.length - 1;
    for (; j < stop; j = j + 1){
      if (items[j] > items[j + 1]){
        var temp = items[j];
        items[j] = items[j + 1];
        items[j + 1] = temp;
      }
    }
  }

  return items;
}

Array.prototype.sort = function () {
  return sort(this);
};

console.log([3, 2, 1].sort());
```
