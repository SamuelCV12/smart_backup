# Smart Backup Kernel-Space Utility 🚀
> **- Sistemas Operativos** 

![C](https://img.shields.io/badge/Language-C-blue.svg)
![Linux](https://img.shields.io/badge/Platform-Linux-orange.svg)

## 👨‍💻 Autor
* **Samuel Correa Velásquez**

## 📝 Descripción
Esta herramienta simula una utilidad de respaldo a bajo nivel, comparando el rendimiento entre **System Calls** directas y la **Librería Estándar de C (stdio.h)**. Incluye un filtro inteligente para optimizar el respaldo de proyectos web modernos.

---

## 🧠 Lógica del "Smart Backup" (Gestión Inteligente de E/S)
A diferencia de un comando de copia tradicional, esta utilidad implementa un **Filtro de Selección Inteligente**. El objetivo es optimizar el uso del disco y minimizar la sobrecarga de procesos en proyectos de desarrollo modernos (Stack Web).

### 📋 Criterios de Respaldo (Whitelist)
El sistema prioriza el **activo intelectual** y los archivos de configuración necesarios para reconstruir el entorno:

| Tipo de Archivo | Extensiones Permitidas | Justificación Técnica |
| :--- | :--- | :--- |
| **Código Fuente** | `.ts`, `.tsx`, `.js`, `.jsx` | Lógica principal del software. |
| **Configuración** | `.json`, `.yml`, `.env`, `.prisma` | Definiciones de entorno, dependencias y base de datos. |
| **Documentación** | `.md` | Manuales y guías de mantenimiento. |
| **Infraestructura**| `Dockerfile`, `Makefile` | Archivos para despliegue y automatización. |
| **Persistencia** | `.sql` | Esquemas y semillas de datos. |

### 🚫 Criterios de Exclusión (Blacklist)
Para evitar el **Double Buffering** innecesario y el agotamiento de descriptores de archivo en el Kernel, el algoritmo ignora:
* **`node_modules/` y `.next/`:** Directorios de dependencias y compilados que se pueden regenerar (ahorra gigabytes de I/O innecesario).
* **Archivos Multimedia:** Extensiones como `.mp4` o `.png` son omitidas para centrar el backup en el código fuente.
* **Metadatos de Git:** Se ignora la carpeta `.git/` para mantener el respaldo ligero y enfocado en el estado actual del código.

---

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
