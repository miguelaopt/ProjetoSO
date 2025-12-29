#include <stdio.h>
#include <unistd.h>
#include "../include/process_logic.h"

void pharmacy_process_main() {
    printf("[PHARMACY] Processo iniciado (PID %d)\n", getpid());
    while(1) sleep(10);
}