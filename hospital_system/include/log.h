#ifndef LOG_H
#define LOG_H

typedef enum {
    LOG_CRITICAL = 1,
    LOG_ERROR = 2,
    LOG_WARNING = 3,
    LOG_INFO = 4,
    LOG_DEBUG = 5
} log_severity_t;

void init_log_manager();
void log_event(log_severity_t severity, const char *component, const char *event_type, const char *details);
void close_log_manager();

#endif