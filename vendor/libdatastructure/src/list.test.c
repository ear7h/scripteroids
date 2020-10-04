#include <datastructure/list.h>

#include <test/test.h>

#define RANGE(idx, arr) (size_t idx = 0; idx < sizeof(arr)/sizeof(arr[0]); idx++)

void test_new(T * t) {
    typedef struct tcase {
        size_t elsize;
    } tcase;

    tcase tcases[] = {
        { // 0
            .elsize = sizeof(int),
        },
        { // 1
            .elsize = sizeof(uint8_t[256]),
        },
    };

    for RANGE(i, tcases) {
        tcase tc = tcases[i];

        list_t l = list_new(tc.elsize);

        test_assert(t, list_len(l) == 0);

        list_free(&l);
    }
}

void test_ops(T * t) {
    enum {
        END,
        INSERT,
        REMOVE,
        ROTATE
    };

    struct list_op {
        int op;
        size_t idx;
        int el;
    };


    typedef struct tcase {
        size_t elsize;
        struct list_op * ops;
        size_t len;
        int * out;
    } tcase;

    tcase tcases[] = {
        { // 0
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = END},
            },
            .len = 1,
            .out = (int[]) {
                1,
            }
        },
        { // 1
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = INSERT, .idx = 0, .el = 2},
                {.op = END},
            },
            .len = 2,
            .out = (int[]) {
                2,
                1,
            }
        },
        { // 2
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = INSERT, .idx = 0, .el = 2},
                {.op = REMOVE, .idx = 0},
                {.op = END},
            },
            .len = 1,
            .out = (int[]) {
                1,
            }
        },
        { // 3
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = INSERT, .idx = 0, .el = 2},
                {.op = INSERT, .idx = 0, .el = 3},
                {.op = REMOVE, .idx = 1},
                {.op = END},
            },
            .len = 2,
            .out = (int[]) {
                3,
                1,
            }
        },
        { // 4
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = INSERT, .idx = 0, .el = 2},
                {.op = INSERT, .idx = 0, .el = 3},
                {.op = REMOVE, .idx = 1},
                {.op = ROTATE},
                {.op = END},
            },
            .len = 2,
            .out = (int[]) {
                1,
                3,
            }
        },
        { // 5
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = ROTATE},
                {.op = END},
            },
            .len = 1,
            .out = (int[]) {
                1,
            }
        },
        { // 6
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = REMOVE, .idx = 0},
                {.op = END},
            },
            .len = 0,
            .out = NULL,
        },
        { // 7
            .elsize = sizeof(int),
            .ops = (struct list_op []) {
                {.op = INSERT, .idx = 0, .el = 1},
                {.op = INSERT, .idx = 1, .el = 2},
                {.op = INSERT, .idx = 2, .el = 3},
                {.op = END},
            },
            .len = 3,
            .out = (int[]) {
                1,
                2,
                3,
            }
        },
    };

    for RANGE(i, tcases) {
        tcase tc = tcases[i];

        list_t l = list_new(tc.elsize);

        test_assert(t, list_len(l) == 0);

        for (struct list_op * op = tc.ops;
                op->op != END;
                op++) {
            switch (op->op) {
            case INSERT:
                list_insert(&l, op->idx, &op->el);
                break;

            case REMOVE:
                list_remove(&l, op->idx);
                break;

            case ROTATE:
                l = list_rotate(l);
                break;
            }
        }

        test_assert(t, list_len(l) == tc.len);

        for (size_t i = 0; i < tc.len; i++) {
            int * el = (int *)list_idx(l, i);

            test_assert(t, *el == tc.out[i]);
        }


        list_free(&l);
    }
}

DECL_TESTS(test_new, test_ops);
