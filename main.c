#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "tree.h"

// Test
int main() {
    const char* original_file = "example.txt";
    const char* compressed_file = "example.bin";
    const char* decompressed_file = "example_decompressed.txt";
    
    printf("=== PRUEBA DEL ALGORITMO DE HUFFMAN ===\n");
    printf("Archivo original: %s\n", original_file);
    printf("Archivo comprimido: %s\n", compressed_file);
    printf("Archivo descomprimido: %s\n", decompressed_file);
    
    // 1. Comprimir el archivo
    printf("\n--- Iniciando compresión ---\n");
    compressFile(original_file, compressed_file);
    printf("Archivo comprimido exitosamente.\n");
    
    printf("\n--- Archivo comprimido, iniciando reconstrucción de códigos ---\n");

    // 2. Llamar a la nueva función para obtener la tabla de códigos
    char** reconstructed_codes = reconstruirCodigos(compressed_file);

    // 3. Verificar y usar los códigos
    if (reconstructed_codes != NULL) {
        printf("Códigos reconstruidos con éxito:\n");
        for (int i = 0; i < 256; i++) {
            if (reconstructed_codes[i] != NULL) {
                printf("  ASCII %d ('%c'): %s\n", i, (char)i, reconstructed_codes[i]);
            }
        }
        // MUY IMPORTANTE: Liberar la memoria de los códigos cuando ya no se necesiten
        liberarCodigos(reconstructed_codes);
        printf("Memoria de los códigos liberada.\n");
    } else {
        printf("Fallo al reconstruir los códigos.\n");
        return 1;
    }

    // 4. Descomprimir el archivo
    printf("\n--- Iniciando descompresión ---\n");
    if (decompressFile(compressed_file, decompressed_file)) {
        printf("¡Descompresión exitosa!\n");
        printf("Archivo descomprimido guardado como: %s\n", decompressed_file);
        
        // Verificar que el contenido sea igual
        printf("\n--- Verificando integridad ---\n");
        printf("Compara manualmente los archivos:\n");
        printf("  Original: %s\n", original_file);
        printf("  Descomprimido: %s\n", decompressed_file);
        printf("Deberían ser idénticos.\n");
    } else {
        printf("Error en la descompresión\n");
        return 1;
    }

    printf("\n=== PRUEBA COMPLETADA ===\n");
    return 0;
}