#pragma once

#include <stdbool.h>

// iterator_t is an interface for iterating items in a container.
// .free must be called on the iterator once it is no longer needed;
// though, the ITERATOR_RANGE macro (and wrappers for it) make this call
// already.
typedef struct iterator {
    void * obj;
    void * val;
    bool (*advance)(struct iterator *);
    void (*free)(struct iterator *);
} iterator_t;

// ITERATOR_RANGE expands to a loop over the itertator iterv where valdcl
// is a declaration of a variable that will be accesible to the series of
// statements in body.
//
// example:
//
//      slice_t my_int_slice = slice_new(sizeof(int), 0, 0);
//
//      for (int i = 0; i <= 3; i++) {
//          my_int_slice = slice_append(my_int_slice, &i);
//      }
//
//      int sum = 0;
//
//      SLICE_RANGE(int * v, slice_iterator(my_int_slice), sum += *v);
//
//      assert(sum == 6);
//
#define ITERATOR_RANGE(valdcl, iterv, body) { \
    iterator_t it = iterv; \
    while (it.advance(&it)) { \
        valdcl = it.val; \
        body; \
    } \
    it.free(&it); \
}

// if the iterv supplies is a function call like slice_iter(...)
// which creates a new iterator object, then the compiler will
// probably not inline the function call. However, if it is
// a literal with the methods being `static` declared in the
// header file, the compiler might be smart enough to inline it

