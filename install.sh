#!/bin/bash

# Script de instalación automática para el Proyecto
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

# Instalar dependencias (MODIFICADO: Se añadió 'ranger')
install_dependencies() {
    print_status "Instalando dependencias..."
    
    if [ "$DISTRO" = "debian" ]; then
        # Se añade 'ranger' a la lista de dependencias
        $SUDO apt-get install -y \
            build-essential \
            gcc \
            make \
            libc6-dev \
            manpages-dev \
            time \
            diffutils \
            coreutils \
            ranger
        print_success "Dependencias instaladas (Debian/Ubuntu)"
        
    elif [ "$DISTRO" = "redhat" ]; then
        # (Lógica para otros sistemas se mantiene)
        $SUDO yum install -y \
            gcc \
            make \
            glibc-devel \
            time \
            diffutils \
            coreutils \
            ranger
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

# (La función verify_build_tools no necesita cambios)
verify_build_tools() {
    print_status "Verificando herramientas de compilación..."
    
    # Verificar GCC
    if command -v gcc &> /dev/null; then
        GCC_VERSION=$(gcc --version | head -n1)
        print_success "GCC encontrado: $GCC_VERSION"
    else
        print_error "GCC no encontrado"; exit 1
    fi
    
    # Verificar Make
    if command -v make &> /dev/null; then
        MAKE_VERSION=$(make --version | head -n1)
        print_success "Make encontrado: $MAKE_VERSION"
    else
        print_error "Make no encontrado"
        exit 1
    fi
    print_status "Verificando bibliotecas del sistema..."
    cat > /tmp/test_libs.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
int main() { printf("Bibliotecas del sistema OK\n"); return 0; }
EOF
    if gcc /tmp/test_libs.c -o /tmp/test_libs -lm -pthread 2>/dev/null; then
        /tmp/test_libs
        rm -f /tmp/test_libs /tmp/test_libs.c
        print_success "Bibliotecas del sistema verificadas"
    else
        print_error "Error en bibliotecas del sistema"; rm -f /tmp/test_libs /tmp/test_libs.c; exit 1
    fi
}


# Compilar el proyecto (MODIFICADO: Se usa 'make full')
compile_project() {
    print_status "Compilando todos los ejecutables..."
    if [ ! -f "Makefile" ]; then
        print_error "Makefile no encontrado"; exit 1
    fi
    
    # Limpiar compilaciones anteriores
    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
    fi
    
    # Compilar usando Makefile si existe
    if [ -f "Makefile" ]; then
        print_status "Usando Makefile para compilar..."
        make full
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
    ./huffman_fork -d test_files -o demo_fork.bin -x demo_fork_out -b
    if diff -r test_files demo_fork_out > /dev/null 2>&1; then
        print_success "Verificación de integridad: OK"
    else
        print_warning "Verificación de integridad: Diferencias encontradas"
    fi
    echo ""
    print_success "Demo automática completada exitosamente"
}
run_manual_demo() {
    clear
    print_status "=== CONFIGURACIÓN MANUAL (MODO ANTIGUO) ==="
    # (Tu función original de configuración manual se mantiene intacta)
    echo ""
    echo -n "Ingrese el directorio a comprimir [./test_files]: "; read input_dir
    input_dir=${input_dir:-./test_files}
    # ... (el resto de la función se mantiene)
    # ...
    echo "Configuración manual completada. Presiona Enter para volver."
    read
}

# Menú de opciones post-instalación (MODIFICADO: Se añade la opción del nuevo menú)
post_installation_menu() {
    while true; do
        clear
        echo "======================================================"
        print_success "El proyecto ha sido compilado y está listo para usar"
        echo "======================================================"
        echo ""
        echo "¿Qué desea hacer ahora?"
        echo ""
        echo "1) Iniciar menú interactivo principal (Recomendado)"
        echo "2) Ejecutar demo automática (Benchmark y verificación)"
        echo "3) Ejecutar configuración manual (Modo antiguo)"
        echo "4) Salir (solo instalar, no ejecutar demo)"
        echo ""
        
        echo -n "Seleccione una opción [1]: "
        read choice
        
        case ${choice:-1} in
            1)
                ./huffman_cli
                break
                ;;
            2)
                run_automatic_demo
                echo "Presiona Enter para volver al menú."
                read
                ;;
            3)
                run_manual_demo
                ;;
            4)
                print_status "Instalación completada. Use los ejecutables cuando guste."
                break
                ;;
            *)
                print_error "Opción inválida. Seleccione 1, 2, 3 o 4."
                sleep 2
                ;;
        esac
    done
}

# (La función show_final_info no necesita cambios)
show_final_info() {
    clear
    echo "=== INFORMACIÓN DEL PROYECTO ==="
    # ... (tu función original se mantiene intacta)
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
    
    post_installation_menu
    
    show_final_info
}

# Manejo de errores
trap 'print_error "Instalación interrumpida"; exit 1' INT TERM

# Ejecutar función principal
main "$@"