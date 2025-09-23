#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>      
#include <sys/types.h>   
#include "tree.h"

// Estructura para opciones de línea de comandos
typedef struct {
    char* input_dir;
    char* output_file;
    char* extract_dir;
    bool compress_only;
    bool decompress_only;
    bool verbose;
    bool benchmark;
    bool help;
} Options;

void printUsage(const char* program_name) {
    printf("Uso: %s [OPCIONES]\n", program_name);
    printf("\nCompresión y descompresión de directorios usando el algoritmo de Huffman (Versión Fork)\n");
    printf("\nOpciones:\n");
    printf("  -d, --directory DIR     Directorio a comprimir\n");
    printf("  -o, --output FILE       Archivo de salida comprimido (.bin)\n");
    printf("  -x, --extract DIR       Directorio donde extraer archivos\n");
    printf("  -c, --compress-only     Solo comprimir (no descomprimir)\n");
    printf("  -u, --decompress-only   Solo descomprimir (especificar -o como entrada)\n");
    printf("  -b, --benchmark         Comparar rendimiento con versión serial\n");
    printf("  -v, --verbose           Mostrar información detallada del proceso\n");
    printf("  -h, --help              Mostrar esta ayuda\n");
    printf("\nEjemplos:\n");
    printf("  %s -d ./textos -o archivo.bin -x ./extraidos\n", program_name);
    printf("  %s -d ./textos -o archivo.bin -c -v\n", program_name);
    printf("  %s -o archivo.bin -x ./extraidos -u\n", program_name);
    printf("  %s -d ./textos -o archivo.bin -b\n", program_name);
    printf("\n");
}

Options parseArguments(int argc, char* argv[]) {
    Options opts = {0};
    
    // Opciones por defecto
    opts.input_dir = "./test_files";
    opts.output_file = "compressed_fork.bin";
    opts.extract_dir = "./extracted_fork";
    opts.compress_only = false;
    opts.decompress_only = false;
    opts.verbose = false;
    opts.benchmark = false;
    opts.help = false;
    
    static struct option long_options[] = {
        {"directory", required_argument, 0, 'd'},
        {"output", required_argument, 0, 'o'},
        {"extract", required_argument, 0, 'x'},
        {"compress-only", no_argument, 0, 'c'},
        {"decompress-only", no_argument, 0, 'u'},
        {"benchmark", no_argument, 0, 'b'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "d:o:x:cubvh", long_options, NULL)) != -1) {
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
            case 'b':
                opts.benchmark = true;
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

int main(int argc, char* argv[]) {
    Options opts = parseArguments(argc, argv);
    
    if (opts.help) {
        printUsage(argv[0]);
        return 0;
    }
    
    printf("=== ALGORITMO DE HUFFMAN - VERSIÓN PARALELA CON FORK() ===\n");
    printf("PID del proceso principal: %d\n", getpid());
    
    if (opts.verbose) {
        printf("\nConfiguración:\n");
        printf("  Directorio de entrada: %s\n", opts.input_dir);
        printf("  Archivo comprimido: %s\n", opts.output_file);
        printf("  Directorio de extracción: %s\n", opts.extract_dir);
        printf("  Solo comprimir: %s\n", opts.compress_only ? "Sí" : "No");
        printf("  Solo descomprimir: %s\n", opts.decompress_only ? "Sí" : "No");
        printf("  Modo benchmark: %s\n", opts.benchmark ? "Sí" : "No");
        printf("\n");
    }
    
    // Variables para medición de tiempo 
    long long fork_compress_time = 0, fork_decompress_time = 0;
    long long serial_compress_time = 0, serial_decompress_time = 0;
    long long start, end;
    
    // MODO BENCHMARK: Comparar con versión serial
    if (opts.benchmark && !opts.decompress_only) {
        printf("=== BENCHMARK: COMPARANDO CON VERSIÓN SERIAL ===\n");
        
        // Crear archivos temporales para la comparación
        char serial_output[256];
        snprintf(serial_output, sizeof(serial_output), "%s_serial_temp.bin", opts.output_file);
        
        if (opts.verbose) {
            listFilesToCompress(opts.input_dir);
        }
        
        // Prueba serial
        printf("\n--- Ejecutando versión SERIAL ---\n");
        start = getCurrentTimeMs();
        if (compressDirectory(opts.input_dir, serial_output)) {
            end = getCurrentTimeMs();
            serial_compress_time = end - start;
            printf("✓ Compresión serial: %lld ms\n", serial_compress_time);
        } else {
            printf("✗ Error en compresión serial\n");
        }
        
        // Prueba fork
        printf("\n--- Ejecutando versión FORK() ---\n");
        start = getCurrentTimeMs();
        if (compressDirectoryFork(opts.input_dir, opts.output_file)) {
            end = getCurrentTimeMs();
            fork_compress_time = end - start;
            printf("✓ Compresión fork: %lld ms\n", fork_compress_time);
        } else {
            printf("✗ Error en compresión fork\n");
        }
        
        // Limpiar archivo temporal
        remove(serial_output);
        
    } else if (!opts.decompress_only) {
        // SOLO COMPRESIÓN CON FORK
        if (opts.verbose) {
            printf("=== ARCHIVOS A COMPRIMIR ===\n");
            listFilesToCompress(opts.input_dir);
        }
        
        printf("\n=== COMPRESIÓN CON FORK() ===\n");
        start = getCurrentTimeMs();
        
        if (compressDirectoryFork(opts.input_dir, opts.output_file)) {
            end = getCurrentTimeMs();
            fork_compress_time = end - start;
            printf("✓ Compresión completada en: %lld ms\n", fork_compress_time);
            
            if (opts.verbose) {
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
        if (opts.benchmark) {
            printf("\n=== BENCHMARK: DESCOMPRESIÓN ===\n");
            
            // Crear directorios temporales para la comparación
            char serial_extract[256];
            snprintf(serial_extract, sizeof(serial_extract), "%s_serial_temp", opts.extract_dir);
            
            // Prueba serial
            printf("\n--- Ejecutando descompresión SERIAL ---\n");
            start = getCurrentTimeMs();
            if (decompressDirectory(opts.output_file, serial_extract)) {
                end = getCurrentTimeMs();
                serial_decompress_time = end - start;
                printf("✓ Descompresión serial: %lld ms\n", serial_decompress_time);
            } else {
                printf("✗ Error en descompresión serial\n");
            }
            
            // Prueba fork
            printf("\n--- Ejecutando descompresión FORK() ---\n");
            start = getCurrentTimeMs();
            if (decompressDirectoryFork(opts.output_file, opts.extract_dir)) {
                end = getCurrentTimeMs();
                fork_decompress_time = end - start;
                printf("✓ Descompresión fork: %lld ms\n", fork_decompress_time);
            } else {
                printf("✗ Error en descompresión fork\n");
            }
            
            // Verificar que ambos métodos produzcan el mismo resultado
            char diff_cmd[512];
            snprintf(diff_cmd, sizeof(diff_cmd), "diff -r %s %s > /dev/null 2>&1", 
                    serial_extract, opts.extract_dir);
            if (system(diff_cmd) == 0) {
                printf("✓ Ambas versiones producen resultados idénticos\n");
            } else {
                printf("⚠ Las versiones producen resultados diferentes\n");
            }
            
            // Limpiar directorio temporal
            char rm_cmd[256];
            snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", serial_extract);
            system(rm_cmd);
            
        } else {
            // SOLO DESCOMPRESIÓN CON FORK
            printf("\n=== DESCOMPRESIÓN CON FORK() ===\n");
            start = getCurrentTimeMs();
            
            if (decompressDirectoryFork(opts.output_file, opts.extract_dir)) {
                end = getCurrentTimeMs();
                fork_decompress_time = end - start;
                printf("✓ Descompresión completada en: %lld ms\n", fork_decompress_time);
            } else {
                printf("✗ Error en la descompresión\n");
                return 1;
            }
        }
    }
    
    // RESUMEN DE RENDIMIENTO
    printf("\n=== RESUMEN DE RENDIMIENTO ===\n");
    
    if (opts.benchmark) {
        printf("COMPRESIÓN:\n");
        if (serial_compress_time > 0 && fork_compress_time > 0) {
            printf("  Serial: %lld ms\n", serial_compress_time);
            printf("  Fork:   %lld ms\n", fork_compress_time);
            double speedup = (double)serial_compress_time / (double)fork_compress_time;
            printf("  Aceleración: %.2fx\n", speedup);
        }
        
        printf("\nDESCOMPRESIÓN:\n");
        if (serial_decompress_time > 0 && fork_decompress_time > 0) {
            printf("  Serial: %lld ms\n", serial_decompress_time);
            printf("  Fork:   %lld ms\n", fork_decompress_time);
            double speedup = (double)serial_decompress_time / (double)fork_decompress_time;
            printf("  Aceleración: %.2fx\n", speedup);
        }
        
        printf("\nTOTAL:\n");
        long long total_serial = serial_compress_time + serial_decompress_time;
        long long total_fork = fork_compress_time + fork_decompress_time;
        if (total_serial > 0 && total_fork > 0) {
            printf("  Serial: %lld ms\n", total_serial);
            printf("  Fork:   %lld ms\n", total_fork);
            double total_speedup = (double)total_serial / (double)total_fork;
            printf("  Aceleración total: %.2fx\n", total_speedup);
        }
    } else {
        if (fork_compress_time > 0) {
            printf("Tiempo de compresión:   %lld ms\n", fork_compress_time);
        }
        if (fork_decompress_time > 0) {
            printf("Tiempo de descompresión: %lld ms\n", fork_decompress_time);
        }
        printf("Tiempo total:           %lld ms\n", fork_compress_time + fork_decompress_time);
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
        printf("\nInformación de procesos:\n");
        printf("  PID principal: %d\n", getpid());
        printf("  Procesos hijos creados durante la ejecución\n");
    }
    
    return 0;
}