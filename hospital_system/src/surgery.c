#include <stdio.h>
#include <unistd.h>
#include "../include/process_logic.h"

void surgery_process_main() {
    printf("[SURGERY] Processo iniciado (PID %d)\n", getpid());
    while(1) sleep(10);
}