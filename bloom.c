#include"bloom.h"
#include"stdint.h"
#include"stdlib.h"

bloom_t bloom_create(size_t size)
{
    bloom_t res = calloc(1, sizeof(struct bloom_filter));
    res->size = size;
    res->bits = malloc(size);
    return res;
}

void bloom_free(bloom_t filter)
{
    if (filter) {
        while (filter->func) {
            struct bloom_hash *h = filter->func;
            filter->func = h->next;
            free(h);
        }
        free(filter->bits);
        free(filter);
    }
}

void bloom_add_hash(bloom_t filter, hash_function func)
{
    struct bloom_hash *h = calloc(1, sizeof(struct bloom_hash));
    h->func = func;
    struct bloom_hash *last = filter->func;
    while (last && last->next) {
        last = last->next;
    }
    if (last) {
        last->next = h;
    } else {
        filter->func = h;
    }
}

void bloom_add(bloom_t filter, const void *item)
{
    struct bloom_hash *h = filter->func;
    uint8_t *bits = filter->bits;
    while (h) {
        unsigned int hash = h->func(item);
        hash %= filter->size;
        bits[hash] = 1 ;
        h = h->next;
    }
}

bool bloom_test(bloom_t filter, const void *item)
{
    struct bloom_hash *h = filter->func;
    uint8_t *bits = filter->bits;
    while (h) {
        unsigned int hash = h->func(item);
        hash %= filter->size ;
        if (!(bits[hash])) {
            return false;
        }
        h = h->next;
    }
    return true;
}
