#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct hashtable hashtable_t;

hashtable_t *hashtable_create(void);
void hashtable_destroy(hashtable_t *ht);

// Inserts or modifies the value associated with key
void hashtable_set(hashtable_t *ht, char *key, int value);

// Returns true if key is present in ht and if so, fills *value with the value from ht
// otherwise returns false and does not modify *value
bool hashtable_get(hashtable_t *ht, char *key, int *value);

// The number of entries (distinct keys) in ht
int hashtable_size(hashtable_t *ht);

// Use this alongside hashtable_probe
// to iterate through the table
int hashtable_probe_max(hashtable_t *ht);

// permits iterating through the table
// iterate with i from 0 to hashtable_probe_max
// and if this function returns true, key and val are copied to.
// Do not mutate key!
bool hashtable_probe(hashtable_t *ht, int i, char **key, int *val);

uint32_t fibonacci32_reduce(uint32_t hash, int bits);
uint32_t rotate_left(uint32_t value, uint32_t count);
uint32_t fxhash32_step(uint32_t hash, uint32_t value);
uint32_t fxhash32_hash(uint8_t *data, int n);
void rehash(hashtable_t *ht);
int hashtable_collision(hashtable_t *ht);
