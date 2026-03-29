#ifndef SMART_COPY_H
#define SMART_COPY_H

// Definimos el tamaño de la página de memoria estándar en Linux (4 KB)
#define BUFFER_SIZE 4096

// Firmas de las funciones implementadas en backup_engine.c
void copy_file_syscall(const char *src, const char *dest);
void copy_file_std(const char *src, const char *dest);

#endif // SMART_COPY_H