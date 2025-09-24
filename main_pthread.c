// Archivo: main_pthread.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "tree.h"
#include <time.h>


// Estructura para opciones de línea de comandos
typedef struct {
    char* input_dir;
    char* output_file;
    char* extract_dir;
    bool compress_only;
    bool decompress_only;
    bool benchmark;
    bool help;
} Options;

void printUsage(const char* program_name) {
    printf("Uso: %s [OPCIONES]\n", program_name);
    printf("\nCompresión y descompresión con Pthreads\n");
    printf("  -d, --directory DIR     Directorio a comprimir\n");
    printf("  -o, --output FILE       Archivo de salida (.bin)\n");
    printf("  -x, --extract DIR       Directorio donde extraer\n");
    printf("  -c, --compress-only     Solo comprimir\n");
    printf("  -u, --decompress-only   Solo descomprimir\n");
    printf("  -b, --benchmark         Comparar con la versión serial\n");
    printf("  -h, --help              Mostrar esta ayuda\n");
}

/**
 * @brief Mide el tiempo actual en milisegundos.
 * @return Tiempo actual en milisegundos.
 */
long long getCurrentTimeMs() {
    return (long long)((double)clock() * 1000 / CLOCKS_PER_SEC);
}


Options parseArguments(int argc, char* argv[]) {
    Options opts = {0};
    opts.input_dir = "./test_files";
    opts.output_file = "compressed_pthread.bin";
    opts.extract_dir = "./extracted_pthread";

    static struct option long_options[] = {
        {"directory", required_argument, 0, 'd'},
        {"output", required_argument, 0, 'o'},
        {"extract", required_argument, 0, 'x'},
        {"compress-only", no_argument, 0, 'c'},
        {"decompress-only", no_argument, 0, 'u'},
        {"benchmark", no_argument, 0, 'b'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "d:o:x:cubh", long_options, NULL)) != -1) {
        switch (c) {
            case 'd': opts.input_dir = optarg; break;
            case 'o': opts.output_file = optarg; break;
            case 'x': opts.extract_dir = optarg; break;
            case 'c': opts.compress_only = true; break;
            case 'u': opts.decompress_only = true; break;
            case 'b': opts.benchmark = true; break;
            case 'h': opts.help = true; break;
            default: exit(1);
        }
    }
    return opts;
}

int main(int argc, char* argv[]) {
    Options opts = parseArguments(argc, argv);

    if (opts.help) {
        printUsage(argv[0]);
        return 0;
    }

    printf("=== ALGORITMO DE HUFFMAN - VERSIÓN PTHREAD ===\n");

    long long pthread_compress_time = 0, serial_compress_time = 0;
    long long pthread_decompress_time = 0, serial_decompress_time = 0;

    // --- FASE DE COMPRESIÓN ---
    if (!opts.decompress_only) {
        if (opts.benchmark) {
            printf("\n--- BENCHMARK: Ejecutando versión SERIAL ---\n");
            long long start = getCurrentTimeMs();
            compressDirectory(opts.input_dir, "temp_serial.bin");
            long long end = getCurrentTimeMs();
            serial_compress_time = end - start;
            printf("✓ Compresión serial: %lld ms\n", serial_compress_time);
        }
        
        printf("\n--- Ejecutando versión PTHREAD ---\n");
        long long start = getCurrentTimeMs();
        compressDirectoryPthread(opts.input_dir, opts.output_file);
        long long end = getCurrentTimeMs();
        pthread_compress_time = end - start;
        printf("✓ Compresión con Pthread: %lld ms\n", pthread_compress_time);
    }

    // --- FASE DE DESCOMPRESIÓN ---
    // (Asumiendo que decompressDirectoryPthread existe)
    if (!opts.compress_only) {
         if (opts.benchmark) {
            printf("\n--- BENCHMARK: Ejecutando descompresión SERIAL ---\n");
            long long start = getCurrentTimeMs();
            decompressDirectory(opts.output_file, "temp_extracted_serial");
            long long end = getCurrentTimeMs();
            serial_decompress_time = end - start;
            printf("✓ Descompresión serial: %lld ms\n", serial_decompress_time);
        }

        printf("\n--- Ejecutando descompresión PTHREAD ---\n");
        long long start = getCurrentTimeMs();
        decompressDirectoryPthread(opts.output_file, opts.extract_dir);
        long long end = getCurrentTimeMs();
        pthread_decompress_time = end - start;
        printf("✓ Descompresión con Pthread: %lld ms\n", pthread_decompress_time);
    }
    
    // --- RESUMEN DE RENDIMIENTO ---
    if (opts.benchmark) {
        printf("\n=== RESUMEN DE RENDIMIENTO ===\n");
        if(serial_compress_time > 0 && pthread_compress_time > 0){
            printf("Compresión:\n");
            printf("  Serial: %lld ms\n", serial_compress_time);
            printf("  Pthread: %lld ms\n", pthread_compress_time);
            printf("  Aceleración: %.2fx\n", (double)serial_compress_time / pthread_compress_time);
        }
        if(serial_decompress_time > 0 && pthread_decompress_time > 0){
            printf("Descompresión:\n");
            printf("  Serial: %lld ms\n", serial_decompress_time);
            printf("  Pthread: %lld ms\n", pthread_decompress_time);
            printf("  Aceleración: %.2fx\n", (double)serial_decompress_time / pthread_decompress_time);
        }
    }

    return 0;
}