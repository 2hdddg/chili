#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "registry.h"


struct entry {
    bool         in_use;
    chili_handle handle;
    char         *key;
};

struct instance {
    struct entry *entries;
    int          capacity;
};


static int _increase_capacity(struct instance *instance,
                              int num)
{
    int size;
    struct entry *new_entries;

    /* Current size */
    size  = instance->capacity * sizeof(struct entry);
    /* New size */
    size += num * sizeof(struct entry);

    new_entries = realloc(instance->entries, size);

    if (new_entries){
        instance->entries   = new_entries;
        instance->capacity += num;

        return 1;
    }

    return -1;
}

int chili_reg_create(int capacity, chili_handle *h)
{
    struct entry    *entries;
    struct instance *instance;

    capacity  = capacity > 0 ? capacity : 1;

    entries = calloc(capacity, sizeof(struct entry));
    if (entries == NULL){
        return -1;
    }

    instance = malloc(sizeof(struct instance));
    if (instance == NULL){
        free(entries);
        return -1;
    }

    instance->entries  = entries;
    instance->capacity = capacity;

    *h = instance;
    return 1;
}

int chili_reg_add(chili_handle h,
                  const char *key,
                  chili_handle client_handle)
{
    struct instance *instance = (struct instance*)h;
    struct entry    *entries  = instance->entries;
    struct entry    *entry    = NULL;
    int             capacity  = instance->capacity;

    for (int i = 0; i < capacity; i++){
        if (entries[i].in_use){
            /* Check that key is unique */
            if (strcmp(entries[i].key, key) == 0){
                return -1;
            }
        }
        else if (entry == NULL) {
            /* Found unused entry */
            entry = &entries[i];
        }
    }

    /* No room, increase capacity */
    if (!entry){
        if (_increase_capacity(instance, 1) < 0){
            return -1;
        }

        entry = &instance->entries[capacity];
    }

    if (entry){
        entry->in_use = true;
        entry->handle = client_handle;
        entry->key    = malloc(strlen(key) + 1);

        if (entry->key == NULL){
            return -1;
        }

        strcpy(entry->key, key);
        return 1;
    }

    return -1;
}

chili_handle chili_reg_next(chili_handle h, int *token)
{
    struct instance *instance = (struct instance*)h;
    struct entry    *entry    = instance->entries;

    int index = *token;

    if (index >= instance->capacity){
        return NULL;
    }

    entry += *token;

    /* Find next used entry */
    for (; index < instance->capacity; index++){
        if (entry->in_use){
            index++;
            *token = index;
            return entry->handle;
        }
        entry++;
    }

    return NULL;
}

chili_handle chili_reg_find(chili_handle h, const char *key)
{
    struct instance *instance = (struct instance*)h;
    struct entry    *entries  = instance->entries;

    for (int i = 0; i < instance->capacity; i++){
        if (entries[i].in_use){
            if (strcmp(entries[i].key, key) == 0){
                return entries[i].handle;
            }
        }
    }
    return NULL;
}

void chili_reg_destroy(chili_handle h)
{
    struct instance *instance = (struct instance*)h;
    struct entry    *entries  = instance->entries;

    /* Free all keys */
    for (int i = 0; i < instance->capacity; i++){
        if (entries[i].in_use){
            free(entries[i].key);
        }
    }

    free(instance->entries);
    free(instance);
}
