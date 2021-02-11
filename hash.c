#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASH_RESIZE_LOAD_FACTOR 0.5

HashTable* hash_table_new() {
  HashTable* hash = malloc(sizeof(HashTable));
  int cap = 10;
  hash->cap = cap;
  hash->used = 0;
  hash->entries = malloc(cap * sizeof(HashTableEntry*));

  for (int i = 0; i < cap; i++) {
    hash->entries[i] = NULL;
  }

  return hash;
}

unsigned int key_hash(const char *key) {
  unsigned int value = 0;
  for (const char *c = key; *c != '\0'; c++) {
    value = (value * 31 + *c);
  }
  return value;
}

HashTableEntry* hash_table_find_entry(HashTable* hash, const char *key) {
  int i = key_hash(key) % hash->cap;

  HashTableEntry *entry = hash->entries[i];
  for (; entry != NULL; entry = entry->next) {
    if (strcmp(entry->key, key) == 0) {
      return entry;
    }
  }

  return NULL;
}

void hash_table_insert_entry(HashTable* hash, int index, HashTableEntry *new_entry) {
  HashTableEntry *entry = hash->entries[index];
  if (entry != NULL) {
    while (entry->next != NULL) entry = entry->next;
    entry->next = new_entry;
  } else {
    hash->entries[index] = new_entry;
  }

  hash->used += 1;
}

void hash_table_resize(HashTable* hash, unsigned int new_size) {
  HashTableEntry **new_entries = malloc(new_size * sizeof(HashTableEntry*));
  // todo: memset?
  for (int i = 0; i < new_size; i++) {
    new_entries[i] = NULL;
  }

  unsigned int size = hash->cap;
  HashTableEntry **old_entries = hash->entries;
  hash->entries = new_entries;

  for (int i = 0; i < size; i++) {
    HashTableEntry *entry = old_entries[i];
    while (entry != NULL) {
      HashTableEntry *next_entry = entry->next;
      entry->next = NULL;

      int table_index = key_hash(entry->key) % new_size;
      hash_table_insert_entry(hash, table_index, entry);
      entry = next_entry;
    }
  }

  free(old_entries);

  hash->cap = new_size;
}

void hash_table_write(HashTable* hash, const char *key, void *value) {
  HashTableEntry *found_entry = hash_table_find_entry(hash, key);
  if (found_entry != NULL) {
    found_entry->value = value;
    return;
  }

  if (hash->cap * HASH_RESIZE_LOAD_FACTOR <= hash->used) {
    hash_table_resize(hash, hash->cap * 2);
  }

  int i = key_hash(key) % hash->cap;

  HashTableEntry *new_entry = malloc(sizeof(HashTableEntry));
  new_entry->key = malloc((strlen(key) + 1) * sizeof(char));
  strcpy(new_entry->key, key);
  new_entry->value = value;
  new_entry->next = NULL;

  hash_table_insert_entry(hash, i, new_entry);
}

void* hash_table_read(HashTable* hash, const char *key) {
  HashTableEntry *entry = hash_table_find_entry(hash, key);
  if (entry != NULL) {
    return entry->value;
  } else {
    return NULL;
  }
}
