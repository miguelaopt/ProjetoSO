#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CONFIG_KEY 50
#define MAX_CONFIG_VALUE 100

typedef struct {
    char key[MAX_CONFIG_KEY];
    char value[MAX_CONFIG_VALUE];
} config_param_t;

// Estrutura completa de configuração para facilitar acesso global
typedef struct {
    int time_unit_ms;
    int max_emergency_patients;
    int max_appointments;
    int max_surgeries_pending;
    
    int triage_simultaneous_patients;
    int triage_critical_stability;
    int triage_emergency_duration;
    int triage_appointment_duration;
    
    int b01_min_duration;
    int b01_max_duration;
    int b02_min_duration;
    int b02_max_duration;
    int b03_min_duration;
    int b03_max_duration;
    int cleanup_min_time;
    int cleanup_max_time;
    int max_medical_teams;
    
    int pharmacy_prep_min;
    int pharmacy_prep_max;
    int auto_restock_enabled;
    int restock_multiplier;
    
    int lab1_min;
    int lab1_max;
    int lab2_min;
    int lab2_max;
    int max_sim_tests_lab1;
    int max_sim_tests_lab2;
    
    // Medicamentos (Stock Inicial e Threshold)
    int med_stock[15];
    int med_threshold[15];
} system_config_t;

int load_config(const char *filename, system_config_t *config);

#endif