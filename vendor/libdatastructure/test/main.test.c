//TODO: write test output to a tmp file
//TODO: change "fatal" -> "now"

#include "test.h"

#include <regex.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct T {
	const char * test_name;
	bool failed;
	jmp_buf fatal_ret;
	void (*fn)(T *);
} T;

void test_failf(T * t, const char * fmt, ...) {
	t->failed = true;
	va_list args;
	va_start(args, fmt);
	printf("fail: %s\n\t", t->test_name);
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

void test_fatalf(T * t, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("fail: %s\n\t", t->test_name);
	vprintf(fmt, args);
	printf("\n");
	longjmp(t->fatal_ret, 0);
	va_end(args);
}

void test_skipfatalf(T * t, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("skip: %s\n\t", t->test_name);
	vprintf(fmt, args);
	printf("\n");
	longjmp(t->fatal_ret, 0);
	va_end(args);
}


void test_logf(T * t, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("log: %s: ", t->test_name);
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

static void test_pass(T * t) {
	printf("passed: %s\n", t->test_name);
}

void usage(const char * name, int exit_code) {
	printf("usage: %s [regex]\n", name);
	exit(exit_code);
}

int main(int argc, char **argv) {
	size_t name_start = 0;
	size_t name_end = 0;
	size_t fn_idx = 0;
	size_t runs = 0;
	size_t passes = 0;
	regex_t * regex = NULL;

	switch (argc) {
		case 0:
			// probably shouldn't happen?
		case 1:
			//regex = NULL;
			break;
		case 2:
			if (!argv[1][0]) {
				break;
			}

			regex = (regex_t *) malloc(sizeof(*regex));
			int err = regcomp(regex, argv[1], REG_EXTENDED | REG_NOSUB);
			if (err) {
				char buf[1024];
				regerror(err, regex, buf, 1024);
				printf("regex: %s\n", buf);
				free(regex);
				exit(1);
			}
			break;
		default:
			printf("expected only one arg\n");
			usage(argv[0], 1);
	}

	while (test_names[name_end] && test_fn[fn_idx]) {
		name_start = name_end;

		while(true) {
			char c = test_names[name_start];
			switch (c) {
				case ' ':
				case ',':
				case '\n':
				case '\r':
				case '\t':
					name_start++;
					break;
				default:
					goto found_start;
			}
		}
found_start:

		name_end = name_start;

		while(true) {
			char c = test_names[name_end];
			switch (c) {
				case 0:
				case ' ':
				case ',':
				case '\n':
				case '\r':
				case '\t':
					goto found_end;
				default:
					name_end++;
			}
		}

found_end:
		{};


		// initiallize T
		T t = {0};

		// allocate and store test name
		size_t buflen = name_end - name_start + 1;
		char * buf = (char *) malloc(buflen * sizeof(char));
		strncpy(buf, &test_names[name_start], buflen);
		buf[buflen - 1] = '\0';

		// set testname
		t.test_name = buf;

		// set test fn and increment
		t.fn = test_fn[fn_idx++];

		if (regex && regexec(regex, t.test_name, 0, NULL, 0)) {
			free(buf);
			continue;
		}

		// run the test
		if (!setjmp(t.fatal_ret)) {
			runs++;
			printf("run: %s\n", t.test_name);
			t.fn(&t);
		}

		if (!t.failed) {
			test_pass(&t);
			passes++;
		}

		free(buf);
	}

	if (regex) {
		regfree(regex);
		free(regex);
	}

	if (!runs) {
		printf("no tests to be run\n");
	} else {
		printf("passed %ld/%ld tests\n", passes, runs);
	}

	printf("done\n");
}


