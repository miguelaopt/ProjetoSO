#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include "../include/ipc.h"
#include "../include/config.h"
#include "../include/log.h"

// Variáveis globais para IDs (apenas para cleanup interno)
static int mq_urgent_id, mq_normal_id, mq_resp_id;
static int shm_stats_id, shm_bo_id, shm_pharm_id, shm_lab_id, shm_log_id;

// Ponteiros globais (serão usados pelos processos)
global_statistics_t *shm_stats = NULL;
surgery_block_shm_t *shm_surgery = NULL;
pharmacy_shm_t *shm_pharmacy = NULL;
lab_queue_shm_t *shm_labs = NULL;
critical_log_shm_t *shm_log = NULL;

// Helper para inicializar mutex partilhado entre processos
void init_shared_mutex(pthread_mutex_t *mutex) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

int create_all_resources(system_config_t *config) {
    // 1. Message Queues
    mq_urgent_id = msgget(ftok("/tmp", MQ_KEY_URGENT), IPC_CREAT | 0666);
    mq_normal_id = msgget(ftok("/tmp", MQ_KEY_NORMAL), IPC_CREAT | 0666);
    mq_resp_id   = msgget(ftok("/tmp", MQ_KEY_RESP),   IPC_CREAT | 0666);

    if (mq_urgent_id == -1 || mq_normal_id == -1 || mq_resp_id == -1) {
        log_event(LOG_ERROR, "SYSTEM", "IPC_ERROR", "Falha ao criar Message Queues");
        return -1;
    }

    // 2. Shared Memory
    // Stats
    shm_stats_id = shmget(IPC_PRIVATE, sizeof(global_statistics_t), IPC_CREAT | 0666);
    shm_stats = (global_statistics_t*) shmat(shm_stats_id, NULL, 0);
    init_shared_mutex(&shm_stats->mutex);
    // Inicializar valores base
    shm_stats->system_start_time = time(NULL);
    shm_stats->simulation_time_units = 0;

    // Surgery BO
    shm_bo_id = shmget(IPC_PRIVATE, sizeof(surgery_block_shm_t), IPC_CREAT | 0666);
    shm_surgery = (surgery_block_shm_t*) shmat(shm_bo_id, NULL, 0);
    init_shared_mutex(&shm_surgery->teams_mutex);
    shm_surgery->medical_teams_available = config->max_medical_teams;
    for(int i=0; i<3; i++) {
        init_shared_mutex(&shm_surgery->rooms[i].mutex);
        shm_surgery->rooms[i].status = 0; // FREE
        shm_surgery->rooms[i].room_id = i+1;
    }

    // Pharmacy
    shm_pharm_id = shmget(IPC_PRIVATE, sizeof(pharmacy_shm_t), IPC_CREAT | 0666);
    shm_pharmacy = (pharmacy_shm_t*) shmat(shm_pharm_id, NULL, 0);
    init_shared_mutex(&shm_pharmacy->global_mutex);
    // Inicializar stock a partir da config
    const char *med_names[] = {
        "ANALGESICO_A", "ANTIBIOTICO_B", "ANESTESICO_C", "SEDATIVO_D", "ANTIINFLAMATORIO_E",
        "CARDIOVASCULAR_F", "NEUROLOGICO_G", "ORTOPEDICO_H", "HEMOSTATIC_I", "ANTICOAGULANTE_J",
        "INSULINA_K", "ANALGESICO_FORTE_L", "ANTIBIOTICO_FORTE_M", "VITAMINA_N", "SUPLEMENTO_O"
    };
    for(int i=0; i<15; i++) {
        init_shared_mutex(&shm_pharmacy->medications[i].mutex);
        strcpy(shm_pharmacy->medications[i].name, med_names[i]);
        shm_pharmacy->medications[i].current_stock = config->med_stock[i];
        shm_pharmacy->medications[i].threshold = config->med_threshold[i];
    }

    // Labs
    shm_lab_id = shmget(IPC_PRIVATE, sizeof(lab_queue_shm_t), IPC_CREAT | 0666);
    shm_labs = (lab_queue_shm_t*) shmat(shm_lab_id, NULL, 0);
    init_shared_mutex(&shm_labs->lab1_mutex);
    init_shared_mutex(&shm_labs->lab2_mutex);
    shm_labs->lab1_available_slots = config->max_sim_tests_lab1;
    shm_labs->lab2_available_slots = config->max_sim_tests_lab2;

    // Log Crítico
    shm_log_id = shmget(IPC_PRIVATE, sizeof(critical_log_shm_t), IPC_CREAT | 0666);
    shm_log = (critical_log_shm_t*) shmat(shm_log_id, NULL, 0);
    init_shared_mutex(&shm_log->mutex);

    // 3. Named Pipes (FIFOs)
    mkfifo("input_pipe", 0666);
    mkfifo("triage_pipe", 0666);
    mkfifo("surgery_pipe", 0666);
    mkfifo("pharmacy_pipe", 0666);
    mkfifo("lab_pipe", 0666);

    // 4. Semáforos POSIX
    sem_unlink("/sem_surgery_bo1");
    sem_unlink("/sem_surgery_bo2");
    sem_unlink("/sem_surgery_bo3");
    sem_unlink("/sem_medical_teams");
    
    sem_open("/sem_surgery_bo1", O_CREAT, 0644, 1);
    sem_open("/sem_surgery_bo2", O_CREAT, 0644, 1);
    sem_open("/sem_surgery_bo3", O_CREAT, 0644, 1);
    sem_open("/sem_medical_teams", O_CREAT, 0644, config->max_medical_teams);

    log_event(LOG_INFO, "SYSTEM", "INIT", "Recursos IPC criados com sucesso");
    return 0;
}

void cleanup_all_resources() {
    // Remover filas
    msgctl(mq_urgent_id, IPC_RMID, NULL);
    msgctl(mq_normal_id, IPC_RMID, NULL);
    msgctl(mq_resp_id, IPC_RMID, NULL);

    // Desanexar e remover SHM
    shmdt(shm_stats); shmctl(shm_stats_id, IPC_RMID, NULL);
    shmdt(shm_surgery); shmctl(shm_bo_id, IPC_RMID, NULL);
    shmdt(shm_pharmacy); shmctl(shm_pharm_id, IPC_RMID, NULL);
    shmdt(shm_labs); shmctl(shm_lab_id, IPC_RMID, NULL);
    shmdt(shm_log); shmctl(shm_log_id, IPC_RMID, NULL);

    // Remover Pipes
    unlink("input_pipe");
    unlink("triage_pipe");
    unlink("surgery_pipe");
    unlink("pharmacy_pipe");
    unlink("lab_pipe");

    // Remover Semáforos
    sem_unlink("/sem_surgery_bo1");
    sem_unlink("/sem_surgery_bo2");
    sem_unlink("/sem_surgery_bo3");
    sem_unlink("/sem_medical_teams");
    
    log_event(LOG_INFO, "SYSTEM", "CLEANUP", "Recursos IPC removidos");
}