#include "../bloom.h"
#include <stdio.h>

int main()
{
    bloom_t bloom = bloom_create(15);

    printf("Should be 0: %d\n", bloom_test(bloom, "hello world"));
    bloom_add(bloom, "hello world");
    bloom_add(bloom, "hello C");
    printf("Should be 1: %d\n", bloom_test(bloom, "hello world"));
    printf("Should be 0: %d\n", bloom_test(bloom, "hello d"));
    printf("Should be 1: %d\n", bloom_test(bloom, "hello C"));
    printf("Should (probably) be 0: %d\n", bloom_test(bloom, "world hello"));
    return 0;
}
