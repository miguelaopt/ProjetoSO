#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "../include/config.h"
#include "../include/ipc.h"
#include "../include/log.h"
#include "../include/process_logic.h"

// Variáveis globais para controle
volatile sig_atomic_t g_shutdown = 0;
pid_t pid_triage, pid_surgery, pid_pharmacy, pid_lab;
system_config_t config;

void sigint_handler(int signum) {
    (void)signum; 
    g_shutdown = 1;
    printf("\n\n[MAIN] Shutdown iniciado... Aguarde.\n");
}

int main() {
    // 1. Setup inicial
    init_log_manager();
    log_event(LOG_INFO, "SYSTEM", "STARTUP", "Sistema Hospitalar Iniciado");

    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "Erro ao carregar configuracao\n");
        return 1;
    }

    // 2. Criar recursos IPC
    if (create_all_resources(&config) != 0) {
        return 1;
    }

    // 3. Capturar SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // 4. Criar Processos Filhos (Fork)
    if ((pid_triage = fork()) == 0) {
        triage_process_main();
        exit(0);
    }
    
    if ((pid_surgery = fork()) == 0) {
        surgery_process_main();
        exit(0);
    }

    if ((pid_pharmacy = fork()) == 0) {
        pharmacy_process_main();
        exit(0);
    }

    if ((pid_lab = fork()) == 0) {
        laboratory_process_main();
        exit(0);
    }

    log_event(LOG_INFO, "SYSTEM", "PROCESSES", "Todos os processos filhos criados");
    printf("[MAIN] Sistema em execucao (PIDs: %d, %d, %d, %d). Ctrl+C para sair.\n",
           pid_triage, pid_surgery, pid_pharmacy, pid_lab);

    // 5. Loop Principal
    while (!g_shutdown) {
        sleep(1);
    }

    // 6. Shutdown Gracioso
    log_event(LOG_INFO, "SYSTEM", "SHUTDOWN", "Enviando SIGTERM para filhos");
    
    kill(pid_triage, SIGTERM);
    kill(pid_surgery, SIGTERM);
    kill(pid_pharmacy, SIGTERM);
    kill(pid_lab, SIGTERM);

    waitpid(pid_triage, NULL, 0);
    waitpid(pid_surgery, NULL, 0);
    waitpid(pid_pharmacy, NULL, 0);
    waitpid(pid_lab, NULL, 0);

    cleanup_all_resources();
    close_log_manager();
    
    printf("[MAIN] Shutdown completo. Adeus.\n");
    return 0;
}