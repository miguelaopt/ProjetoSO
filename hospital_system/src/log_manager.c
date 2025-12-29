#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include "../include/log.h"

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_log_manager() {
    log_file = fopen("logs/hospital_log.txt", "w"); // "w" para limpar o log anterior
    if (!log_file) {
        perror("Falha ao criar ficheiro de log");
    }
}

void log_event(log_severity_t severity, const char *component, const char *event_type, const char *details) {
    if (!log_file) return;

    pthread_mutex_lock(&log_mutex);

    // Timestamp
    time_t now = time(NULL);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Severity String
    const char *sev_str;
    switch(severity) {
        case LOG_CRITICAL: sev_str = "CRITICAL"; break;
        case LOG_ERROR:    sev_str = "ERROR"; break;
        case LOG_WARNING:  sev_str = "WARNING"; break;
        case LOG_INFO:     sev_str = "INFO"; break;
        case LOG_DEBUG:    sev_str = "DEBUG"; break;
        default:           sev_str = "UNKNOWN"; break;
    }

    // Write to file: [TIMESTAMP] [COMPONENT] [SEVERITY] [EVENT_TYPE] DETAILS
    fprintf(log_file, "[%s] [%s] [%s] [%s] %s\n", 
            time_str, component, sev_str, event_type, details);
    
    // Flush para garantir escrita imediata em caso de crash
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}

void close_log_manager() {
    pthread_mutex_lock(&log_mutex);
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
}