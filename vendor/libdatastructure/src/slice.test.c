#include <test/test.h>

#include <datastructure/slice.h>

#define RANGE(idx, arr) (size_t idx = 0; idx < sizeof(arr)/sizeof(arr[0]); idx++)

void test_new(T * t) {
    typedef struct tcase {
		size_t elsize;
		size_t len;
		size_t cap;
	} tcase;

	tcase tcases[] = {
		{ // 0
			.elsize=sizeof(int),
			.len=10,
			.cap=10,
		},
		{ // 1
			.elsize=sizeof(int),
			.len=0,
			.cap=10,
		},
		{ // 1
			.elsize=sizeof(int),
			.len=0,
			.cap=0,
		},
	};

	for RANGE(i, tcases) {
		tcase tc = tcases[i];

		slice_t s = slice_new(tc.elsize, tc.len, tc.cap);

		test_assert(t, slice_len(s) == tc.len);
		test_assert(t, slice_cap(s) == tc.cap);

		for (size_t i = 0; i < slice_len(s); i++) {
			test_assert(t, *(int *) slice_idx(s, i) == 0);
		}

		slice_free(&s);
	}
}

void test_ops(T * t) {
    enum {
        END,
        APPEND,
    };

    struct list_op {
        int op;
        int el;
    };


    typedef struct tcase {
        size_t elsize;
        struct list_op * ops;
        size_t len;
        size_t cap;
        size_t outlen;
        size_t outcap;
        int * out;
    } tcase;

    tcase tcases[] = {
        { // 0
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = APPEND, .el = 1},
                {.op = END},
            },
            .len = 0,
            .cap = 0,
            .outlen = 1,
            .outcap = 1,
            .out = (int[]) {
                1,
            }
        },
        { // 1
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = APPEND, .el = 1},
                {.op = APPEND, .el = 2},
                {.op = END},
            },
            .len = 0,
            .cap = 0,
            .outlen = 2,
            .outcap = 3,
            .out = (int[]) {
                2,
                1,
            }
        },
        { // 3
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = APPEND, .el = 1},
                {.op = APPEND, .el = 2},
                {.op = APPEND, .el = 3},
                {.op = END},
            },
            .len = 0,
            .cap = 0,
            .outlen = 3,
            .outcap = 3,
            .out = (int[]) {
                1,
                2,
                3,
            }
        },
        { // 4
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = APPEND, .el = 1},
                {.op = APPEND, .el = 2},
                {.op = APPEND, .el = 3},
                {.op = END},
            },
            .len = 2,
            .cap = 3,
            .outlen = 5,
            .outcap = 7,
            .out = (int[]) {
                0,
                0,
                1,
                2,
                3,
            }
        },
    };

    for RANGE(i, tcases) {
        tcase tc = tcases[i];

        slice_t s = slice_new(tc.elsize, tc.len, tc.cap);

        test_assert(t, slice_len(s) == tc.len);
        test_assert(t, slice_cap(s) == tc.cap);

        for (struct list_op * op = tc.ops;
                op->op != END;
                op++) {
            switch (op->op) {
            case APPEND:
                s = slice_append(s, &op->el);
                break;
            }
        }

        test_assert(t, slice_len(s) == tc.outlen);
        test_assert(t, slice_cap(s) == tc.outcap);

        for (size_t i = 0; i < tc.len; i++) {
            int * el = (int *)slice_idx(s, i);

            test_assert(t, *el == tc.out[i]);
        }


        slice_free(&s);
    }
}

DECL_TESTS(test_new, test_ops);
