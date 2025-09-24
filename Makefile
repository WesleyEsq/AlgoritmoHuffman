# Makefile para el Proyecto de Compresión Huffman
# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -O2
LDFLAGS = -lm -lrt

# Archivos fuente comunes
COMMON_SOURCES = tree.c readFile.c
HEADERS = tree.h

# Archivos objeto
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)

# Ejecutables
SERIAL_TARGET = huffman_serial
FORK_TARGET = huffman_fork
PTHREAD_TARGET = huffman_pthread
TEST_TARGET = huffman_test
#Menu principal
MENU_TARGET = huffman_cli
# Directorio para archivos de prueba
TEST_DIR = test_files
COMPRESSED_DIR = compressed_output
EXTRACTED_DIR = extracted_output

# Regla por defecto
all: $(SERIAL_TARGET) $(FORK_TARGET) $(PTHREAD_TARGET) info
full: all $(MENU_TARGET)

# Información del proyecto
info:
	@echo "=== Proyecto de Compresión Huffman ==="
	@echo "Ejecutables generados:"
	@echo "  $(SERIAL_TARGET)  - Versión serial"
	@echo "  $(FORK_TARGET)    - Versión paralela con fork()"
	@echo ""
	@echo "Uso:"
	@echo "  make test        - Ejecutar pruebas automáticas"
	@echo "  make clean       - Limpiar archivos generados"
	@echo "  make setup       - Crear archivos de prueba"
	@echo "  make pthread     - Compilar versión pthread (cuando esté lista)"
	@echo ""

# Compilar versión serial
$(SERIAL_TARGET): main_serial.c $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar versión fork
$(FORK_TARGET): main_fork.c readFile_fork.o $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar versión pthread
$(PTHREAD_TARGET): main_pthread.c readFile_pthread.c $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

# Compilar la aplicación 
$(MENU_TARGET): main_menu.c readFile.o readFile_fork.o readFile_pthread.o tree.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

# Compilar archivos objeto
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear archivos de prueba
setup:
	@echo "Creando estructura de directorios de prueba..."
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(COMPRESSED_DIR)
	@mkdir -p $(EXTRACTED_DIR)
	@echo "Este es un archivo de prueba simple." > $(TEST_DIR)/archivo1.txt
	@echo "Archivo de prueba con más contenido para el algoritmo de Huffman. Este texto debería comprimirse bien debido a la repetición de caracteres y palabras." > $(TEST_DIR)/archivo2.txt
	@echo "Tercer archivo con contenido diferente: números 1234567890 y símbolos !@#$$%^&*()." > $(TEST_DIR)/archivo3.txt
	@echo "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." > $(TEST_DIR)/archivo4.txt
	@echo "Archivo con caracteres especiales: áéíóú ñ ¿¡ ©®™" > $(TEST_DIR)/archivo5.txt
	@echo "✓ Archivos de prueba creados en $(TEST_DIR)/"

# Crear archivo de prueba grande
big-test:
	@echo "Creando archivo de prueba grande..."
	@for i in {1..1000}; do echo "Línea $$i: Este es un archivo grande para probar el rendimiento del algoritmo de Huffman con contenido repetitivo." >> $(TEST_DIR)/archivo_grande.txt; done
	@echo "✓ Archivo grande creado: $(TEST_DIR)/archivo_grande.txt"

# Ejecutar pruebas automáticas
test: $(SERIAL_TARGET) $(FORK_TARGET) setup
	@echo "=== EJECUTANDO PRUEBAS AUTOMÁTICAS ==="
	@echo ""
	@echo "--- Prueba 1: Versión Serial ---"
	@./$(SERIAL_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/test_serial.bin -x $(EXTRACTED_DIR)/serial
	@echo ""
	@echo "--- Prueba 2: Versión Fork ---"
	@./$(FORK_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/test_fork.bin -x $(EXTRACTED_DIR)/fork
	@echo ""
	@echo "--- Verificación de Integridad ---"
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
benchmark: $(SERIAL_TARGET) $(FORK_TARGET) big-test
	@echo "=== PRUEBAS DE RENDIMIENTO ==="
	@echo "Probando con archivo grande..."
	@time ./$(SERIAL_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/bench_serial.bin -x $(EXTRACTED_DIR)/bench_serial
	@time ./$(FORK_TARGET) -d $(TEST_DIR) -o $(COMPRESSED_DIR)/bench_fork.bin -x $(EXTRACTED_DIR)/bench_fork

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos generados..."
	@rm -f *.o
	@rm -f $(SERIAL_TARGET) $(FORK_TARGET) $(PTHREAD_TARGET) $(TEST_TARGET)
	@rm -rf $(TEST_DIR) $(COMPRESSED_DIR) $(EXTRACTED_DIR)
	@rm -f *.bin *.tmp
	@echo "✓ Limpieza completada"

# Limpiar solo ejecutables
clean-bin:
	@rm -f $(SERIAL_TARGET) $(FORK_TARGET) $(PTHREAD_TARGET) $(TEST_TARGET) $(PTHREAD_TARGET)
	@echo "✓ Ejecutables eliminados"

# Mostrar ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  make              - Compilar versiones serial y fork"
	@echo "  make all          - Compilar todas las versiones"
	@echo "  make serial       - Compilar solo versión serial"
	@echo "  make fork         - Compilar solo versión fork"
	@echo "  make pthread      - Compilar versión pthread"
	@echo "  make setup        - Crear archivos de prueba"
	@echo "  make big-test     - Crear archivo de prueba grande"
	@echo "  make test         - Ejecutar pruebas automáticas"
	@echo "  make benchmark    - Ejecutar pruebas de rendimiento"
	@echo "  make clean        - Limpiar todo"
	@echo "  make clean-bin    - Limpiar solo ejecutables"
	@echo "  make help         - Mostrar esta ayuda"

# Compilar solo versión serial
serial: $(SERIAL_TARGET)

# Compilar solo versión fork
fork: $(FORK_TARGET)

# Compilar solo versión pthread
pthread: $(PTHREAD_TARGET)

# Reglas que no crean archivos
.PHONY: all clean clean-bin setup test benchmark help info serial fork pthread big-test