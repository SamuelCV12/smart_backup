/**
 * @file smart_copy.h
 * @brief Definiciones y firmas de funciones para la utilidad de respaldo inteligente.
 */

 #ifndef SMART_COPY_H
 #define SMART_COPY_H
 
 #include <stdbool.h>
 
 /** @brief Tamaño de la página de memoria estándar en Linux (4 KB). */
 #define BUFFER_SIZE 4096
 
 void sys_smart_copy(const char *src, const char *dest);
 void copy_file_std(const char *src, const char *dest);
 
 /**
  * @brief Verifica si un archivo debe ser respaldado basándose en un stack web.
  * * @param filename Nombre del archivo a evaluar.
  * @return true si tiene una extensión permitida (ej. .ts, .json) o es un Dockerfile.
  * @return false si es un archivo no deseado.
  */
 bool is_file_allowed(const char *filename);
 
 /**
  * @brief Copia un directorio de forma recursiva aplicando el filtro inteligente.
  * * Ignora automáticamente carpetas pesadas como 'node_modules' o '.next'.
  * * @param src Ruta del directorio de origen.
  * @param dest Ruta del directorio de destino.
  */
 void copy_directory_smart(const char *src, const char *dest);
 
 #endif // SMART_COPY_H