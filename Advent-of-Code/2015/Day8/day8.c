#include <stdio.h>
#include <string.h>

/*
 * Advent of Code 2015 - Day 8: Matchsticks
 * C64 Edition
 */

unsigned int calculate_memory_length(const char* s) {
    unsigned int len = 0;
    unsigned int i = 0;
    unsigned int content_len;

    /* The input string includes the surrounding quotes */
    content_len = strlen(s);
    if (content_len < 2) return 0;

    /* Process characters between the quotes */
    i = 1;
    while (i < content_len - 1) {
        if (s[i] == '\\') {
            if (s[i+1] == '\\' || s[i+1] == '\"') {
                len++;
                i += 2;
            } else if (s[i+1] == 'x') {
                /* Hex escape \xHH */
                len++;
                i += 4;
            }
        } else {
            len++;
            i++;
        }
    }
    return len;
}

unsigned int calculate_encoded_length(const char* s) {
    unsigned int len = 2; /* Starting quotes */
    unsigned int i = 0;
    unsigned int content_len = strlen(s);

    for (i = 0; i < content_len; i++) {
        if (s[i] == '\"' || s[i] == '\\') {
            len += 2;
        } else {
            len += 1;
        }
    }
    return len;
}

void run_test(const char* test_str, int expected_mem, int expected_enc) {
    unsigned int code_len = strlen(test_str);
    unsigned int mem_len = calculate_memory_length(test_str);
    unsigned int enc_len = calculate_encoded_length(test_str);
    int part1_diff = (int)code_len - (int)mem_len;
    int part2_diff = (int)enc_len - (int)code_len;

    printf("STR: %s\n", test_str);
    printf("CODE: %u, MEM: %u, ENC: %u\n", code_len, mem_len, enc_len);
    printf("P1 DIFF: %d (EXP: %d) %s\n", part1_diff, expected_mem, (part1_diff == expected_mem ? "OK" : "FAIL"));
    printf("P2 DIFF: %d (EXP: %d) %s\n", part2_diff, expected_enc, (part2_diff == expected_enc ? "OK" : "FAIL"));
    printf("--------------------------\n");
}

int main(void) {
    printf("AOC 2015 DAY 8 - MATCHSTICKS\n");
    printf("--------------------------\n");

    /* Test Cases */
    /* Note: In C strings, we need to escape backslashes and quotes to represent the literals correctly */
    
    /* Case 1: "" */
    run_test("\"\"", 2, 4);

    /* Case 2: "abc" */
    run_test("\"abc\"", 2, 4);

    /* Case 3: "aaa\"aaa" */
    run_test("\"aaa\\\"aaa\"", 3, 6);

    /* Case 4: "\x27" */
    run_test("\"\\x27\"", 5, 5);

    printf("TESTS COMPLETED.\n");

    return 0;
}
