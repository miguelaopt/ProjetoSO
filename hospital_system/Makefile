# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread -D_REENTRANT -D_XOPEN_SOURCE=700 -g -O2 -Iinclude
LDFLAGS = -pthread -lrt

# Lista de Objetos
OBJS = src/main.o src/triage.o src/surgery.o src/pharmacy.o \
       src/laboratory.o src/patient_thread.o src/ipc_utils.o \
       src/sync_utils.o src/log_manager.o src/stats_manager.o \
       src/config_parser.o src/time_simulation.o

TARGET = hospital_system

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: clean $(TARGET)

clean:
	rm -f src/*.o $(TARGET)
	rm -f logs/*.txt
	rm -f results/*/*.txt results/*.txt
	rm -f /dev/shm/* 2>/dev/null || true

ipc_clean:
	@echo "Removendo recursos IPC..."
	@ipcs -q | awk '$$3~/^[0-9]/ {print $$2}' | xargs -r ipcrm -q
	@ipcs -m | awk '$$3~/^[0-9]/ {print $$2}' | xargs -r ipcrm -m
	@rm -f /dev/shm/sem.*
	@rm -f input_pipe triage_pipe surgery_pipe pharmacy_pipe lab_pipe
	@echo "Recursos IPC removidos."

.PHONY: all clean debug ipc_clean
