#include <datastructure/list.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NODE_DATA(nodev) ((void *) (&(nodev)->data))

list_t list_new(size_t elsize) {
    list_t ret = {
        .elsize = elsize,
        .len = 0,
        .first = NULL,
    };

    return ret;
}

static list_node_t * list_node_new(list_t l) {
    return (list_node_t *) malloc(sizeof(list_node_t) + l.elsize);
}

size_t list_len(list_t l) {
    return l.len;
}

static list_node_t * list_node_idx(list_t l, size_t idx) {
    assert(idx >= 0 && idx < l.len);

    while (idx--) {
        l = list_rotate(l);
    }

    return l.first;
}

void list_insert(list_t * l, size_t idx, const void * el) {
    assert(idx >= 0 && idx <= l->len);

    list_node_t * new = list_node_new(*l);
    memcpy(NODE_DATA(new), el, l->elsize);

    if (!l->first) {
        new->prev = new;
        new->next = new;
        l->first = new;
        l->len = 1;
        return;
    }

    list_node_t * old;

    if (idx == l->len || idx == 0) {
        old = l->first;
    } else {
        old = list_node_idx(*l, idx);
    }

    /*
     *                    new
     * ... <- old.prev ->     <- old -> <- old.next -> ...
     * 1. old.prev <- new
     * 2. old.prev -> new
     * 3. new -> old
     * 4. new <- old
     */

    new->prev = old->prev;
    old->prev->next = new;
    new->next = old;
    old->prev = new;

    // if we're inserting to the beginning of the list,
    // we also need to change the first pointer
    if (idx == 0) {
        l->first = new;
    }

    // increment length
    l->len++;
    return;
}

void list_remove(list_t * l, size_t idx) {
    assert(idx >= 0 && idx < l->len);
    if (l->len == 1) {
        free(l->first);
        l->first = NULL;
        l->len = 0;
        return;
    }

    list_node_t * rm = list_node_idx(*l, idx);

    if (idx == 0) {
        l->first = rm->next;
    }

    rm->prev->next = rm->next;
    rm->next->prev = rm->prev;

    free(rm);

    l->len--;
    return;
}

void * list_idx(list_t l, size_t idx) {
    return NODE_DATA(list_node_idx(l, idx));
}


list_t list_rotate(list_t l) {
    assert(l.first);

    l.first = l.first->next;

    return l;
}

void list_free(list_t * l) {
    while (l->len) {
        list_remove(l, 0);
    }

    l->len = 0;
    l->first = NULL;
}

struct list_iter {
    size_t idx;
    list_t l;
};

static bool list_iter_advance(iterator_t * it) {
    struct list_iter * obj = (struct list_iter *) it->obj;

    if (obj->idx == obj->l.len) {
        it->val = NULL;
        return false;
    }

    obj->idx++;
    obj->l = list_rotate(obj->l);
    it->val = NODE_DATA(obj->l.first);

    return true;
}

static void list_iter_free(iterator_t * it) {
    free(it->obj);
    it->obj = NULL;
    it->val = NULL;
}

iterator_t list_iter(list_t l) {
    struct list_iter * obj = (struct list_iter *)
        malloc(sizeof(struct list_iter));

    *obj = (struct list_iter) {
        .idx = 0,
        .l = l,
    };

    return (iterator_t) {
        .obj = obj,
        .val = NULL,
        .advance = list_iter_advance,
        .free = list_iter_free,
    };
}


