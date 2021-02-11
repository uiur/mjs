typedef struct HashTable {
  unsigned int cap;
  unsigned int used;
  struct HashTableEntry **entries;
} HashTable;

typedef struct HashTableEntry {
  struct HashTableEntry *next;
  char *key;
  void *value;
} HashTableEntry;

HashTable* hash_table_new();
void hash_table_write(HashTable *hash, const char *key, void *value);
void* hash_table_read(HashTable *hash, const char *key);
