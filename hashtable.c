#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "hashtable.h"

#define BIGRAM_SIZE 256

typedef struct hashtable_entry {
    char *key;
    int value;
} hashtable_entry_t;

typedef struct hashtable {
    hashtable_entry_t *entries;
    int table_size;
    int n_entries;
    int bits;
} hashtable_t;

uint32_t fibonacci32_reduce(uint32_t hash, int bits) {
    const uint32_t factor32 = 2654435769;
    uint32_t value = (uint32_t)(hash * factor32) >> (32 - bits);
    return value;
}

uint32_t rotate_left(uint32_t value, uint32_t count) {
    return value << count | value >> (32 - count);
}

uint32_t fxhash32_step(uint32_t hash, uint32_t value) {
    const uint32_t key = 0x27220a95;
    // const uint64_t key = 0x517cc1b727220a95;
    return (rotate_left(hash, 5) ^ value) * key;
}

uint32_t fxhash32_hash(uint8_t *data, int n) {
    uint32_t hash = 0;
    int count = 0;
    for (int i = 0; i < n / 4; i++) {
        uint32_t number;
        memcpy(&number, data, sizeof(number));
        hash = fxhash32_step(hash, number);
        data += 4;
    }

    for (int i = 0; i < n % 4; i++) {
        hash = fxhash32_step(hash, data[i]);
    }
    return hash;
}

hashtable_t *hashtable_create(void) {
    hashtable_t *ht = malloc(sizeof(hashtable_t));
    ht->table_size = 128;
    ht->entries = calloc(ht->table_size, sizeof(hashtable_entry_t));
    ht->bits = 7;
    ht->n_entries = 0;
    return ht;
}

void hashtable_set(hashtable_t *ht, char *key, int value) {
    if ((double)ht->n_entries / (double)ht->table_size >= 0.5) {
        int numCol1 = hashtable_collision(ht);
        rehash(ht);
        int numCol2 = hashtable_collision(ht);
        printf("Rehashing reduced collisions from %d to %d\n", numCol1, numCol2);
    }
    int n = (int)strlen(key);
    uint32_t hash = fibonacci32_reduce(fxhash32_hash((uint8_t *)key, n), ht->bits);
    int count = 0;
    while (true && count < ht->table_size) {
        if (!ht->entries[hash].key) {
            ht->entries[hash].key = key;
            ht->entries[hash].value = value;
            ht->n_entries++;
            return;
        }
        if (!strcmp(ht->entries[hash].key, key)) {
            ht->entries[hash].value = value;
            return;
        }
        hash = (hash + 1) % ht->table_size;
        count++;
    }
    printf("n_entries: %d\n", ht->n_entries);
    fprintf(stderr, "currently in infinite loop in hashtable_set\n");
    exit(1);
}

bool hashtable_get(hashtable_t *ht, char *key, int *value) {
    int n = (int)strlen(key);
    uint32_t hash = fibonacci32_reduce(fxhash32_hash((uint8_t *)key, n), ht->bits);
    int count = 0;
    while (true && count < ht->table_size) {
        if (!ht->entries[hash].key) {
            return false;
        }
        if (!strcmp(ht->entries[hash].key, key)) {
            *value = ht->entries[hash].value;
            return true;
        }
        hash = (hash + 1) % ht->table_size;
        count++;
    }
    printf("n_entries: %d\n", ht->n_entries);
    fprintf(stderr, "currently in infinite loop in hashtable_get\n");
    exit(1);
}

void rehash(hashtable_t *ht) {
    hashtable_t *ht2 = malloc(sizeof(hashtable_t));
    ht2->table_size = 2 * ht->table_size;
    ht2->entries = calloc(ht2->table_size, sizeof(hashtable_entry_t));
    ht2->n_entries = 0;
    ht2->bits = ht->bits + 1;
    for (int i = 0; i < ht->table_size; i++) {
        if (ht->entries[i].key) {
            hashtable_set(ht2, ht->entries[i].key, ht->entries[i].value);
        }
    }
    free(ht->entries);
    *ht = *ht2;
    free(ht2);
}

void hashtable_destroy(hashtable_t *ht) {
    for (int i = 0; i < ht->table_size; i++) {
        if (ht->entries[i].key) {
            free(ht->entries[i].key);
        }
    }
    free(ht->entries);
    free(ht);
}

int hashtable_size(hashtable_t *ht) {
    return ht->n_entries;
}

int hashtable_probe_max(hashtable_t *ht) {
    return ht->table_size;
}

bool hashtable_probe(hashtable_t *ht, int i, char **key, int *val) {
    if (ht->entries[i].key) {
        *val = ht->entries[i].value;
        *key = ht->entries[i].key;
        return true;
    }
    return false;
}

int hashtable_collision(hashtable_t *ht) {
    int table[ht->table_size];
    for (int i = 0; i < ht->table_size; i++) {
        table[i] = 0;
    }
    int numCollisions = 0;
    for (int i = 0; i < ht->table_size; i++) {
        if (ht->entries[i].key) {
            int n = (int)strlen(ht->entries[i].key);
            uint32_t hash = fibonacci32_reduce(fxhash32_hash((uint8_t *)ht->entries[i].key, n),
                                               ht->bits);
            table[hash] += 1;
        }
    }
    for (int i = 0; i < ht->table_size; i++) {
        if (ht->entries[i].key) {
            if (table[i] > 1) {
                numCollisions += table[i] - 1;
            }
        }
    }
    return numCollisions;
}
