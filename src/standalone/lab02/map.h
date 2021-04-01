#pragma once

#include <stdbool.h>
#include "../../lib/kernel/list.h"


// FRÅGA: Hur ska vi sköta felhantering?
#define PANIC(message) do { printf("PANIC: %s\n", message); exit(1); } while (0)

typedef int key_t;
typedef char* value_t;

struct association
{
    key_t key;
    value_t value;
    struct list_elem elem;
};

struct map
{
    struct list content;
    key_t next_key;
};

void map_init(struct map*);
key_t map_insert(struct map*, const value_t);
value_t map_find(struct map*, const key_t);
value_t map_remove(struct map*, const key_t);

void map_for_each(struct map*, void (*exec)(key_t, value_t, int aux), int aux);
void map_remove_if(struct map* this, bool (*cond)(key_t, value_t, int aux), int aux);