/*
 * Unit tests for utils/config.c (read_port_from_config).
 *
 * Writes temporary files to /tmp and cleans them up after each test.
 * Linked against tests/logs_stub.c to avoid the zlog dependency.
 */

#include "unity/unity.h"
#include "../utils/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char tmp_path[64];

void setUp(void) {
    snprintf(tmp_path, sizeof(tmp_path), "/tmp/test_config_%d.txt", getpid());
}

void tearDown(void) {
    remove(tmp_path);
}

static void write_config(const char *content) {
    FILE *f = fopen(tmp_path, "w");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Could not create temp config file");
    fputs(content, f);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* Valid configurations                                                */
/* ------------------------------------------------------------------ */

void test_valid_port_8080(void) {
    write_config("SERVER_PORT=8080\n");
    TEST_ASSERT_EQUAL_INT(8080, read_port_from_config(tmp_path));
}

void test_valid_port_9090(void) {
    write_config("SERVER_PORT=9090\n");
    TEST_ASSERT_EQUAL_INT(9090, read_port_from_config(tmp_path));
}

void test_valid_port_1(void) {
    write_config("SERVER_PORT=1\n");
    TEST_ASSERT_EQUAL_INT(1, read_port_from_config(tmp_path));
}

void test_valid_port_65535(void) {
    write_config("SERVER_PORT=65535\n");
    TEST_ASSERT_EQUAL_INT(65535, read_port_from_config(tmp_path));
}

/* Key may appear on a line other than the first. */
void test_port_on_second_line(void) {
    write_config("# comment\nSERVER_PORT=7777\n");
    TEST_ASSERT_EQUAL_INT(7777, read_port_from_config(tmp_path));
}

/* ------------------------------------------------------------------ */
/* Missing or invalid configurations                                   */
/* ------------------------------------------------------------------ */

void test_missing_key_returns_minus_one(void) {
    write_config("SOME_OTHER_KEY=8080\n");
    TEST_ASSERT_EQUAL_INT(-1, read_port_from_config(tmp_path));
}

void test_empty_file_returns_minus_one(void) {
    write_config("");
    TEST_ASSERT_EQUAL_INT(-1, read_port_from_config(tmp_path));
}

void test_file_not_found_returns_minus_one(void) {
    TEST_ASSERT_EQUAL_INT(-1,
        read_port_from_config("/tmp/yathr_nonexistent_file_xyz.txt"));
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_valid_port_8080);
    RUN_TEST(test_valid_port_9090);
    RUN_TEST(test_valid_port_1);
    RUN_TEST(test_valid_port_65535);
    RUN_TEST(test_port_on_second_line);

    RUN_TEST(test_missing_key_returns_minus_one);
    RUN_TEST(test_empty_file_returns_minus_one);
    RUN_TEST(test_file_not_found_returns_minus_one);

    return UNITY_END();
}
