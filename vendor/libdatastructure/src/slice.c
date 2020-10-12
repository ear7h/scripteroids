#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <datastructure/slice.h>

#define BYTEPTR(x) ((uint8_t *) x)

slice_t slice_new(size_t elsize, size_t len, size_t cap) {
    if (cap == 0) {
        cap = len;
    }

    assert(len <= cap);

    void * data = NULL;
	if (cap) {
		data = calloc(elsize, cap);
	}

	assert((data == NULL) == (cap == 0));

    slice_t ret = {
        .elsize = elsize,
        .len = len,
        .cap = cap,
        .data = data,
    };

    return ret;
}

size_t slice_len(slice_t s) {
    return s.len;
}

size_t slice_cap(slice_t s) {
    return s.cap;
}

void * slice_data(slice_t s) {
    return s.data;
}

slice_t slice_append(slice_t s, void * el) {
    if (s.len == s.cap) {
		size_t newcap = 1 + s.cap * 2;
        void * data = realloc(s.data, newcap * s.elsize);
		assert(data);

        s.data = data;
		s.cap = newcap;
    }

    memcpy(BYTEPTR(s.data) + s.elsize * s.len, el, s.elsize);
    s.len++;
    return s;
}

void * slice_idx(slice_t s, size_t idx) {
    return BYTEPTR(s.data) + s.elsize * idx;
}

slice_t slice_slice(slice_t s, size_t start, size_t end) {
    assert(start < s.len && end <= s.len);

    s.data = BYTEPTR(s.data) + s.elsize * start;
    s.cap = s.cap - start;
    s.len = end - start;
    return s;
}

void slice_clear(slice_t s) {
	assert(!"TODO");
}

void slice_free(slice_t * s) {
    free(s->data);
    s->data = 0;
    s->len = 0;
    s->cap = 0;
}

