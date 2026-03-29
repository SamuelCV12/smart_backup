CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = smart_backup
OBJS = main.o backup_engine.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo "========================================"
	@echo " Compilación exitosa: ./$(TARGET)"
	@echo "========================================"

main.o: main.c smart_copy.h
	$(CC) $(CFLAGS) -c main.c

backup_engine.o: backup_engine.c smart_copy.h
	$(CC) $(CFLAGS) -c backup_engine.c

clean:
	rm -f $(OBJS) $(TARGET) *.bak
	@echo "Archivos compilados y backups de prueba eliminados."