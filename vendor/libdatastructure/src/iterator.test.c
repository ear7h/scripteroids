#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <datastructure/iterator.h>

#include <test/test.h>

struct int_array_iter_obj {
    int * array;
    size_t len;
    size_t idx;
};

bool int_array_iter_advance(iterator_t * it) {
    struct int_array_iter_obj * obj = it->obj;

    if (obj->idx >= obj->len) {
        return false;
    }

    it->val = &obj->array[obj->idx++];
    return true;
}

void int_array_iter_free(iterator_t * it) {
    free(it->obj);
    it->obj = NULL;
    it->val = NULL;
}

iterator_t int_array_iter(int * array, size_t len) {
    struct int_array_iter_obj * obj = (struct int_array_iter_obj *) malloc(
            sizeof(struct int_array_iter_obj));
    *obj = (struct int_array_iter_obj) {
        .array = array,
        .len = len,
        .idx = 0,

    };

    iterator_t ret = {
        .obj = obj,
        .val = NULL,
        .advance = int_array_iter_advance,
        .free = int_array_iter_free,
    };

    return ret;
}

void int_array_iterator(T * t) {

    int array[10] = {1, 2, 3};

    int sum = 0;
    ITERATOR_RANGE(int * x, int_array_iter(array, 10), sum += *x)

    if (sum != 6) {
        test_failf(t, "expected sum 6 got %d", sum);
    }
}

DECL_TESTS(int_array_iterator);
