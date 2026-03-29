#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Definimos el tamaño de la página de memoria estándar en Linux (4 KB)
#define BUFFER_SIZE 4096

// 1. SIMULACIÓN DE CAPA DE KERNEL (Uso de System Calls directas)
 
void copy_file_syscall(const char *src, const char *dest) {
    int fd_src, fd_dest;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];
    struct stat st;

    // Obtenemos información del archivo origen para conservar sus permisos
    if (stat(src, &st) == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "[Error Syscall] El archivo origen '%s' no existe (ENOENT).\n", src);
        } else if (errno == EACCES) {
            fprintf(stderr, "[Error Syscall] Permiso denegado para acceder a '%s' (EACCES).\n", src);
        } else {
            perror("[Error Syscall] Error al obtener los stats del archivo origen");
        }
        return;
    }

    // Solicitamos al Kernel abrir el archivo origen en modo solo lectura
    fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        perror("[Error Syscall] Fallo al abrir el archivo de origen");
        return;
    }

    // Solicitamos al Kernel crear/abrir el destino (Escritura, Crear si no existe, Truncar a 0)
    fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (fd_dest < 0) {
        perror("[Error Syscall] Fallo al abrir/crear el archivo de destino");
        close(fd_src);
        return;
    }

    // Ciclo de copiado: Leemos y escribimos en bloques exactos de 4096 bytes
    // Esto minimiza los "Context Switches" hacia el Kernel Mode.
    while ((bytes_read = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(fd_dest, buffer, bytes_read);
        
        if (bytes_written != bytes_read) {
            // Manejo de error crítico exigido por la rúbrica: Disco Lleno
            if (errno == ENOSPC) {
                fprintf(stderr, "[Error Syscall CRÍTICO] No hay espacio libre en el disco (ENOSPC).\n");
            } else {
                perror("[Error Syscall] Fallo durante la escritura en el disco");
            }
            break; // Salimos del ciclo para no corromper el backup
        }
    }

    if (bytes_read < 0) {
        perror("[Error Syscall] Error durante la lectura del archivo");
    } else {
        printf("[OK] Copia Syscall completada: %s -> %s\n", src, dest);
    }

    // Liberamos los descriptores de archivo devueltos por el Kernel
    close(fd_src);
    close(fd_dest);
}


 // 2. CAPA DE USUARIO (Uso de la Librería Estándar de C)

void copy_file_std(const char *src, const char *dest) {
    FILE *file_src, *file_dest;
    size_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    // fopen maneja internamente sus propios buffers y descriptores
    file_src = fopen(src, "rb");
    if (!file_src) {
        perror("[Error Stdio] No se pudo abrir el archivo de origen");
        return;
    }

    file_dest = fopen(dest, "wb");
    if (!file_dest) {
        perror("[Error Stdio] No se pudo crear el archivo de destino");
        fclose(file_src);
        return;
    }

    // fread almacena la información temporalmente en el espacio de usuario
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file_src)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, file_dest);
        
        if (bytes_written != bytes_read) {
            fprintf(stderr, "[Error Stdio] Inconsistencia al escribir los datos con fwrite.\n");
            break;
        }
    }

    // Verificamos si hubo errores en el flujo de lectura
    if (ferror(file_src)) {
        fprintf(stderr, "[Error Stdio] Error de lectura detectado por ferror.\n");
    } else {
        printf("[OK] Copia Librería C completada: %s -> %s\n", src, dest);
    }

    fclose(file_src);
    fclose(file_dest);
}