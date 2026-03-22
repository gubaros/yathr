/*
 * Unit tests for routing.c
 *
 * Covers: default entries, unknown/null keys, add_redirect (new entry,
 * update, null args, boundary insertions, capacity growth), and
 * cleanup/reinitialize behaviour.
 */

#include "unity/unity.h"
#include "../routing.h"

#include <stdio.h>
#include <string.h>

/* Reset global routing state before and after every test. */
void setUp(void)    { cleanup_routing(); }
void tearDown(void) { cleanup_routing(); }

/* ------------------------------------------------------------------ */
/* find_redirect – default entries                                     */
/* ------------------------------------------------------------------ */

void test_find_redirect_google(void) {
    TEST_ASSERT_EQUAL_STRING("https://www.google.com", find_redirect("google"));
}

void test_find_redirect_all_defaults(void) {
    static const char *cases[][2] = {
        {"amazon",    "https://www.amazon.com"},
        {"apple",     "https://www.apple.com"},
        {"bbc",       "https://www.bbc.com"},
        {"bing",      "https://www.bing.com"},
        {"cnn",       "https://www.cnn.com"},
        {"ebay",      "https://www.ebay.com"},
        {"facebook",  "https://www.facebook.com"},
        {"google",    "https://www.google.com"},
        {"guardian",  "https://www.theguardian.com"},
        {"instagram", "https://www.instagram.com"},
        {"linkedin",  "https://www.linkedin.com"},
        {"microsoft", "https://www.microsoft.com"},
        {"netflix",   "https://www.netflix.com"},
        {"nytimes",   "https://www.nytimes.com"},
        {"pinterest", "https://www.pinterest.com"},
        {"reddit",    "https://www.reddit.com"},
        {"twitter",   "https://www.twitter.com"},
        {"wikipedia", "https://www.wikipedia.org"},
        {"yahoo",     "https://www.yahoo.com"},
        {"youtube",   "https://www.youtube.com"},
    };
    size_t n = sizeof(cases) / sizeof(cases[0]);
    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_EQUAL_STRING(cases[i][1], find_redirect(cases[i][0]));
    }
}

void test_find_redirect_unknown_key_returns_null(void) {
    TEST_ASSERT_NULL(find_redirect("nonexistent"));
}

void test_find_redirect_null_key_returns_null(void) {
    TEST_ASSERT_NULL(find_redirect(NULL));
}

/* Empty string is not NULL – binary search should miss every key and
   return NULL rather than crash. */
void test_find_redirect_empty_string_returns_null(void) {
    TEST_ASSERT_NULL(find_redirect(""));
}

/* ------------------------------------------------------------------ */
/* add_redirect – basic behaviour                                      */
/* ------------------------------------------------------------------ */

void test_add_redirect_new_key_is_findable(void) {
    TEST_ASSERT_EQUAL_INT(1, add_redirect("warp", "https://www.warp.dev"));
    TEST_ASSERT_EQUAL_STRING("https://www.warp.dev", find_redirect("warp"));
}

void test_add_redirect_updates_existing_key(void) {
    TEST_ASSERT_EQUAL_INT(1, add_redirect("google", "https://search.example.com"));
    TEST_ASSERT_EQUAL_STRING("https://search.example.com", find_redirect("google"));
    /* Other defaults unaffected. */
    TEST_ASSERT_EQUAL_STRING("https://www.youtube.com", find_redirect("youtube"));
}

void test_add_redirect_null_key_returns_zero(void) {
    TEST_ASSERT_EQUAL_INT(0, add_redirect(NULL, "https://example.com"));
}

void test_add_redirect_null_url_returns_zero(void) {
    TEST_ASSERT_EQUAL_INT(0, add_redirect("newkey", NULL));
    /* Key must not have been inserted. */
    TEST_ASSERT_NULL(find_redirect("newkey"));
}

/* ------------------------------------------------------------------ */
/* add_redirect – sorted-order boundary insertions                    */
/* ------------------------------------------------------------------ */

/* "aaa" sorts before "amazon" → insertion at head of array. */
void test_add_redirect_insert_before_first_default(void) {
    TEST_ASSERT_EQUAL_INT(1, add_redirect("aaa", "https://www.aaa.com"));
    TEST_ASSERT_EQUAL_STRING("https://www.aaa.com",    find_redirect("aaa"));
    TEST_ASSERT_EQUAL_STRING("https://www.amazon.com", find_redirect("amazon"));
}

/* "zzz" sorts after "youtube" → insertion at tail of array. */
void test_add_redirect_insert_after_last_default(void) {
    TEST_ASSERT_EQUAL_INT(1, add_redirect("zzz", "https://www.zzz.com"));
    TEST_ASSERT_EQUAL_STRING("https://www.zzz.com",    find_redirect("zzz"));
    TEST_ASSERT_EQUAL_STRING("https://www.youtube.com", find_redirect("youtube"));
}

/* "hacker" sorts between "guardian" and "instagram". */
void test_add_redirect_insert_in_middle(void) {
    TEST_ASSERT_EQUAL_INT(1, add_redirect("hacker", "https://news.ycombinator.com"));
    TEST_ASSERT_EQUAL_STRING("https://news.ycombinator.com",  find_redirect("hacker"));
    TEST_ASSERT_EQUAL_STRING("https://www.theguardian.com",   find_redirect("guardian"));
    TEST_ASSERT_EQUAL_STRING("https://www.instagram.com",     find_redirect("instagram"));
}

/* ------------------------------------------------------------------ */
/* add_redirect – capacity growth                                      */
/* ------------------------------------------------------------------ */

/* INITIAL_CAPACITY is 32; 20 defaults are pre-loaded.
   Adding 15 more (total 35) forces at least one realloc. */
void test_add_redirect_beyond_initial_capacity(void) {
    char key[32], url[64];
    for (int i = 0; i < 15; i++) {
        snprintf(key, sizeof(key), "site%02d", i);
        snprintf(url, sizeof(url), "https://www.site%02d.com", i);
        TEST_ASSERT_EQUAL_INT(1, add_redirect(key, url));
    }
    for (int i = 0; i < 15; i++) {
        snprintf(key, sizeof(key), "site%02d", i);
        snprintf(url, sizeof(url), "https://www.site%02d.com", i);
        TEST_ASSERT_EQUAL_STRING(url, find_redirect(key));
    }
    /* Default entries must survive the realloc. */
    TEST_ASSERT_EQUAL_STRING("https://www.google.com",  find_redirect("google"));
    TEST_ASSERT_EQUAL_STRING("https://www.youtube.com", find_redirect("youtube"));
}

/* ------------------------------------------------------------------ */
/* cleanup_routing                                                     */
/* ------------------------------------------------------------------ */

/* Custom entry added before cleanup must not survive it. */
void test_cleanup_removes_added_entries(void) {
    add_redirect("custom", "https://custom.example.com");
    TEST_ASSERT_NOT_NULL(find_redirect("custom"));
    cleanup_routing();
    /* Lazy reinit restores only the defaults – "custom" is gone. */
    TEST_ASSERT_NULL(find_redirect("custom"));
}

/* Explicit reinit after cleanup must restore all defaults. */
void test_cleanup_then_reinitialize_restores_defaults(void) {
    cleanup_routing();
    init_routing();
    TEST_ASSERT_EQUAL_STRING("https://www.google.com",  find_redirect("google"));
    TEST_ASSERT_EQUAL_STRING("https://www.amazon.com",  find_redirect("amazon"));
    TEST_ASSERT_EQUAL_STRING("https://www.youtube.com", find_redirect("youtube"));
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_find_redirect_google);
    RUN_TEST(test_find_redirect_all_defaults);
    RUN_TEST(test_find_redirect_unknown_key_returns_null);
    RUN_TEST(test_find_redirect_null_key_returns_null);
    RUN_TEST(test_find_redirect_empty_string_returns_null);

    RUN_TEST(test_add_redirect_new_key_is_findable);
    RUN_TEST(test_add_redirect_updates_existing_key);
    RUN_TEST(test_add_redirect_null_key_returns_zero);
    RUN_TEST(test_add_redirect_null_url_returns_zero);

    RUN_TEST(test_add_redirect_insert_before_first_default);
    RUN_TEST(test_add_redirect_insert_after_last_default);
    RUN_TEST(test_add_redirect_insert_in_middle);
    RUN_TEST(test_add_redirect_beyond_initial_capacity);

    RUN_TEST(test_cleanup_removes_added_entries);
    RUN_TEST(test_cleanup_then_reinitialize_restores_defaults);

    return UNITY_END();
}
