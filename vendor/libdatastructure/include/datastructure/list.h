#pragma once

#include <stddef.h>
#include <stdint.h>

#include <datastructure/iterator.h>

// list_node_t is a doubly-linked list node,
// with next and prev pointers.
typedef struct list_node {
    struct list_node * next;
    struct list_node * prev;
    max_align_t data[];
} list_node_t;

// list_t is a circular, doubly-linked list.
typedef struct list {
    size_t elsize;
    size_t len;
    list_node_t * first;
} list_t;

// list_new creates a new list with 0 items.
list_t list_new(size_t elsize);

// list_len returns the length of the list.
size_t list_len(list_t l);

// list_insert inserts an item at the specified idx
// pushing the current element back, if it exists.
void list_insert(list_t * l, size_t idx, const void * el);

// list_remove removes the element at the specified
// idx.
void list_remove(list_t * l, size_t idx);

// list_idx returns a pointer to the element data.
void * list_idx(list_t l, size_t idx);

// list_rotate rotates the list left by one element,
// that is, the new first element points to old first's
// next element.
list_t list_rotate(list_t l);

// list_free frees all associated heap memory from l and sets l to
// be a list with 0 elements.
void list_free(list_t * l);

// slice_iter crates a new iterator for the list.
iterator_t list_iter(list_t l);

// LIST_RANGE is a wrapper over ITERATOR_RANGE (see iterator.h) where the
// second argument is a slice rather than an iterator.
#define LIST_RANGE(valdcl, listv, body) \
    ITERATOR_RANGE(valdcl, list_iter(listv), body)
