#!/bin/bash

# Script de instalación automática para el Proyecto de Compresión Huffman
# Compatible con Debian/Ubuntu

set -e  # Salir si algún comando falla

echo "=== INSTALADOR DEL PROYECTO DE COMPRESIÓN HUFFMAN ==="
echo "Compatible con: Debian, Ubuntu y derivados"
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[⚠]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Verificar si estamos en un sistema compatible
check_system() {
    print_status "Verificando sistema operativo..."
    
    if [ -f /etc/debian_version ]; then
        print_success "Sistema Debian/Ubuntu detectado"
        DISTRO="debian"
    elif [ -f /etc/redhat-release ]; then
        print_warning "Sistema RedHat/CentOS detectado (no totalmente compatible)"
        DISTRO="redhat"
    else
        print_error "Sistema operativo no reconocido"
        print_error "Este script está optimizado para Debian/Ubuntu"
        read -p "¿Continuar de todos modos? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
        DISTRO="unknown"
    fi
}

# Verificar permisos
check_permissions() {
    print_status "Verificando permisos..."
    
    if [ "$EUID" -eq 0 ]; then
        print_warning "Ejecutándose como root (no recomendado para desarrollo)"
        SUDO=""
    else
        print_success "Ejecutándose como usuario normal"
        SUDO="sudo"
        
        # Verificar si sudo está disponible
        if ! command -v sudo &> /dev/null; then
            print_error "sudo no está instalado. Instale sudo o ejecute como root."
            exit 1
        fi
    fi
}

# Actualizar paquetes del sistema
update_system() {
    print_status "Actualizando lista de paquetes..."
    
    if [ "$DISTRO" = "debian" ]; then
        $SUDO apt-get update -qq
        print_success "Lista de paquetes actualizada"
    elif [ "$DISTRO" = "redhat" ]; then
        $SUDO yum update -y -q
        print_success "Sistema actualizado"
    fi
}

# Instalar dependencias
install_dependencies() {
    print_status "Instalando dependencias..."
    
    if [ "$DISTRO" = "debian" ]; then
        $SUDO apt-get install -y \
            build-essential \
            gcc \
            make \
            libc6-dev \
            manpages-dev \
            time \
            diffutils \
            coreutils
        print_success "Dependencias instaladas (Debian/Ubuntu)"
        
    elif [ "$DISTRO" = "redhat" ]; then
        $SUDO yum install -y \
            gcc \
            make \
            glibc-devel \
            time \
            diffutils \
            coreutils
        print_success "Dependencias instaladas (RedHat/CentOS)"
        
    else
        print_warning "Sistema desconocido - verificar dependencias manualmente"
        print_status "Dependencias requeridas:"
        echo "  - gcc (compilador C)"
        echo "  - make (herramienta de construcción)"
        echo "  - glibc-dev (bibliotecas de desarrollo C)"
        echo "  - time (para medición de rendimiento)"
        echo "  - diffutils (para verificación)"
    fi
}

# Verificar herramientas de compilación
verify_build_tools() {
    print_status "Verificando herramientas de compilación..."
    
    # Verificar GCC
    if command -v gcc &> /dev/null; then
        GCC_VERSION=$(gcc --version | head -n1)
        print_success "GCC encontrado: $GCC_VERSION"
    else
        print_error "GCC no encontrado"
        exit 1
    fi
    
    # Verificar Make
    if command -v make &> /dev/null; then
        MAKE_VERSION=$(make --version | head -n1)
        print_success "Make encontrado: $MAKE_VERSION"
    else
        print_error "Make no encontrado"
        exit 1
    fi
    
    # Verificar bibliotecas de matemáticas
    print_status "Verificando bibliotecas del sistema..."
    cat > /tmp/test_libs.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Bibliotecas del sistema OK\n");
    return 0;
}
EOF
    
    if gcc /tmp/test_libs.c -o /tmp/test_libs -lm 2>/dev/null; then
        /tmp/test_libs
        rm -f /tmp/test_libs /tmp/test_libs.c
        print_success "Bibliotecas del sistema verificadas"
    else
        print_error "Error en bibliotecas del sistema"
        rm -f /tmp/test_libs /tmp/test_libs.c
        exit 1
    fi
}

# Compilar el proyecto
compile_project() {
    print_status "Compilando proyecto..."
    
    # Verificar que estamos en el directorio correcto
    if [ ! -f "tree.h" ] || [ ! -f "tree.c" ]; then
        print_error "Archivos del proyecto no encontrados"
        print_error "Ejecute este script desde el directorio del proyecto"
        exit 1
    fi
    
    # Limpiar compilaciones anteriores
    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
    fi
    
    # Compilar usando Makefile si existe
    if [ -f "Makefile" ]; then
        print_status "Usando Makefile para compilar..."
        make all
        print_success "Compilación con Makefile completada"
    else
        # Compilar manualmente
        print_status "Compilando manualmente..."
        
        # Compilar versión serial
        if [ -f "main_serial.c" ]; then
            gcc -Wall -Wextra -std=gnu99 -O2 -o huffman_serial main_serial.c readFile.c tree.c -lm
            print_success "Versión serial compilada"
        fi
        
        # Compilar versión fork
        if [ -f "main_fork.c" ] && [ -f "readFile_fork.c" ]; then
            gcc -Wall -Wextra -std=gnu99 -O2 -o huffman_fork main_fork.c readFile.c readFile_fork.c tree.c -lm
            print_success "Versión fork compilada"
        fi
        
        # Compilar versión pthread si existe
        if [ -f "main_pthread.c" ] && [ -f "readFile_pthread.c" ]; then
            gcc -Wall -Wextra -std=gnu99 -O2 -o huffman_pthread main_pthread.c readFile.c readFile_pthread.c tree.c -lm -lpthread
            print_success "Versión pthread compilada"
        fi
    fi
}

# Crear archivos de prueba
setup_test_files() {
    print_status "Creando archivos de prueba..."
    
    if [ -f "Makefile" ]; then
        make setup
    else
        mkdir -p test_files
        echo "Este es un archivo de prueba simple para el algoritmo de Huffman." > test_files/archivo1.txt
        echo "Archivo con más contenido repetitivo: texto texto texto repetido repetido repetido." > test_files/archivo2.txt
        echo "Números y símbolos: 1234567890 !@#$%^&*() áéíóú ñ" > test_files/archivo3.txt
        
        # Crear archivo más grande
        for i in {1..100}; do
            echo "Línea $i: Contenido repetitivo para probar la compresión de Huffman con texto largo."
        done > test_files/archivo_grande.txt
    fi
    
    print_success "Archivos de prueba creados"
}

# Ejecutar pruebas básicas
run_basic_tests() {
    print_status "Ejecutando pruebas básicas de compilación..."
    
    # Probar versión serial (prueba rápida)
    if [ -f "huffman_serial" ]; then
        ./huffman_serial -h > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            print_success "Versión serial: Compilación OK"
        else
            print_error "Versión serial: FALLO"
        fi
    fi
    
    # Probar versión fork (prueba rápida)
    if [ -f "huffman_fork" ]; then
        ./huffman_fork -h > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            print_success "Versión fork: Compilación OK"
        else
            print_error "Versión fork: FALLO"
        fi
    fi
}

# Ejecutar demo automática
run_automatic_demo() {
    print_status "Ejecutando demo automática con archivos de prueba..."
    
    # Probar versión serial
    if [ -f "huffman_serial" ]; then
        print_status "Probando versión serial..."
        ./huffman_serial -d test_files -o demo_serial.bin -x demo_serial_out -v
        if [ $? -eq 0 ]; then
            print_success "Demo serial: OK"
        else
            print_error "Demo serial: FALLO"
        fi
    fi
    
    # Probar versión fork
    if [ -f "huffman_fork" ]; then
        print_status "Probando versión fork con benchmark..."
        ./huffman_fork -d test_files -o demo_fork.bin -x demo_fork_out -b
        if [ $? -eq 0 ]; then
            print_success "Demo fork: OK"
        else
            print_error "Demo fork: FALLO"
        fi
    fi
    
    # Verificar integridad
    if [ -d "test_files" ] && [ -d "demo_serial_out" ]; then
        if diff -r test_files demo_serial_out > /dev/null 2>&1; then
            print_success "Verificación de integridad: OK"
        else
            print_warning "Verificación de integridad: Diferencias encontradas"
        fi
    fi
    
    echo ""
    print_success "Demo automática completada exitosamente"
}

# Ejecutar configuración manual
run_manual_demo() {
    echo ""
    print_status "=== CONFIGURACIÓN MANUAL ==="
    echo ""
    
    # Solicitar directorio de entrada
    while true; do
        echo -n "📁 Ingrese el directorio a comprimir [./test_files]: "
        read input_dir
        
        # Usar valor por defecto si está vacío
        if [ -z "$input_dir" ]; then
            input_dir="./test_files"
        fi
        
        # Verificar que el directorio existe
        if [ -d "$input_dir" ]; then
            break
        else
            print_error "El directorio '$input_dir' no existe. Inténtelo de nuevo."
        fi
    done
    
    # Mostrar archivos en el directorio
    echo ""
    print_status "Archivos encontrados en '$input_dir':"
    ls -la "$input_dir" | grep -v "^d" | awk '{print "  • " $9}' | grep -v "^  • $"
    echo ""
    
    # Solicitar archivo de salida
    echo -n "💾 Nombre del archivo comprimido [mi_compresion.bin]: "
    read output_file
    if [ -z "$output_file" ]; then
        output_file="mi_compresion.bin"
    fi
    
    # Solicitar directorio de extracción
    echo -n "📂 Directorio donde extraer [./extraidos]: "
    read extract_dir
    if [ -z "$extract_dir" ]; then
        extract_dir="./extraidos"
    fi
    
    # Preguntar qué versión usar
    echo ""
    echo "Versiones disponibles:"
    [ -f "huffman_serial" ] && echo "  1) Serial (secuencial)"
    [ -f "huffman_fork" ] && echo "  2) Fork (paralelo con procesos)"
    [ -f "huffman_pthread" ] && echo "  3) Pthread (paralelo con hilos)"
    echo "  4) Comparar serial vs fork"
    echo ""
    
    while true; do
        echo -n "🚀 Seleccione la versión a usar [1]: "
        read version_choice
        
        if [ -z "$version_choice" ]; then
            version_choice=1
        fi
        
        case $version_choice in
            1)
                if [ -f "huffman_serial" ]; then
                    print_status "Ejecutando versión SERIAL..."
                    ./huffman_serial -d "$input_dir" -o "$output_file" -x "$extract_dir" -v
                    break
                else
                    print_error "huffman_serial no está disponible"
                fi
                ;;
            2)
                if [ -f "huffman_fork" ]; then
                    print_status "Ejecutando versión FORK..."
                    ./huffman_fork -d "$input_dir" -o "$output_file" -x "$extract_dir" -v
                    break
                else
                    print_error "huffman_fork no está disponible"
                fi
                ;;
            3)
                if [ -f "huffman_pthread" ]; then
                    print_status "Ejecutando versión PTHREAD..."
                    ./huffman_pthread -d "$input_dir" -o "$output_file" -x "$extract_dir" -v
                    break
                else
                    print_error "huffman_pthread no está disponible aún"
                fi
                ;;
            4)
                if [ -f "huffman_fork" ]; then
                    print_status "Ejecutando COMPARACIÓN Serial vs Fork..."
                    ./huffman_fork -d "$input_dir" -o "$output_file" -x "$extract_dir" -b
                    break
                else
                    print_error "huffman_fork no está disponible para comparación"
                fi
                ;;
            *)
                print_error "Opción inválida. Seleccione 1, 2, 3 o 4."
                ;;
        esac
    done
    
    # Verificar integridad si se hizo compresión y descompresión completa
    if [ -d "$extract_dir" ]; then
        echo ""
        print_status "Verificando integridad de los archivos..."
        if diff -r "$input_dir" "$extract_dir" > /dev/null 2>&1; then
            print_success "✓ Los archivos son idénticos al original"
        else
            print_warning "⚠ Se encontraron diferencias entre original y extraído"
            echo "Para ver las diferencias: diff -r '$input_dir' '$extract_dir'"
        fi
    fi
    
    echo ""
    print_success "Configuración manual completada exitosamente"
    echo ""
    echo "Archivos generados:"
    echo "  📁 Directorio original: $input_dir"
    echo "  💾 Archivo comprimido: $output_file"
    echo "  📂 Directorio extraído: $extract_dir"
}

# Menú de opciones post-instalación
post_installation_menu() {
    echo ""
    echo "=== INSTALACIÓN COMPLETADA EXITOSAMENTE ==="
    echo ""
    print_success "El proyecto ha sido compilado y está listo para usar"
    echo ""
    echo "¿Qué desea hacer ahora?"
    echo ""
    echo "1) 🎮 Ejecutar demo automática (usar archivos de prueba creados)"
    echo "2) ⚙️  Configuración manual (elegir sus propios directorios)"
    echo "3) ❌ Salir (solo instalar, no ejecutar demo)"
    echo ""
    
    while true; do
        echo -n "Seleccione una opción [1]: "
        read choice
        
        if [ -z "$choice" ]; then
            choice=1
        fi
        
        case $choice in
            1)
                run_automatic_demo
                break
                ;;
            2)
                run_manual_demo
                break
                ;;
            3)
                print_status "Instalación completada. Use los ejecutables cuando guste."
                break
                ;;
            *)
                print_error "Opción inválida. Seleccione 1, 2 o 3."
                ;;
        esac
    done
}

# Mostrar información final
show_final_info() {
    echo ""
    echo "=== INFORMACIÓN DEL PROYECTO ==="
    echo ""
    echo "Ejecutables disponibles:"
    [ -f "huffman_serial" ] && echo "  • huffman_serial  - Versión serial"
    [ -f "huffman_fork" ] && echo "  • huffman_fork    - Versión paralela con fork()"
    [ -f "huffman_pthread" ] && echo "  • huffman_pthread - Versión concurrente con pthread()"
    echo ""
    echo "Comandos útiles para después:"
    echo "  make help         - Mostrar ayuda del Makefile"
    echo "  make test         - Ejecutar pruebas automáticas"
    echo "  make benchmark    - Pruebas de rendimiento"
    echo "  make clean        - Limpiar archivos generados"
    echo ""
    echo "Ejemplos de uso manual:"
    echo "  ./huffman_serial -d ./mis_textos -o mi_archivo.bin -x ./extraidos"
    echo "  ./huffman_fork -d ./mis_textos -o mi_archivo.bin -b  # Con benchmark"
    echo "  ./huffman_serial -h  # Ver todas las opciones"
    echo ""
    print_success "¡Proyecto listo para usar!"
    echo ""
}

# Función principal
main() {
    echo "Iniciando instalación automática..."
    echo ""
    
    check_system
    check_permissions
    update_system
    install_dependencies
    verify_build_tools
    compile_project
    setup_test_files
    run_basic_tests
    
    # Menú post-instalación
    post_installation_menu
    
    show_final_info
}

# Manejo de errores
trap 'print_error "Instalación interrumpida"; exit 1' INT TERM

# Ejecutar función principal
main "$@"