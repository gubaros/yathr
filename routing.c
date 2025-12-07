/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "routing.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    char *key;
    char *url;
} Redirect;

// Default entries for initialization (sorted alphabetically)
static const struct {
    const char *key;
    const char *url;
} default_redirects[] = {
    {"amazon", "https://www.amazon.com"},
    {"apple", "https://www.apple.com"},
    {"bbc", "https://www.bbc.com"},
    {"bing", "https://www.bing.com"},
    {"cnn", "https://www.cnn.com"},
    {"ebay", "https://www.ebay.com"},
    {"facebook", "https://www.facebook.com"},
    {"google", "https://www.google.com"},
    {"guardian", "https://www.theguardian.com"},
    {"instagram", "https://www.instagram.com"},
    {"linkedin", "https://www.linkedin.com"},
    {"microsoft", "https://www.microsoft.com"},
    {"netflix", "https://www.netflix.com"},
    {"nytimes", "https://www.nytimes.com"},
    {"pinterest", "https://www.pinterest.com"},
    {"reddit", "https://www.reddit.com"},
    {"twitter", "https://www.twitter.com"},
    {"wikipedia", "https://www.wikipedia.org"},
    {"yahoo", "https://www.yahoo.com"},
    {"youtube", "https://www.youtube.com"},
};

#define DEFAULT_REDIRECTS_COUNT (sizeof(default_redirects) / sizeof(default_redirects[0]))
#define INITIAL_CAPACITY 32

static Redirect *redirects = NULL;
static size_t redirects_count = 0;
static size_t redirects_capacity = 0;
static int routing_initialized = 0;

// Find insertion position for maintaining sorted order (optimized binary search)
// Returns insertion position. If *exists is not NULL, sets it to 1 if key exists, 0 otherwise
static int find_insert_position(const char *key, int *exists) {
    int left = 0;
    int right = (int)redirects_count - 1;
    int cmp = 0;
    
    // Optimized binary search with reduced comparisons
    while (left < right) {
        int mid = left + ((right - left) >> 1); // Use bit shift instead of division
        cmp = strcmp(redirects[mid].key, key);
        
        if (cmp < 0) {
            left = mid + 1;
        } else if (cmp > 0) {
            right = mid;
        } else {
            // Found exact match
            if (exists) *exists = 1;
            return mid;
        }
    }
    
    // Final check for equality when left == right
    if (left < (int)redirects_count) {
        cmp = strcmp(redirects[left].key, key);
        if (cmp == 0) {
            if (exists) *exists = 1;
            return left;
        } else if (cmp > 0) {
            // Key is less than redirects[left], insert at left
            if (exists) *exists = 0;
            return left;
        }
        // else: cmp < 0, key is greater, insert after left
    }
    
    if (exists) *exists = 0;
    return left; // Insert at end (key is greater than all existing keys)
}

// Ensure capacity for at least one more entry
static int ensure_capacity(void) {
    if (redirects_count >= redirects_capacity) {
        size_t new_capacity = redirects_capacity == 0 ? INITIAL_CAPACITY : redirects_capacity * 2;
        Redirect *new_redirects = realloc(redirects, new_capacity * sizeof(Redirect));
        if (new_redirects == NULL) {
            return 0; // Allocation failed
        }
        redirects = new_redirects;
        redirects_capacity = new_capacity;
    }
    return 1; // Success
}

void init_routing(void) {
    if (routing_initialized) {
        return;
    }
    
    redirects_capacity = INITIAL_CAPACITY;
    redirects = malloc(redirects_capacity * sizeof(Redirect));
    if (redirects == NULL) {
        return;
    }
    
    // Add all default entries (already sorted)
    for (size_t i = 0; i < DEFAULT_REDIRECTS_COUNT; i++) {
        redirects[i].key = strdup(default_redirects[i].key);
        redirects[i].url = strdup(default_redirects[i].url);
        if (redirects[i].key == NULL || redirects[i].url == NULL) {
            // Free allocated memory on failure
            for (size_t j = 0; j < i; j++) {
                free(redirects[j].key);
                free(redirects[j].url);
            }
            free(redirects);
            redirects = NULL;
            redirects_capacity = 0;
            return;
        }
    }
    
    redirects_count = DEFAULT_REDIRECTS_COUNT;
    routing_initialized = 1;
}

int add_redirect(const char *key, const char *url) {
    if (key == NULL || url == NULL) {
        return 0; // Invalid parameters
    }
    
    if (!routing_initialized) {
        init_routing();
    }
    
    // Find insertion position and check if key exists (optimized: single search)
    int exists = 0;
    int pos = find_insert_position(key, &exists);
    if (exists) {
        // Key exists, update URL
        free(redirects[pos].url);
        redirects[pos].url = strdup(url);
        return redirects[pos].url != NULL ? 1 : 0;
    }
    
    // Ensure we have capacity
    if (!ensure_capacity()) {
        return 0; // Allocation failed
    }
    
    // Shift elements to make room for new entry
    for (int i = (int)redirects_count; i > pos; i--) {
        redirects[i] = redirects[i - 1];
    }
    
    // Insert new entry at the correct sorted position
    redirects[pos].key = strdup(key);
    redirects[pos].url = strdup(url);
    
    if (redirects[pos].key == NULL || redirects[pos].url == NULL) {
        // Free on failure and restore array
        if (redirects[pos].key != NULL) free(redirects[pos].key);
        if (redirects[pos].url != NULL) free(redirects[pos].url);
        for (int i = pos; i < (int)redirects_count; i++) {
            redirects[i] = redirects[i + 1];
        }
        return 0;
    }
    
    redirects_count++;
    return 1; // Success
}

const char *find_redirect(const char *key) {
    if (key == NULL) {
        return NULL;
    }
    
    // Lazy initialization if not already initialized
    if (!routing_initialized) {
        init_routing();
        if (!routing_initialized) {
            return NULL; // Initialization failed
        }
    }
    
    if (redirects_count == 0) {
        return NULL;
    }
    
    // Optimized binary search using bit shift (same as find_insert_position)
    int left = 0;
    int right = (int)redirects_count - 1;
    
    while (left <= right) {
        int mid = left + ((right - left) >> 1); // Use bit shift instead of division
        int cmp = strcmp(redirects[mid].key, key);
        
        if (cmp == 0) {
            return redirects[mid].url;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return NULL;
}

void cleanup_routing(void) {
    if (!routing_initialized) {
        return;
    }
    
    for (size_t i = 0; i < redirects_count; i++) {
        free(redirects[i].key);
        free(redirects[i].url);
    }
    
    free(redirects);
    redirects = NULL;
    redirects_count = 0;
    redirects_capacity = 0;
    routing_initialized = 0;
}
