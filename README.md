# Smart Backup Kernel-Space Utility 🚀
> **- Sistemas Operativos** 

![C](https://img.shields.io/badge/Language-C-blue.svg)
![Linux](https://img.shields.io/badge/Platform-Linux-orange.svg)

## 👨‍💻 Autor
* **Samuel Correa Velásquez**

## 📝 Descripción
Esta herramienta simula una utilidad de respaldo a bajo nivel, comparando el rendimiento entre **System Calls** directas y la **Librería Estándar de C (stdio.h)**. Incluye un filtro inteligente para optimizar el respaldo de proyectos web modernos.

## 🛠️ Características Técnicas
- **Buffer de 4KB:** Alineación exacta con el tamaño de página de memoria de Linux para maximizar el *throughput*.
- **Benchmark de Precisión:** Medición con `clock_gettime` para obtener resultados en nanosegundos.
- **Smart Filter:** Exclusión automática de carpetas pesadas como `node_modules` y `.next`.
- **Logging:** Registro de actividad mediante `syslog` para trazabilidad del sistema.

## 🚀 Cómo Ejecutar
1. **Compilar:** `make`
2. **Benchmark:** `./smart_backup <archivo_origen> <destino>`
3. **Smart Backup:** `./smart_backup <carpeta_origen> <carpeta_destino>`
4. **Documentación:** `make docs`

## 📊 Análisis de Rendimiento
En las pruebas realizadas, se observaron los siguientes comportamientos:
* **Archivos Pequeños:** La librería estándar gana gracias al *user-space buffering* que reduce los *context switches*.
* **Archivos Grandes (1GB):** Las syscalls directas lideran al reducir el *overhead* de gestión de memoria interna.

---
