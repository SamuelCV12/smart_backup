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
 #include <syslog.h>       /* agregado para logging al sistema */
 #include <limits.h>       /* para PATH_MAX */
 #ifndef PATH_MAX
 #define PATH_MAX 4096
 #endif
 #include "smart_copy.h"
 
 /* =========================================================
  * 1. FILTRO INTELIGENTE PARA STACK WEB
  * ========================================================= */
 
 const char *allowed_extensions[] = {
     ".ts", ".tsx", ".js", ".jsx", ".json",
     ".prisma", ".md", ".env", ".sql", ".yml"
 };
 
 /* Tamaño calculado automáticamente, no hardcodeado */
 const int num_extensions = sizeof(allowed_extensions) / sizeof(allowed_extensions[0]);
 
 bool is_file_allowed(const char *filename) {
     if (strcmp(filename, "Dockerfile") == 0 || strcmp(filename, "Makefile") == 0)
         return true;
     const char *ext = strrchr(filename, '.');
     if (!ext || ext == filename) return false;
     for (int i = 0; i < num_extensions; i++) {
         if (strcmp(ext, allowed_extensions[i]) == 0) return true;
     }
     return false;
 }
 
 /* =========================================================
  * 2. SIMULACIÓN DE CAPA DE KERNEL (Syscalls directas)
  * ========================================================= */
 
 /**
  * @brief Copia un archivo usando syscalls directas (open/read/write).
  *
  * Simula el comportamiento de una función de kernel: opera sin buffering
  * en espacio de usuario, usando un buffer estricto de 4 KB alineado a
  * página de memoria. Registra la actividad y errores en el syslog.
  *
  * @param src  Ruta del archivo origen.
  * @param dest Ruta del archivo destino.
  */
 void sys_smart_copy(const char *src, const char *dest) {
     int fd_src, fd_dest;
     ssize_t bytes_read, bytes_written;
     char buffer[BUFFER_SIZE];
     struct stat st;
 
     /*logging al sistema al inicio de la operación */
     openlog("smart_backup", LOG_PID | LOG_CONS, LOG_USER);
 
     /* Validar que el origen existe y obtener sus metadatos */
     if (stat(src, &st) == -1) {
         syslog(LOG_ERR, "sys_smart_copy: stat() falló en '%s': %s", src, strerror(errno));
         perror("[Error] No se pudo acceder al archivo origen");
         closelog();
         return;
     }
 
     /* verificar permiso de lectura explícitamente */
     if (access(src, R_OK) == -1) {
         syslog(LOG_ERR, "sys_smart_copy: sin permiso de lectura en '%s': %s", src, strerror(errno));
         fprintf(stderr, "[Error] Sin permiso de lectura en '%s': %s\n", src, strerror(errno));
         closelog();
         return;
     }
 
     fd_src = open(src, O_RDONLY);
     if (fd_src < 0) {
         syslog(LOG_ERR, "sys_smart_copy: open() falló en origen '%s': %s", src, strerror(errno));
         fprintf(stderr, "[Error] No se pudo abrir el origen '%s': %s\n", src, strerror(errno));
         closelog();
         return;
     }
 
     fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
     if (fd_dest < 0) {
         syslog(LOG_ERR, "sys_smart_copy: open() falló en destino '%s': %s", dest, strerror(errno));
         fprintf(stderr, "[Error] No se pudo abrir el destino '%s': %s\n", dest, strerror(errno));
         close(fd_src);
         closelog();
         return;
     }
 
     /* Ciclo de copia con buffer de 4 KB (tamaño de página de memoria) */
     while ((bytes_read = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
         bytes_written = write(fd_dest, buffer, bytes_read);
 
         /* detectar errores de escritura (disco lleno, etc.) */
         if (bytes_written < 0) {
             syslog(LOG_ERR, "sys_smart_copy: write() falló en '%s': %s", dest, strerror(errno));
             fprintf(stderr, "[Error] Escritura fallida en '%s': %s\n", dest, strerror(errno));
 
             /* Eliminar el archivo destino corrupto */
             close(fd_src);
             close(fd_dest);
             unlink(dest);
             closelog();
             return;
         }
 
         if (bytes_written != bytes_read) {
             /* disco lleno (ENOSPC) — escritura parcial */
             syslog(LOG_ERR, "sys_smart_copy: escritura parcial en '%s' (errno: %s)", dest, strerror(errno));
             fprintf(stderr, "[Error] Disco lleno o escritura parcial en '%s'\n", dest);
             close(fd_src);
             close(fd_dest);
             unlink(dest);   /* Eliminar archivo destino incompleto */
             closelog();
             return;
         }
     }
 
     /* verificar error en read() */
     if (bytes_read < 0) {
         syslog(LOG_ERR, "sys_smart_copy: read() falló en '%s': %s", src, strerror(errno));
         fprintf(stderr, "[Error] Lectura fallida en '%s': %s\n", src, strerror(errno));
         close(fd_src);
         close(fd_dest);
         unlink(dest);
         closelog();
         return;
     }
 
     syslog(LOG_INFO, "sys_smart_copy: copia exitosa '%s' -> '%s' (%ld bytes)", src, dest, (long)st.st_size);
 
     close(fd_src);
     close(fd_dest);
     closelog();
 }
 
 /* =========================================================
  * 3. COPIA RECURSIVA CON FILTRO
  * ========================================================= */
 
 void copy_directory_smart(const char *src, const char *dest) {
     struct stat st;
     if (stat(src, &st) == -1) {
         perror("[Error] No se pudo acceder al directorio origen");
         return;
     }
 
     if (mkdir(dest, st.st_mode) == -1 && errno != EEXIST) {
         perror("[Error] No se pudo crear el directorio de destino");
         return;
     }
 
     DIR *dir = opendir(src);
     if (!dir) {
         perror("[Error] No se pudo abrir el directorio");
         return;
     }
 
     struct dirent *entry;
 
     /* rutas con PATH_MAX */
     char next_src[PATH_MAX];
     char next_dest[PATH_MAX];
 
     while ((entry = readdir(dir)) != NULL) {
         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
             continue;
 
         /* Ignorar carpetas autogeneradas pesadas */
         if (strcmp(entry->d_name, "node_modules") == 0 ||
             strcmp(entry->d_name, ".next") == 0 ||
             strcmp(entry->d_name, ".git") == 0) {
             printf("[-] Ignorando carpeta: %s\n", entry->d_name);
             continue;
         }
 
         /* verificar que snprintf no desbordó el buffer */
         if (snprintf(next_src,  sizeof(next_src),  "%s/%s", src,  entry->d_name) >= (int)sizeof(next_src) ||
             snprintf(next_dest, sizeof(next_dest), "%s/%s", dest, entry->d_name) >= (int)sizeof(next_dest)) {
             fprintf(stderr, "[Error] Ruta demasiado larga, saltando: %s\n", entry->d_name);
             continue;
         }
 
         struct stat next_st;
         if (lstat(next_src, &next_st) == -1) continue;
 
         if (S_ISDIR(next_st.st_mode)) {
             copy_directory_smart(next_src, next_dest);
         } else if (S_ISREG(next_st.st_mode)) {
             if (is_file_allowed(entry->d_name)) {
                 printf("[+] Respaldando: %s\n", entry->d_name);
                 sys_smart_copy(next_src, next_dest);
             }
         }
     }
     closedir(dir);
 }
 
 /* =========================================================
  * 4. CAPA DE USUARIO (Librería Estándar C — stdio.h)
  * ========================================================= */
 
 /**
  * @brief Copia un archivo usando la librería estándar de C (fread/fwrite).
  *
  * A diferencia de sys_smart_copy, esta función opera en espacio de usuario
  * aprovechando el buffering interno de glibc, lo que reduce la cantidad de
  * context switches al kernel y resulta más rápida en archivos pequeños.
  *
  * @param src  Ruta del archivo origen.
  * @param dest Ruta del archivo destino.
  */
 void copy_file_std(const char *src, const char *dest) {
     FILE *file_src  = fopen(src,  "rb");
     if (!file_src) {
         fprintf(stderr, "[Error] No se pudo abrir el origen '%s': %s\n", src, strerror(errno));
         return;
     }
 
     FILE *file_dest = fopen(dest, "wb");
     if (!file_dest) {
         fprintf(stderr, "[Error] No se pudo abrir el destino '%s': %s\n", dest, strerror(errno));
         fclose(file_src);
         return;
     }
 
     char buffer[BUFFER_SIZE];
     size_t bytes_read;
 
     while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file_src)) > 0) {
         size_t bytes_written = fwrite(buffer, 1, bytes_read, file_dest);
 
         /* detectar error de escritura */
         if (bytes_written != bytes_read) {
             fprintf(stderr, "[Error] Escritura fallida en '%s': %s\n", dest, strerror(errno));
             fclose(file_src);
             fclose(file_dest);
             remove(dest);   /* Eliminar archivo destino incompleto */
             return;
         }
     }
 
     /* verificar si fread terminó por error o por EOF */
     if (ferror(file_src)) {
         fprintf(stderr, "[Error] Lectura fallida en '%s': %s\n", src, strerror(errno));
         fclose(file_src);
         fclose(file_dest);
         remove(dest);
         return;
     }
 
     fclose(file_src);
     fclose(file_dest);
 }