#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define auto_rehash_const 0.5

/*
 * value
 */

enum value_kind value_get_kind(const struct value *self) {
    return self->kind;
}

bool value_is_nil(const struct value *self) {
    return self->kind == VALUE_NIL;
}

bool value_is_boolean(const struct value *self) {
    return self->kind == VALUE_BOOLEAN;
}

bool value_is_integer(const struct value *self) {
    return self->kind == VALUE_INTEGER;
}

bool value_is_real(const struct value *self) {
    return self->kind == VALUE_REAL;
}

bool value_is_custom(const struct value *self) {
    return self->kind == VALUE_CUSTOM;
}

void value_set_nil(struct value *self) {
    self->kind = VALUE_NIL;
}

void value_set_boolean(struct value *self, bool val) {
    self->kind = VALUE_BOOLEAN;
    self->as.boolean = val;
}

void value_set_integer(struct value *self, int64_t val) {
    self->kind = VALUE_INTEGER;
    self->as.integer = val;
}

void value_set_real(struct value *self, double val) {
    self->kind = VALUE_REAL;
    self->as.real = val;
}

void value_set_custom(struct value *self, void *val) {
    self->kind = VALUE_CUSTOM;
    self->as.custom = val;
}


bool value_get_boolean(const struct value *self) {
    assert(self->kind == VALUE_BOOLEAN);
    return self->as.boolean;
}

int64_t value_get_integer(const struct value *self) {
    assert(self->kind == VALUE_INTEGER);
    return self->as.integer;
}

double value_get_real(const struct value *self) {
    assert(self->kind == VALUE_REAL);
    return self->as.real;
}

void *value_get_custom(const struct value *self) {
    assert(self->kind == VALUE_CUSTOM);
    return self->as.custom;
}


struct value value_make_nil(void) {
    struct value res;
    res.kind = VALUE_NIL;
    return res;
}

struct value value_make_boolean(bool val) {
    struct value res;
    res.kind = VALUE_BOOLEAN;
    res.as.boolean = val;
    return res;
}

struct value value_make_integer(int64_t val) {
    struct value res;
    res.kind = VALUE_INTEGER;
    res.as.integer = val;
    return res;
}

struct value value_make_real(double val) {
    struct value res;
    res.kind = VALUE_REAL;
    res.as.real = val;
    return res;
}

struct value value_make_custom(void *val) {
    struct value res;
    res.kind = VALUE_CUSTOM;
    res.as.custom = val;
    return res;
}

/*
 * hashtable
 */

void hashtable_create(struct hashtable *self) {
    if (self != NULL) {
        self->buckets = calloc(HASHTABLE_INITIAL_SIZE, sizeof(struct bucket *));
        self->count = 0;
        self->size = HASHTABLE_INITIAL_SIZE;
    }
}

void hashtable_destroy(struct hashtable *self) {
    if (self != NULL) {
        for (size_t i = 0; i < self->size; ++i) {
            struct bucket *current = self->buckets[i];
            while (current != NULL) {
                struct bucket *next = current->next;
                free(current->key);
                free(current);
                current = next;
            }
        }
        free(self->buckets);
    }
}

size_t hashtable_get_count(const struct hashtable *self) {
    return (self != NULL) ? self->count : 0;
}

size_t hashtable_get_size(const struct hashtable *self) {
    return (self != NULL) ? self->size : 0;
}

size_t fnv1a_hash(const char *key) {
    size_t hash = 14695981039346656037ULL;
    while (*key) {
        hash ^= (size_t)(*key++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

char *copy_string(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    char *copy = malloc(length + 1); // +1 pour le caractère de fin de chaîne
    for (size_t i = 0; i < length; i++) {
        copy[i] = str[i];
    }
    copy[length] = '\0';
    return copy;
}


bool hashtable_insert(struct hashtable *self, const char *key, struct value value) {
    size_t index = fnv1a_hash(key) % self->size;
    struct bucket *current = self->buckets[index];

    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return false; // Clé déjà présente, mise à jour de la valeur
        }
        current = current->next;
    }

    // Insertion d'une nouvelle clé
    struct bucket *new_bucket = malloc(sizeof(struct bucket));
    new_bucket->key = copy_string(key);
    new_bucket->value = value;
    new_bucket->next = self->buckets[index];
    self->buckets[index] = new_bucket;
    self->count++;


    if (self->count > (size_t)(self->size * auto_rehash_const)) {
        hashtable_rehash(self);
    }

    return true;
}

bool hashtable_remove(struct hashtable *self, const char *key) {
    size_t index = fnv1a_hash(key) % self->size;
    struct bucket *current = self->buckets[index];
    struct bucket *previous = NULL;

    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (previous) {
                previous->next = current->next;
            } else {
                self->buckets[index] = current->next;
            }
            free(current->key);
            free(current);
            self->count--;
            return true;
        }
        previous = current;
        current = current->next;
    }

    return false; // Clé non trouvée
}

bool hashtable_contains(const struct hashtable *self, const char *key) {
    if (self == NULL) return false;
    size_t index = fnv1a_hash(key) % self->size;
    struct bucket *current = self->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void hashtable_rehash(struct hashtable *self) {
    size_t new_size = self->size * 2;
    struct bucket **new_buckets = calloc(new_size, sizeof(struct bucket *));

    for (size_t i = 0; i < self->size; ++i) {
        struct bucket *current = self->buckets[i];
        while (current != NULL) {
            size_t new_index = fnv1a_hash(current->key) % new_size;
            struct bucket *next = current->next;

            // Insérez le seau dans le nouveau tableau
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;

            current = next;
        }
    }

    free(self->buckets);
    self->buckets = new_buckets;
    self->size = new_size;
}


void hashtable_set_nil(struct hashtable *self, const char *key) {
    struct value val = value_make_nil();
    hashtable_insert(self, key, val);
}

void hashtable_set_boolean(struct hashtable *self, const char *key, bool val) {
    struct value v = value_make_boolean(val);
    hashtable_insert(self, key, v);
}

void hashtable_set_integer(struct hashtable *self, const char *key, int64_t val) {
    struct value v = value_make_integer(val);
    hashtable_insert(self, key, v);
}

void hashtable_set_real(struct hashtable *self, const char *key, double val) {
    struct value v = value_make_real(val);
    hashtable_insert(self, key, v);
}

void hashtable_set_custom(struct hashtable *self, const char *key, void *val) {
    struct value v = value_make_custom(val);
    hashtable_insert(self, key, v);
}


struct value hashtable_get(struct hashtable *self, const char *key) {
    if (self == NULL) return value_make_nil();
    size_t index = fnv1a_hash(key) % self->size;
    struct bucket *current = self->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return value_make_nil();
}

