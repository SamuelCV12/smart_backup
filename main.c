/**
 * @file main.c
 * @brief Punto de entrada de la utilidad de respaldo.
 */

 #define _POSIX_C_SOURCE 199309L
 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <sys/stat.h>
 #include "smart_copy.h"
 
 double measure_time(void (*copy_func)(const char*, const char*), const char *src, const char *dest) {
     struct timespec start, end;
     clock_gettime(CLOCK_MONOTONIC, &start);
     copy_func(src, dest);
     clock_gettime(CLOCK_MONOTONIC, &end);
     double time_taken = (end.tv_sec - start.tv_sec) * 1e9;
     return (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
 }
 
 int main(int argc, char *argv[]) {
     if (argc != 3) {
         fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
         return EXIT_FAILURE;
     }
 
     const char *src = argv[1];
     const char *dest = argv[2];
     struct stat st;
 
     if (stat(src, &st) == -1) {
         perror("Error accediendo al origen");
         return EXIT_FAILURE;
     }
 
     printf("\n==========================================\n");
     printf("   SMART BACKUP KERNEL-SPACE UTILITY      \n");
     printf("==========================================\n");
 
     // LÓGICA DE DECISIÓN
     if (S_ISDIR(st.st_mode)) {
         printf("Modo: Smart Directory Backup (Filtro Web Activado)\n");
         printf("Origen: %s | Destino: %s\n\n", src, dest);
         copy_directory_smart(src, dest);
         printf("\n[OK] Respaldo de directorio completado.\n\n");
     } 
     else if (S_ISREG(st.st_mode)) {
         printf("Modo: Benchmark de Rendimiento de Archivo\n");
         printf("Archivo origen: %s\n\n", src);
         
         char dest_syscall[512], dest_std[512];
         snprintf(dest_syscall, sizeof(dest_syscall), "%s_syscall.bak", dest);
         snprintf(dest_std, sizeof(dest_std), "%s_std.bak", dest);
 
         double time_syscall = measure_time(sys_smart_copy, src, dest_syscall);
         double time_std = measure_time(copy_file_std, src, dest_std);
 
         printf("\n==========================================\n");
         printf("          TABLA DE RENDIMIENTO            \n");
         printf("==========================================\n");
         printf("Método                  | Tiempo (Segundos)\n");
         printf("------------------------------------------\n");
         printf("System Calls (4KB buf)  | %f\n", time_syscall);
         printf("Librería Estándar (C)   | %f\n", time_std);
         printf("==========================================\n\n");
     }
 
     return EXIT_SUCCESS;
 }