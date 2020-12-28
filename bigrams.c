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

void read_word(FILE *f, char *word, int buffer_size);

void read_word(FILE *f, char *word, int buffer_size) {
    int i = 0;
    char letter = (char)fgetc(f);
    while (isalpha(letter) && i < buffer_size) {
        word[i] = letter;
        i++;
        letter = (char)fgetc(f);
    }
    word[i] = '\0';
}

int main(int argc, char **argv) {
    hashtable_t *ht = hashtable_create();
    FILE *f = fopen("PoGP.txt", "r");
    if (!f) {
        perror("Could not open book.txt");
        exit(1);
    }

    int buffer_size = 256;
    char *word1 = malloc(buffer_size);
    char *word2 = malloc(buffer_size);
    read_word(f, word1, buffer_size);
    while (!feof(f)) {
        int count = 1;
        read_word(f, word2, buffer_size);
        char *bigram = malloc(BIGRAM_SIZE);
        if (*word2) {
            snprintf(bigram, BIGRAM_SIZE, "%s %s", word1, word2);
            strcpy(word1, word2);
            if (!hashtable_get(ht, bigram, &count)) {
                hashtable_set(ht, bigram, count);
            } else {
                count++;
                hashtable_set(ht, bigram, count);
            }
        } else {
            free(bigram);
        }
        if (count > 1) {
            free(bigram);
        }
    }
    fclose(f);
    int n = hashtable_probe_max(ht);
    int val200 = 0;
    for (int i = 0; i < n; i++) {
        char *key;
        int val;
        if (hashtable_probe(ht, i, &key, &val) && val >= 190) {
            printf("Bigram '%s' has count of %d\n", key, val);
            val200++;
        }
    }
    if (!val200) {
        for (int i = 0; i < n; i++) {
            char *key;
            int val;
            if (hashtable_probe(ht, i, &key, &val)) {
                printf("Bigram '%s' has count of %d\n", key, val);
            }
        }
    }
    printf("Total of %d different bigrams recorded\n", hashtable_size(ht));
    hashtable_destroy(ht);
    free(word1);
    free(word2);
    return 0;
}
