/**
 * @file backup_engine.c
 * @brief Implementación de las rutinas de copia y filtrado del Smart Backup.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <dirent.h>
 #include <stdbool.h>
 #include "smart_copy.h"
 
 // 1. FILTRO INTELIGENTE PARA STACK WEB
 const char *allowed_extensions[] = {
     ".ts", ".tsx", ".js", ".jsx", ".json", 
     ".prisma", ".md", ".env", ".sql", ".yml"
 };
 const int num_extensions = 10;
 
 bool is_file_allowed(const char *filename) {
     if (strcmp(filename, "Dockerfile") == 0 || strcmp(filename, "Makefile") == 0) return true;
     const char *ext = strrchr(filename, '.');
     if (!ext || ext == filename) return false;
     for (int i = 0; i < num_extensions; i++) {
         if (strcmp(ext, allowed_extensions[i]) == 0) return true;
     }
     return false;
 }
 
 // 2. SIMULACIÓN DE CAPA DE KERNEL (Syscalls)
 void sys_smart_copy(const char *src, const char *dest) {
     int fd_src, fd_dest;
     ssize_t bytes_read, bytes_written;
     char buffer[BUFFER_SIZE];
     struct stat st;
 
     if (stat(src, &st) == -1) return;
     fd_src = open(src, O_RDONLY);
     if (fd_src < 0) return;
     fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
     if (fd_dest < 0) { close(fd_src); return; }
 
     while ((bytes_read = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
         bytes_written = write(fd_dest, buffer, bytes_read);
         if (bytes_written != bytes_read) break; 
     }
     close(fd_src);
     close(fd_dest);
 }
 
 // 3. COPIA RECURSIVA CON FILTRO
 void copy_directory_smart(const char *src, const char *dest) {
     struct stat st;
     if (stat(src, &st) == -1) return;
 
     if (mkdir(dest, st.st_mode) == -1 && errno != EEXIST) {
         perror("[Error] No se pudo crear el directorio de destino");
         return;
     }
 
     DIR *dir = opendir(src);
     if (!dir) return;
 
     struct dirent *entry;
     char next_src[1024], next_dest[1024];
 
     while ((entry = readdir(dir)) != NULL) {
         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
         
         // Ignorar carpetas basura
         if (strcmp(entry->d_name, "node_modules") == 0 || strcmp(entry->d_name, ".next") == 0 || strcmp(entry->d_name, ".git") == 0) {
             printf("[-] Ignorando carpeta basura: %s\n", entry->d_name);
             continue;
         }
 
         snprintf(next_src, sizeof(next_src), "%s/%s", src, entry->d_name);
         snprintf(next_dest, sizeof(next_dest), "%s/%s", dest, entry->d_name);
 
         struct stat next_st;
         if (lstat(next_src, &next_st) == -1) continue;
 
         if (S_ISDIR(next_st.st_mode)) {
             copy_directory_smart(next_src, next_dest);
         } else if (S_ISREG(next_st.st_mode)) {
             if (is_file_allowed(entry->d_name)) {
                 printf("[+] Respaldando: %s\n", entry->d_name);
                 copy_file_syscall(next_src, next_dest);
             }
         }
     }
     closedir(dir);
 }
 
 // 4. CAPA DE USUARIO (Librería Estándar C)
 void copy_file_std(const char *src, const char *dest) {
     FILE *file_src, *file_dest;
     size_t bytes_read, bytes_written;
     char buffer[BUFFER_SIZE];
 
     file_src = fopen(src, "rb");
     if (!file_src) return;
     file_dest = fopen(dest, "wb");
     if (!file_dest) { fclose(file_src); return; }
 
     while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file_src)) > 0) {
         bytes_written = fwrite(buffer, 1, bytes_read, file_dest);
         if (bytes_written != bytes_read) break;
     }
     fclose(file_src);
     fclose(file_dest);
 }