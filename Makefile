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
	@echo "Iniciando limpieza profunda del entorno..."
	# 1. Borrar binarios y objetos
	rm -f $(OBJS) $(TARGET)
	# 2. Borrar archivos de backup generados (.bak)
	rm -f *.bak
	# 3. Borrar archivos de prueba autogenerados (1KB, 1MB, 1GB)
	rm -f *1KB* *1MB* *1GB*
	# 4. Borrar carpetas de destino de backups 
	# Ajusta 'backup_dest' al nombre de carpeta que suelas usar
	rm -rf backup_dest/ 
	@echo "-------------------------------------------"
	@echo "¡Entorno impecable para la próxima ejecución!"

docs:
	doxygen Doxyfile
	@echo "Documentación generada en la carpeta doc/html/"