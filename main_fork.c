#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "tree.h"

int main() {
    printf("=== ALGORITMO DE HUFFMAN - VERSIÓN PARALELA CON FORK() ===\n");
    printf("PID del proceso principal: %d\n\n", getpid());
    
    // Configuración de archivos de prueba
    const char* test_dir = "./test_files";
    const char* compressed_dir_serial = "directorio_serial.bin";
    const char* compressed_dir_fork = "directorio_fork.bin";
    const char* output_dir_serial = "./test_files_serial";
    const char* output_dir_fork = "./test_files_fork";
    
    // Mostrar archivos a procesar
    printf("=== ARCHIVOS A PROCESAR ===\n");
    listFilesToCompress(test_dir);
    
    // ========================================================================
    // COMPARACIÓN: VERSIÓN SERIAL vs VERSIÓN FORK
    // ========================================================================
    
    printf("=== PRUEBA 1: COMPRESIÓN SERIAL ===\n");
    long long start_serial = getCurrentTimeMs();
    
    if (compressDirectory(test_dir, compressed_dir_serial)) {
        long long end_serial = getCurrentTimeMs();
        long long time_serial = end_serial - start_serial;
        printf("✓ Compresión serial completada en: %lld ms\n", time_serial);
    } else {
        printf("✗ Error en compresión serial\n");
        return 1;
    }
    
    printf("\n=== PRUEBA 2: COMPRESIÓN CON FORK() ===\n");
    long long start_fork = getCurrentTimeMs();
    
    if (compressDirectoryFork(test_dir, compressed_dir_fork)) {
        long long end_fork = getCurrentTimeMs();
        long long time_fork_compress = end_fork - start_fork;
        printf("✓ Compresión con fork() completada en: %lld ms\n", time_fork_compress);
        
        // Calcular aceleración
        if (time_fork_compress > 0) {
            double speedup_compress = (double)(end_serial - start_serial) / (double)time_fork_compress;
            printf("🚀 Aceleración en compresión: %.2fx\n", speedup_compress);
        }
    } else {
        printf("✗ Error en compresión con fork()\n");
        return 1;
    }
    
    // ========================================================================
    // DESCOMPRESIÓN: SERIAL vs FORK
    // ========================================================================
    
    printf("\n=== PRUEBA 3: DESCOMPRESIÓN SERIAL ===\n");
    start_serial = getCurrentTimeMs();
    
    if (decompressDirectory(compressed_dir_serial, output_dir_serial)) {
        end_serial = getCurrentTimeMs();
        long long time_serial_decomp = end_serial - start_serial;
        printf("✓ Descompresión serial completada en: %lld ms\n", time_serial_decomp);
    } else {
        printf("✗ Error en descompresión serial\n");
        return 1;
    }
    
    printf("\n=== PRUEBA 4: DESCOMPRESIÓN CON FORK() ===\n");
    start_fork = getCurrentTimeMs();
    
    if (decompressDirectoryFork(compressed_dir_fork, output_dir_fork)) {
        end_fork = getCurrentTimeMs();
        long long time_fork_decomp = end_fork - start_fork;
        printf("✓ Descompresión con fork() completada en: %lld ms\n", time_fork_decomp);
        
        // Calcular aceleración
        if (time_fork_decomp > 0) {
            double speedup_decomp = (double)time_serial_decomp / (double)time_fork_decomp;
            printf("🚀 Aceleración en descompresión: %.2fx\n", speedup_decomp);
        }
    } else {
        printf("✗ Error en descompresión con fork()\n");
        return 1;
    }
    
    // ========================================================================
    // VERIFICACIÓN DE INTEGRIDAD
    // ========================================================================
    
    printf("\n=== VERIFICACIÓN DE INTEGRIDAD ===\n");
    printf("Para verificar que ambas versiones producen los mismos resultados:\n");
    printf("1. Comparar archivos comprimidos:\n");
    printf("   diff %s %s\n", compressed_dir_serial, compressed_dir_fork);
    printf("   (Pueden diferir debido al orden de procesamiento)\n\n");
    
    printf("2. Comparar directorios descomprimidos:\n");
    printf("   diff -r %s %s\n", output_dir_serial, output_dir_fork);
    printf("   (Deberían ser idénticos)\n\n");
    
    printf("3. Comparar con directorio original:\n");
    printf("   diff -r %s %s\n", test_dir, output_dir_serial);
    printf("   diff -r %s %s\n", test_dir, output_dir_fork);
    printf("   (Ambos deberían ser idénticos al original)\n\n");
    
    // ========================================================================
    // RESUMEN DE RENDIMIENTO
    // ========================================================================
    
    printf("=== RESUMEN DE RENDIMIENTO ===\n");
    printf("Compresión serial:    %lld ms\n", end_serial - start_serial);
    printf("Compresión fork():    %lld ms\n", time_fork_compress);
    printf("Descompresión serial: %lld ms\n", time_serial_decomp);
    printf("Descompresión fork(): %lld ms\n", time_fork_decomp);
    
    // Calcular aceleraciones totales
    long long total_serial = (end_serial - start_serial) + time_serial_decomp;
    long long total_fork = time_fork_compress + time_fork_decomp;
    
    if (total_fork > 0) {
        double total_speedup = (double)total_serial / (double)total_fork;
        printf("\nTiempo total serial:  %lld ms\n", total_serial);
        printf("Tiempo total fork():  %lld ms\n", total_fork);
        printf("🚀 Aceleración total: %.2fx\n", total_speedup);
    }
    
    printf("\n=== PRUEBAS COMPLETADAS ===\n");
    return 0;
}