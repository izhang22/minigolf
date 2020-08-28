#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "polygon.h"

const int REALLOC_FACTOR = 2;

typedef struct list {
    void **items;
    size_t length;
    size_t alloc_size;
    free_func_t freer;
} list_t;

void null_free(void *something) {
    // :)
}

list_t *list_init(size_t initial_size, free_func_t freer) {
    if (initial_size == 0) {
        initial_size = 1;
    }

    if (freer == NULL) {
        freer = null_free;
    }

    list_t *list = malloc(sizeof(list_t));
    list->items = malloc(initial_size * sizeof(void *));

    assert(list->items != NULL);
    assert(list != NULL);

    list->length = 0;
    list->alloc_size = initial_size;
    list->freer = freer;
    return list;
}

void list_free(list_t *list) {
    for (size_t i = 0; i < list->length; i++) {
        list->freer(list->items[i]);
    }

    free(list->items);
    free(list);
}

size_t list_size(list_t *list) {
    return list->length;
}

void *list_get(list_t *list, size_t index) {
    size_t length = list->length;
    assert(index < length);

    void **items = list->items;

    void *item_at_i = items[index];
    return item_at_i;
}

void resize_list(list_t *list) {
    void **items =
        realloc(list->items, REALLOC_FACTOR * list->alloc_size * sizeof(void *));

    assert(items != NULL);

    list->alloc_size *= 2;
    list->items = items;
}

void list_add(list_t *list, void *value) {
    assert(value != NULL);
    size_t length = list->length;

    if (length >= list->alloc_size) {
        resize_list(list);
    }

    list->items[length] = value;
    list->length++;
}

void *list_remove(list_t *list, size_t index) {
    size_t length = list->length;
    assert(length != 0);
    assert(index >= 0);

    void **items = list->items;
    list_t *list_item = items[index];

    if (index == length - 1) {
        items[length - 1] = NULL;
    } else {
        memmove(items + index, items + index + 1, (length - index - 1) * sizeof(void *));
    }

    list->length--;

    return list_item;
}
