#pragma once
#include <stdarg.h>
#include <stddef.h>

// T is an object that stores test state and is needed
// to execute testing operations like logging and failing.
typedef struct T T;


// the follwoing two declarations are used for
// listing the tests to run. This is actually
// defined in the DECL_TESTS macro.

// test_fn is an array of test functions
extern void (*test_fn[])(T *);

// test_names contains the names of the tests
// for logging success and failure
extern char * test_names; // stringized

// DECL_TESTS is a macro for declaring the tests in
// a test file. Only functions names should be provided
// (no expressions).
#define DECL_TESTS(...) \
    void (*test_fn[])(T *) = {__VA_ARGS__, NULL}; \
    char * test_names = #__VA_ARGS__

// test_failf sets the test state to failed
// with a message. Execution of the current
// test continues.
void test_failf(T *, const char *, ...);

// test_fatalf sets the test state to failed
// with a message and stops execution of the current
// test.
void test_fatalf(T *, const char *, ...);

#define test_assert(test, expr) if (!(expr)) {\
    test_fatalf(test, "%s:%d: assertion failed: %s", \
        __FILE__, \
        __LINE__, \
        #expr);\
}

// test_skipfatalf skipst the test
// with a message and stops execution of the current
// test.
void test_skipfatalf(T *, const char *, ...);

// test_logf writes a message to the output log.
void test_logf(T *, const char *, ...);
