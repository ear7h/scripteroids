#pragma once

#include <stddef.h>
#include <stdint.h>

#include <datastructure/iterator.h>

// slice_t is a resizeable array.
typedef struct slice {
    size_t elsize;
    size_t len;
    size_t cap;
    void * data;
} slice_t;

// slice_new returns a new slice for elements of elsize, length len,
// and capacity cap.
slice_t slice_new(size_t elsize, size_t len, size_t cap);

// slice_len returns the length of s
size_t slice_len(slice_t s);

// slice_cap returns the capacity of s
size_t slice_cap(slice_t s);

// slice_append appends items to the end of a slice
slice_t slice_append(slice_t s, void * el);

// slice_idx returns a pointer to the data at index idx
void * slice_idx(slice_t s, size_t idx);

// slice_slice returns a subslice of s
slice_t slice_slice(slice_t s, size_t start, size_t end);

// slice_iter creates a new iterator for the slice
iterator_t slice_iter(slice_t s);

// slice_free frees heap data associated with a slice and sets
// s to have 0 length and 0 capacity.
void slice_free(slice_t * s);

// SLICE_RANGE is a wrapper over ITERATOR_RANGE (see iterator.h) where the
// second argument is a slice rather than an iterator.
//
// TODO: build the iterator in the macro using header-defined,
// static methods for advance and free.
#define SLICE_RANGE(valdcl, slicev, body) \
    ITERATOR_RANGE(valdcl, slice_iter(slicev), body)

