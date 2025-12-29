#ifndef IPC_H
#define IPC_H

#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "config.h" // <--- ESTA LINHA CORRIGE O ERRO

// --- Message Queues Keys & Types ---
#define MQ_KEY_URGENT 'U'
#define MQ_KEY_NORMAL 'N'
#define MQ_KEY_RESP   'R'

// Tipos de Mensagem
#define MSG_NEW_EMERGENCY     1
#define MSG_NEW_APPOINTMENT   2
#define MSG_NEW_SURGERY       3
#define MSG_PHARMACY_REQUEST  4
#define MSG_LAB_REQUEST       5
#define MSG_PHARMACY_READY    6
#define MSG_LAB_RESULTS_READY 7
#define MSG_CRITICAL_STATUS   8
#define MSG_TRANSFER_PATIENT  9
#define MSG_REJECT_PATIENT    10

typedef struct {
    long msg_priority; // 1=urgent, 2=high, 3=normal
    int msg_type;
    char source[20];
    char target[20];
    char patient_id[15];
    int operation_id;
    time_t timestamp;
    char data[512];
} hospital_message_t;

// --- Shared Memory Structures ---

// SHM1: Global Statistics
typedef struct {
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    
    // Triage
    int total_emergency_patients;
    int total_appointments;
    double total_emergency_wait_time;
    double total_appointment_wait_time;
    int completed_emergencies;
    int completed_appointments;
    int critical_transfers;
    int rejected_patients;
    
    // Surgery
    int total_surgeries_bo1;
    int total_surgeries_bo2;
    int total_surgeries_bo3;
    double total_surgery_wait_time;
    int completed_surgeries;
    int cancelled_surgeries;
    double bo1_utilization_time;
    double bo2_utilization_time;
    double bo3_utilization_time;
    
    // Pharmacy
    int total_pharmacy_requests;
    int urgent_requests;
    int normal_requests;
    double total_pharmacy_response_time;
    int stock_depletions;
    int auto_restocks;
    
    // Labs
    int total_lab_tests_lab1;
    int total_lab_tests_lab2;
    int total_preop_tests;
    double total_lab_turnaround_time;
    int urgent_lab_tests;
    
    // Global
    int total_operations;
    int system_errors;
    time_t system_start_time;
    int simulation_time_units;
} global_statistics_t;

// SHM2: Surgery Block Status
typedef struct {
    int room_id;
    int status; // 0=FREE, 1=OCCUPIED, 2=CLEANING
    char current_patient[15];
    int surgery_start_time;
    int estimated_end_time;
    pthread_mutex_t mutex;
} surgery_room_t;

typedef struct {
    surgery_room_t rooms[3];
    int medical_teams_available;
    pthread_mutex_t teams_mutex;
} surgery_block_shm_t;

// SHM3: Pharmacy Stock
typedef struct {
    char name[30];
    int current_stock;
    int reserved;
    int threshold;
    int max_capacity;
    pthread_mutex_t mutex;
} medication_stock_t;

typedef struct {
    medication_stock_t medications[15];
    int total_active_requests;
    pthread_mutex_t global_mutex;
} pharmacy_shm_t;

// SHM4: Lab Queues
typedef struct {
    char request_id[20];
    char patient_id[15];
    int test_type;
    int priority;
    int status; // 0=pending, 1=processing, 2=done
    time_t request_time;
    time_t completion_time;
} lab_request_entry_t;

typedef struct {
    lab_request_entry_t queue_lab1[50];
    lab_request_entry_t queue_lab2[50];
    int lab1_count;
    int lab2_count;
    int lab1_available_slots;
    int lab2_available_slots;
    pthread_mutex_t lab1_mutex;
    pthread_mutex_t lab2_mutex;
} lab_queue_shm_t;

// SHM5: Critical Log
typedef struct {
    time_t timestamp;
    char event_type[30];
    char component[20];
    char description[256];
    int severity;
} critical_event_t;

typedef struct {
    critical_event_t events[1000]; // Circular buffer
    int event_count;
    int current_index;
    pthread_mutex_t mutex;
} critical_log_shm_t;

// Funções IPC globais
int create_all_resources(system_config_t *config);
void cleanup_all_resources();

#endif