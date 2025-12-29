#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"
#include "../include/log.h"

// Remove espaços e quebras de linha
void trim(char * str) {
    char * p = str;
    int l = strlen(p);
    while (l > 0 && (p[l - 1] == '\n' || p[l - 1] == '\r' || p[l - 1] == ' ')) p[--l] = 0;
}

int load_config(const char *filename, system_config_t *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir config.txt");
        return -1;
    }

    char line[256];
    char key[MAX_CONFIG_KEY];
    char value[MAX_CONFIG_VALUE];
    int med_count = 0;

    // Valores padrão caso falhe
    config->time_unit_ms = 500;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue; // Ignora comentários

        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            trim(key);
            trim(value);

            if (strcmp(key, "TIME_UNIT_MS") == 0) config->time_unit_ms = atoi(value);
            else if (strcmp(key, "MAX_EMERGENCY_PATIENTS") == 0) config->max_emergency_patients = atoi(value);
            else if (strcmp(key, "MAX_APPOINTMENTS") == 0) config->max_appointments = atoi(value);
            else if (strcmp(key, "MAX_SURGERIES_PENDING") == 0) config->max_surgeries_pending = atoi(value);
            
            // Triagem
            else if (strcmp(key, "TRIAGE_SIMULTANEOUS_PATIENTS") == 0) config->triage_simultaneous_patients = atoi(value);
            else if (strcmp(key, "TRIAGE_CRITICAL_STABILITY") == 0) config->triage_critical_stability = atoi(value);
            else if (strcmp(key, "TRIAGE_EMERGENCY_DURATION") == 0) config->triage_emergency_duration = atoi(value);
            else if (strcmp(key, "TRIAGE_APPOINTMENT_DURATION") == 0) config->triage_appointment_duration = atoi(value);
            
            // Blocos
            else if (strcmp(key, "B01_MIN_DURATION") == 0) config->b01_min_duration = atoi(value);
            else if (strcmp(key, "B01_MAX_DURATION") == 0) config->b01_max_duration = atoi(value);
            else if (strcmp(key, "B02_MIN_DURATION") == 0) config->b02_min_duration = atoi(value);
            else if (strcmp(key, "B02_MAX_DURATION") == 0) config->b02_max_duration = atoi(value);
            else if (strcmp(key, "B03_MIN_DURATION") == 0) config->b03_min_duration = atoi(value);
            else if (strcmp(key, "B03_MAX_DURATION") == 0) config->b03_max_duration = atoi(value);
            else if (strcmp(key, "CLEANUP_MIN_TIME") == 0) config->cleanup_min_time = atoi(value);
            else if (strcmp(key, "CLEANUP_MAX_TIME") == 0) config->cleanup_max_time = atoi(value);
            else if (strcmp(key, "MAX_MEDICAL_TEAMS") == 0) config->max_medical_teams = atoi(value);
            
            // Farmácia e Labs
            else if (strcmp(key, "PHARMACY_PREPARATION_TIME_MIN") == 0) config->pharmacy_prep_min = atoi(value);
            else if (strcmp(key, "PHARMACY_PREPARATION_TIME_MAX") == 0) config->pharmacy_prep_max = atoi(value);
            else if (strcmp(key, "LAB1_TEST_MIN_DURATION") == 0) config->lab1_min = atoi(value);
            else if (strcmp(key, "LAB1_TEST_MAX_DURATION") == 0) config->lab1_max = atoi(value);
            else if (strcmp(key, "LAB2_TEST_MIN_DURATION") == 0) config->lab2_min = atoi(value);
            else if (strcmp(key, "LAB2_TEST_MAX_DURATION") == 0) config->lab2_max = atoi(value);
            
            // Medicamentos (Formato STOCK:THRESHOLD)
            else if (med_count < 15) {
                // Assume que qualquer outra chave é um medicamento
                int stock, threshold;
                if (sscanf(value, "%d:%d", &stock, &threshold) == 2) {
                    config->med_stock[med_count] = stock;
                    config->med_threshold[med_count] = threshold;
                    // Guardamos apenas o índice, o nome será mapeado no processo da farmácia
                    med_count++;
                }
            }
        }
    }

    fclose(file);
    return 0;
}