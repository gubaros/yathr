/*
 * No-op log stubs for unit tests.
 * Replaces utils/logs.c so tests don't require zlog.
 */

#include "../utils/logs.h"
#include <stdarg.h>

void init_logs(void) {}
void log_info(const char *format, ...)    { (void)format; }
void log_error(const char *format, ...)   { (void)format; }
void log_warning(const char *format, ...) { (void)format; }
