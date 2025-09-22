#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "tree.h"

// Estructura para opciones de línea de comandos
typedef struct {
    char* input_dir;
    char* output_file;
    char* extract_dir;
    bool compress_only;
    bool decompress_only;
    bool verbose;
    bool help;
} Options;

void printUsage(const char* program_name) {
    printf("Uso: %s [OPCIONES]\n", program_name);
    printf("\nCompresión y descompresión de directorios usando el algoritmo de Huffman (Versión Serial)\n");
    printf("\nOpciones:\n");
    printf("  -d, --directory DIR     Directorio a comprimir\n");
    printf("  -o, --output FILE       Archivo de salida comprimido (.bin)\n");
    printf("  -x, --extract DIR       Directorio donde extraer archivos\n");
    printf("  -c, --compress-only     Solo comprimir (no descomprimir)\n");
    printf("  -u, --decompress-only   Solo descomprimir (especificar -o como entrada)\n");
    printf("  -v, --verbose           Mostrar información detallada\n");
    printf("  -h, --help              Mostrar esta ayuda\n");
    printf("\nEjemplos:\n");
    printf("  %s -d ./textos -o archivo.bin -x ./extraidos\n", program_name);
    printf("  %s -d ./textos -o archivo.bin -c\n", program_name);
    printf("  %s -o archivo.bin -x ./extraidos -u\n", program_name);
    printf("\n");
}

Options parseArguments(int argc, char* argv[]) {
    Options opts = {0};
    
    // Opciones por defecto
    opts.input_dir = "./test_files";
    opts.output_file = "compressed_serial.bin";
    opts.extract_dir = "./extracted_serial";
    opts.compress_only = false;
    opts.decompress_only = false;
    opts.verbose = false;
    opts.help = false;
    
    static struct option long_options[] = {
        {"directory", required_argument, 0, 'd'},
        {"output", required_argument, 0, 'o'},
        {"extract", required_argument, 0, 'x'},
        {"compress-only", no_argument, 0, 'c'},
        {"decompress-only", no_argument, 0, 'u'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "d:o:x:cuvh", long_options, NULL)) != -1) {
        switch (c) {
            case 'd':
                opts.input_dir = optarg;
                break;
            case 'o':
                opts.output_file = optarg;
                break;
            case 'x':
                opts.extract_dir = optarg;
                break;
            case 'c':
                opts.compress_only = true;
                break;
            case 'u':
                opts.decompress_only = true;
                break;
            case 'v':
                opts.verbose = true;
                break;
            case 'h':
                opts.help = true;
                break;
            case '?':
                fprintf(stderr, "Opción desconocida. Use -h para ayuda.\n");
                exit(1);
                break;
        }
    }
    
    return opts;
}

long long getCurrentTimeMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
}

int main(int argc, char* argv[]) {
    Options opts = parseArguments(argc, argv);
    
    if (opts.help) {
        printUsage(argv[0]);
        return 0;
    }
    
    printf("=== ALGORITMO DE HUFFMAN - VERSIÓN SERIAL ===\n");
    
    if (opts.verbose) {
        printf("Configuración:\n");
        printf("  Directorio de entrada: %s\n", opts.input_dir);
        printf("  Archivo comprimido: %s\n", opts.output_file);
        printf("  Directorio de extracción: %s\n", opts.extract_dir);
        printf("  Solo comprimir: %s\n", opts.compress_only ? "Sí" : "No");
        printf("  Solo descomprimir: %s\n", opts.decompress_only ? "Sí" : "No");
        printf("\n");
    }
    
    long long total_start = getCurrentTimeMs();
    long long compress_time = 0, decompress_time = 0;
    
    // FASE DE COMPRESIÓN
    if (!opts.decompress_only) {
        if (opts.verbose) {
            printf("=== ARCHIVOS A COMPRIMIR ===\n");
            listFilesToCompress(opts.input_dir);
        }
        
        printf("=== COMPRESIÓN SERIAL ===\n");
        long long start = getCurrentTimeMs();
        
        if (compressDirectory(opts.input_dir, opts.output_file)) {
            long long end = getCurrentTimeMs();
            compress_time = end - start;
            printf("✓ Compresión completada en: %lld ms\n", compress_time);
            
            if (opts.verbose) {
                // Mostrar información del archivo comprimido
                printf("\nInformación del archivo comprimido:\n");
                listCompressedDirectoryContents(opts.output_file);
            }
        } else {
            printf("✗ Error en la compresión\n");
            return 1;
        }
    }
    
    // FASE DE DESCOMPRESIÓN
    if (!opts.compress_only) {
        printf("\n=== DESCOMPRESIÓN SERIAL ===\n");
        long long start = getCurrentTimeMs();
        
        if (decompressDirectory(opts.output_file, opts.extract_dir)) {
            long long end = getCurrentTimeMs();
            decompress_time = end - start;
            printf("✓ Descompresión completada en: %lld ms\n", decompress_time);
        } else {
            printf("✗ Error en la descompresión\n");
            return 1;
        }
    }
    
    // RESUMEN DE RENDIMIENTO
    long long total_end = getCurrentTimeMs();
    long long total_time = total_end - total_start;
    
    printf("\n=== RESUMEN DE RENDIMIENTO ===\n");
    if (compress_time > 0) {
        printf("Tiempo de compresión:   %lld ms\n", compress_time);
    }
    if (decompress_time > 0) {
        printf("Tiempo de descompresión: %lld ms\n", decompress_time);
    }
    printf("Tiempo total:           %lld ms\n", total_time);
    
    // VERIFICACIÓN DE INTEGRIDAD
    if (!opts.compress_only && !opts.decompress_only) {
        printf("\n=== VERIFICACIÓN DE INTEGRIDAD ===\n");
        printf("Para verificar que los archivos son idénticos:\n");
        printf("  diff -r %s %s\n", opts.input_dir, opts.extract_dir);
        printf("  (No debería mostrar diferencias)\n");
    }
    
    printf("\n=== PROCESO COMPLETADO ===\n");
    
    if (opts.verbose) {
        printf("\nArchivos generados:\n");
        if (!opts.decompress_only) {
            printf("  Archivo comprimido: %s\n", opts.output_file);
        }
        if (!opts.compress_only) {
            printf("  Directorio extraído: %s\n", opts.extract_dir);
        }
    }
    
    return 0;
}