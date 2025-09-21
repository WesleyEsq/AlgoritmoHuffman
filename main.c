#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "tree.h"

// Test
int main() {
    const char* original_file = "example.txt";
    const char* compressed_file = "example.bin";
    compressFile(original_file, compressed_file);
    printf("\n--- Archivo comprimido, iniciando reconstrucción de códigos ---\n");

    // 2. Llamar a la nueva función para obtener la tabla de códigos
    char** reconstructed_codes = reconstruirCodigos(compressed_file);

    // 3. Verificar y usar los códigos
    if (reconstructed_codes != NULL) {
        printf("Códigos reconstruidos con éxito:\n");
        for (int i = 0; i < 256; i++) {
            if (reconstructed_codes[i] != NULL) {
                printf("  ASCII %d: %s\n", i, reconstructed_codes[i]);
            }
        }
        // MUY IMPORTANTE: Liberar la memoria de los códigos cuando ya no se necesiten
        liberarCodigos(reconstructed_codes);
        printf("Memoria de los códigos liberada.\n");
    } else {
        printf("Fallo al reconstruir los códigos.\n");
    }

    return 0;
}