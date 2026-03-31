#include "test_framework.h"

// Global test counters
int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

int run_test_case(const test_case_t* test)
{
    printf("  Running %s... ", test->name);
    tests_run++;
    
    if (test->func()) {
        printf("PASS\n");
        tests_passed++;
        return 1;
    } else {
        tests_failed++;
        return 0;
    }
}

int run_test_suite(const test_suite_t* suite)
{
    printf("Running test suite: %s\n", suite->name);
    
    int passed = 0;
    for (int i = 0; i < suite->count; i++) {
        if (run_test_case(&suite->tests[i])) {
            passed++;
        }
    }
    
    printf("Suite results: %d/%d passed\n\n", passed, suite->count);
    return passed == suite->count;
}

void print_test_summary(void)
{
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
}
