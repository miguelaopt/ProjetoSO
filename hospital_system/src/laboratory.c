#include <stdio.h>
#include <unistd.h>
#include "../include/process_logic.h"

void laboratory_process_main() {
    printf("[LAB] Processo iniciado (PID %d)\n", getpid());
    while(1) sleep(10);
}