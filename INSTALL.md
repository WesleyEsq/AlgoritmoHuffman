# Instalación - Proyecto de Compresión Huffman

## Instalación Rápida (Recomendada)

```bash
chmod +x install.sh
./install.sh
```

**El script automáticamente:**

- Instala dependencias necesarias
- Compila todas las versiones
- Crea archivos de prueba
- Ofrece demo automática o configuración manual

---

## Requisitos del Sistema

**Sistemas compatibles:**

- Debian 10+ / Ubuntu 18.04+ (recomendado para evaluación)
- Otros sistemas Linux (puede requerir ajustes menores)

**Dependencias automáticas:**

- GCC, Make, bibliotecas de desarrollo C

---

## Instalación Manual

### Si el script automático no funciona:

```bash
# 1. Instalar dependencias (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install build-essential gcc make libc6-dev

# 2. Compilar con Makefile
make all

# 3. O compilar manualmente
gcc -Wall -Wextra -std=gnu99 -O2 -o huffman_serial \
    main_serial.c readFile.c tree.c -lm

gcc -Wall -Wextra -std=gnu99 -O2 -o huffman_fork \
    main_fork.c readFile.c readFile_fork.c tree.c -lm
```

---

## Uso Básico

### Ejecutables generados:

- `huffman_serial` - Versión secuencial
- `huffman_fork` - Versión paralela con fork()

### Ejemplos:

```bash
# Comprimir y descomprimir un directorio
./huffman_serial -d ./mis_textos -o archivo.bin -x ./extraidos

# Solo comprimir
./huffman_fork -d ./mis_textos -o archivo.bin -c

# Solo descomprimir
./huffman_serial -o archivo.bin -x ./extraidos -u

# Comparar rendimiento serial vs fork
./huffman_fork -d ./mis_textos -o archivo.bin -b
```

### Opciones principales:

- `-d DIR` - Directorio a comprimir
- `-o FILE` - Archivo de salida (.bin)
- `-x DIR` - Directorio donde extraer
- `-c` - Solo comprimir
- `-u` - Solo descomprimir
- `-b` - Benchmark (comparar versiones)
- `-v` - Modo verbose
- `-h` - Ayuda completa

---

## Verificación

```bash
# Pruebas automáticas
make test

# Verificar integridad manualmente
diff -r directorio_original directorio_extraido
# (No debería mostrar diferencias)
```

---

## 🔧 Solución de Problemas

**Error de compilación:**

```bash
make clean
make all
```

**Permisos:**

```bash
chmod +x huffman_serial huffman_fork
```

**Dependencias faltantes:**

```bash
sudo apt-get install build-essential libc6-dev
```

---

## Comandos Útiles

```bash
make help      # Ver todas las opciones
make test      # Pruebas automáticas
make benchmark # Pruebas de rendimiento
make clean     # Limpiar archivos generados
```

---

**Para más detalles:** Ver `README.md` con documentación técnica completa.
