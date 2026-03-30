/**
 * @file main.c
 * @brief Punto de entrada de la utilidad de respaldo con unidades dinámicas y protección de 64 bits.
 */

 #define _POSIX_C_SOURCE 199309L
 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <errno.h>
 #include "smart_copy.h"
 
 /**
  * @brief Genera archivos de prueba evitando desbordamientos de 32 bits.
  */
 void generate_test_file_if_needed(const char *filename) {
     struct stat st;
     if (stat(filename, &st) == 0) return;
 
     unsigned long long size = 0;
 
     // Uso de ULL para asegurar cálculos en 64 bits
     if (strstr(filename, "1KB"))      size = 1024ULL;
     else if (strstr(filename, "1MB")) size = 1024ULL * 1024ULL;
     else if (strstr(filename, "1GB")) size = 1024ULL * 1024ULL * 1024ULL;
     else return; 
 
     printf("\n[*] Generando archivo de prueba: %s (%llu bytes)...\n", filename, size);
     
     FILE *f = fopen(filename, "wb");
     if (!f) {
         perror("[Error] No se pudo crear el archivo de prueba");
         exit(EXIT_FAILURE);
     }
 
     char buffer[BUFFER_SIZE];
     memset(buffer, 'A', BUFFER_SIZE); 
     unsigned long long written = 0;
     
     while (written < size) {
         size_t to_write = (size - written < BUFFER_SIZE) ? (size - written) : BUFFER_SIZE;
         size_t result = fwrite(buffer, 1, to_write, f);
         
         if (result < to_write) {
             fprintf(stderr, "\n[Fallo] Error de escritura (¿Disco lleno?): %s\n", strerror(errno));
             fclose(f);
             remove(filename);
             exit(EXIT_FAILURE);
         }
         written += result;
 
         if (size >= (1024ULL * 1024ULL * 1024ULL) && (written % (100 * 1024 * 1024) == 0)) {
             printf("."); fflush(stdout);
         }
     }
     
     fclose(f);
     printf("\n[+] Archivo '%s' listo.\n", filename);
 }
 
 double measure_time(void (*copy_func)(const char*, const char*), const char *src, const char *dest) {
     struct timespec start, end;
     clock_gettime(CLOCK_MONOTONIC, &start);
     copy_func(src, dest);
     clock_gettime(CLOCK_MONOTONIC, &end);
     return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
 }
 
 int main(int argc, char *argv[]) {
     if (argc != 3) {
         fprintf(stderr, "Uso: %s <origen> <destino_base>\n", argv[0]);
         return EXIT_FAILURE;
     }
 
     const char *src = argv[1];
     const char *dest_base = argv[2];
     
     generate_test_file_if_needed(src);
 
     struct stat st;
     if (stat(src, &st) == -1) {
         perror("[Error] No se pudo acceder al origen");
         return EXIT_FAILURE;
     }
 
     printf("\n==================================================\n");
     printf("     SMART BACKUP - BENCHMARK & UTILITY           \n");
     printf("==================================================\n");
 
     if (S_ISDIR(st.st_mode)) {
         printf("Modo: Respaldo de Directorio (Filtro Web)\n");
         copy_directory_smart(src, dest_base);
     } 
     else {
         printf("Modo: Comparativa de Rendimiento (Archivo Único)\n");
         
         // Unidades dinámicas para evitar el "0.00 MB"
         if (st.st_size < 1024) {
             printf("Tamaño del archivo: %ld Bytes\n", st.st_size);
         } else if (st.st_size < 1024 * 1024) {
             printf("Tamaño del archivo: %.2f KB\n", (double)st.st_size / 1024);
         } else {
             printf("Tamaño del archivo: %.2f MB\n", (double)st.st_size / (1024 * 1024));
         }
 
         char d_sys[512], d_std[512];
         snprintf(d_sys, sizeof(d_sys), "%s_syscall.bak", dest_base);
         snprintf(d_std, sizeof(d_std), "%s_std.bak", dest_base);
 
         double t_sys = measure_time(sys_smart_copy, src, d_sys);
         double t_std = measure_time(copy_file_std, src, d_std);
 
         double mb_size = (double)st.st_size / (1024 * 1024);
         double speed_sys = mb_size / t_sys;
         double speed_std = mb_size / t_std;
 
         printf("\n==================================================\n");
         printf("%-20s | %-12s | %-10s\n", "Método", "Tiempo (s)", "MB/s");
         printf("--------------------------------------------------\n");
         printf("%-20s | %-12.6f | %-10.2f\n", "System Calls (4KB)", t_sys, speed_sys);
         printf("%-20s | %-12.6f | %-10.2f\n", "Stdio.h (Buffered)", t_std, speed_std);
         printf("==================================================\n\n");
     }
 
     return EXIT_SUCCESS;
 }
