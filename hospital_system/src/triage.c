#include <stdio.h>
#include <unistd.h>
#include "../include/process_logic.h"

void triage_process_main() {
    printf("[TRIAGE] Processo iniciado (PID %d)\n", getpid());
    while(1) sleep(10);
}