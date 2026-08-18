/*
 * test_framework.h - minimal host-side unit test framework (no dependencies)
 */
#ifndef TEST_FRAMEWORK_H_
#define TEST_FRAMEWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tf_failures = 0;
static int tf_checks = 0;
static const char *tf_case = "";

#define TF_CASE(name)                          \
    do {                                       \
        tf_case = (name);                      \
        printf("  [ RUN  ] %s\n", tf_case);    \
    } while (0)

#define TF_CHECK(cond)                                                         \
    do {                                                                       \
        tf_checks++;                                                           \
        if (!(cond)) {                                                         \
            tf_failures++;                                                     \
            printf("  [ FAIL ] %s:%d: %s (case: %s)\n", __FILE__, __LINE__,    \
                   #cond, tf_case);                                            \
        }                                                                      \
    } while (0)

#define TF_CHECK_EQ(actual, expected)                                          \
    do {                                                                       \
        long long _a = (long long)(actual);                                    \
        long long _e = (long long)(expected);                                  \
        tf_checks++;                                                           \
        if (_a != _e) {                                                        \
            tf_failures++;                                                     \
            printf("  [ FAIL ] %s:%d: %s == %lld, expected %lld (case: %s)\n", \
                   __FILE__, __LINE__, #actual, _a, _e, tf_case);              \
        }                                                                      \
    } while (0)

#define TF_CHECK_MEM(actual, expected, size)                                    \
    do {                                                                        \
        tf_checks++;                                                            \
        if (memcmp((actual), (expected), (size)) != 0) {                        \
            tf_failures++;                                                      \
            printf("  [ FAIL ] %s:%d: memory mismatch of %zu bytes (case: %s)\n", \
                   __FILE__, __LINE__, (size_t)(size), tf_case);                \
        }                                                                       \
    } while (0)

#define TF_MAIN_END()                                                          \
    printf("%s: %d checks, %d failures\n", tf_failures ? "FAILED" : "PASSED",  \
           tf_checks, tf_failures);                                            \
    return tf_failures ? 1 : 0;

#endif /* TEST_FRAMEWORK_H_ */
