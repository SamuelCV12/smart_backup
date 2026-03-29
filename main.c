#define _POSIX_C_SOURCE 199309L // Habilita las funciones POSIX de tiempo
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "smart_copy.h"

// Función auxiliar para medir el tiempo de ejecución de nuestras funciones de copia
double measure_time(void (*copy_func)(const char*, const char*), const char *src, const char *dest) {
    struct timespec start, end;
    
    // Iniciamos el cronómetro
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Ejecutamos la función de copia (ya sea la de Syscalls o la de stdio)
    copy_func(src, dest);

    // Detenemos el cronómetro
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculamos el tiempo transcurrido en segundos
    double time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    
    return time_taken;
}

int main(int argc, char *argv[]) {
    // Validación de argumentos de entrada
    if (argc != 3) {
        fprintf(stderr, "Uso incorrecto.\nFormato esperado: %s <archivo_origen> <prefijo_destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *src = argv[1];
    char dest_syscall[512];
    char dest_std[512];

    // Construimos los nombres de los archivos de salida
    snprintf(dest_syscall, sizeof(dest_syscall), "%s_syscall.bak", argv[2]);
    snprintf(dest_std, sizeof(dest_std), "%s_std.bak", argv[2]);

    printf("\n==========================================\n");
    printf("   SMART BACKUP KERNEL-SPACE UTILITY      \n");
    printf("==========================================\n");
    printf("Archivo origen: %s\n\n", src);

    // 1. Prueba de Rendimiento: Capa de Kernel (Syscalls + Buffer 4KB)
    printf("[*] Ejecutando copia con System Calls (Buffer %d bytes)...\n", BUFFER_SIZE);
    double time_syscall = measure_time(copy_file_syscall, src, dest_syscall);
    
    // 2. Prueba de Rendimiento: Capa de Usuario (Librería Estándar de C)
    printf("\n[*] Ejecutando copia con Librería Estándar de C (fread/fwrite)...\n");
    double time_std = measure_time(copy_file_std, src, dest_std);

    // 3. Impresión de la Tabla Comparativa
    printf("\n==========================================\n");
    printf("          TABLA DE RENDIMIENTO            \n");
    printf("==========================================\n");
    printf("Método                  | Tiempo (Segundos)\n");
    printf("------------------------------------------\n");
    printf("System Calls (4KB buf)  | %f\n", time_syscall);
    printf("Librería Estándar (C)   | %f\n", time_std);
    printf("==========================================\n\n");

    return EXIT_SUCCESS;
}