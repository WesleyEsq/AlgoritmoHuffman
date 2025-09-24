# Makefile para el Proyecto de Compresión Huffman
# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -O2
LDFLAGS = -lm

# --- Archivos Fuente y Objetos ---
COMMON_SOURCES = tree.c readFile.c
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)
FORK_OBJECTS = readFile_fork.o
PTHREAD_OBJECTS = readFile_pthread.o

# --- Objetivos (Ejecutables) ---
SERIAL_TARGET = huffman_serial
FORK_TARGET = huffman_fork
PTHREAD_TARGET = huffman_pthread
MENU_TARGET = huffman_cli # Nuevo ejecutable del menú

# --- Directorios para Pruebas ---
TEST_DIR = test_files
COMPRESSED_DIR = compressed_output
EXTRACTED_DIR = extracted_output

# --- Regla por defecto: compila los 3 ejecutables base ---
all: $(SERIAL_TARGET) $(FORK_TARGET) $(PTHREAD_TARGET)

# --- NUEVA REGLA: Compila absolutamente todo, incluido el menú ---
full: all $(MENU_TARGET)

# --- Reglas de Compilación ---
$(SERIAL_TARGET): main_serial.c $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(FORK_TARGET): main_fork.c $(FORK_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PTHREAD_TARGET): main_pthread.c $(PTHREAD_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

$(MENU_TARGET): main_menu.c $(COMMON_OBJECTS) $(FORK_OBJECTS) $(PTHREAD_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

# Regla genérica para archivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Comandos Utilitarios (Restaurados de tu original) ---

# Crear archivos de prueba
setup:
	@echo "Creando estructura de directorios de prueba..."
	@mkdir -p $(TEST_DIR) $(COMPRESSED_DIR) $(EXTRACTED_DIR)
	@echo "Este es un archivo de prueba simple." > $(TEST_DIR)/archivo1.txt
	@echo "Archivo de prueba con más contenido para el algoritmo de Huffman." > $(TEST_DIR)/archivo2.txt
	@echo "Tercer archivo con contenido diferente." > $(TEST_DIR)/archivo3.txt
	@echo "✓ Archivos de prueba creados en $(TEST_DIR)/"

# Crear archivo de prueba grande para benchmark
big-test:
	@echo "Creando archivo de prueba grande..."
	@for i in {1..1000}; do echo "Línea $$i: Contenido repetitivo para pruebas de rendimiento." >> $(TEST_DIR)/archivo_grande.txt; done
	@echo "✓ Archivo grande creado: $(TEST_DIR)/archivo_grande.txt"

# Ejecutar pruebas automáticas
test: all setup
	@echo "=== EJECUTANDO PRUEBAS AUTOMÁTICAS ==="
	@echo "\n--- Prueba 1: Versión Serial ---"
	@./$(SERIAL_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/test_serial.bin -x $(EXTRACTED_DIR)/serial
	@echo "\n--- Prueba 2: Versión Fork ---"
	@./$(FORK_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/test_fork.bin -x $(EXTRACTED_DIR)/fork
	@echo "\n--- Verificación de Integridad ---"
	@if diff -r $(TEST_DIR) $(EXTRACTED_DIR)/serial > /dev/null 2>&1; then \
		echo "✓ Versión serial: Integridad verificada"; \
	else \
		echo "✗ Versión serial: Error de integridad"; \
	fi
	@if diff -r $(TEST_DIR) $(EXTRACTED_DIR)/fork > /dev/null 2>&1; then \
		echo "✓ Versión fork: Integridad verificada"; \
	else \
		echo "✗ Versión fork: Error de integridad"; \
	fi

# Ejecutar pruebas de rendimiento
benchmark: all big-test
	@echo "=== PRUEBAS DE RENDIMIENTO ==="
	@echo "Probando con archivo grande..."
	@echo "\n--- Benchmark Serial ---"
	@time ./$(SERIAL_TARGET) -d $(TEST_DIR) -c -o $(COMPRESSED_DIR)/bench_serial.bin
	@echo "\n--- Benchmark Fork ---"
	@time ./$(FORK_TARGET) -d $(TEST_DIR) -c -o $(COMPRESSED_DIR)/bench_fork.bin
	@echo "\n--- Benchmark Pthread ---"
	@time ./$(PTHREAD_TARGET) -d $(TEST_DIR) -c -o $(COMPRESSED_DIR)/bench_pthread.bin

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos generados..."
	@rm -f *.o $(SERIAL_TARGET) $(FORK_TARGET) $(PTHREAD_TARGET) $(MENU_TARGET)
	@rm -rf $(TEST_DIR) $(COMPRESSED_DIR) $(EXTRACTED_DIR)
	@rm -f *.bin *.huff
	@echo "✓ Limpieza completada"

# Mostrar ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  make full         - Compilar todos los ejecutables (incluido el menú)"
	@echo "  make all          - Compilar solo las versiones de línea de comandos"
	@echo "  make huffman_cli  - Compilar solo el menú interactivo"
	@echo "  make setup        - Crear archivos de prueba"
	@echo "  make test         - Ejecutar pruebas de integridad"
	@echo "  make benchmark    - Ejecutar pruebas de rendimiento"
	@echo "  make clean        - Limpiar todo"
	@echo "  make help         - Mostrar esta ayuda"

# Reglas que no crean archivos
.PHONY: all full clean setup test benchmark help big-test