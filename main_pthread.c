#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <time.h>     

// Incluimos el header con todas nuestras funciones
#include "tree.h"

// --- Prototipos de funciones del menú ---
void clearScreen();
void printHeader(const char* title);
int getMenuChoice();
long long getCurrentTimeMs(); // Para benchmarking

// --- Función Principal ---
int main() {
    char inputPath[1024];
    char outputPath[1024];
    bool isInputDirectory = false;

    while (true) {
        // --- ETAPA 1: SELECCIONAR ENTRADA ---
        clearScreen();
        printHeader("ETAPA 1: SELECCIONAR ARCHIVO O DIRECTORIO");
        printf("Introduce la ruta del archivo o directorio a procesar.\n");
        printf("Escribe 'salir' para terminar el programa.\n\n");
        printf("Ruta: ");

        if (fgets(inputPath, sizeof(inputPath), stdin) == NULL) continue;
        inputPath[strcspn(inputPath, "\n")] = 0;

        if (strcmp(inputPath, "salir") == 0) break;

        struct stat path_stat;
        if (stat(inputPath, &path_stat) != 0) {
            printf("\nError: La ruta '%s' no existe. Presiona Enter para intentar de nuevo.", inputPath);
            getchar();
            continue;
        }
        isInputDirectory = S_ISDIR(path_stat.st_mode);
        
        printf("\nEntrada detectada como: %s\n", isInputDirectory ? "DIRECTORIO" : "ARCHIVO");
        printf("Presiona Enter para continuar...");
        getchar();

        // --- ETAPA 2: COMPRIMIR O DESCOMPRIMIR ---
        int actionChoice = 0;
        while (true) {
            clearScreen();
            printHeader("ETAPA 2: ELEGIR ACCIÓN");
            printf("Entrada seleccionada: %s\n\n", inputPath);
            printf("1. Comprimir\n");
            printf("2. Descomprimir\n");
            printf("--------------------\n");
            printf("9. Volver (elegir otra ruta)\n");
            printf("0. Salir del programa\n\n");
            printf("Opción: ");
            
            actionChoice = getMenuChoice();
            if (actionChoice == 1 || actionChoice == 2 || actionChoice == 9 || actionChoice == 0) break;
            printf("\nOpción no válida. Presiona Enter para intentar de nuevo.");
            getchar();
        }

        if (actionChoice == 9) continue;
        if (actionChoice == 0) break;

        // --- ETAPA 3: ELEGIR ALGORITMO ---
        int algoChoice = 0;
        while (true) {
            clearScreen();
            printHeader("ETAPA 3: ELEGIR MÉTODO");
            printf("Entrada: %s (%s)\n", inputPath, isInputDirectory ? "Directorio" : "Archivo");
            printf("Acción: %s\n\n", actionChoice == 1 ? "Comprimir" : "Descomprimir");
            printf("1. Serial (Secuencial)\n");
            printf("2. Fork (Procesos Paralelos)\n");
            printf("3. Pthread (Hilos Paralelos)\n");
            printf("--------------------\n");
            printf("9. Volver (elegir otra acción)\n");
            printf("0. Salir del programa\n\n");
            printf("Opción: ");

            algoChoice = getMenuChoice();
            if ((algoChoice >= 1 && algoChoice <= 3) || algoChoice == 9 || algoChoice == 0) break;
            printf("\nOpción no válida. Presiona Enter para intentar de nuevo.");
            getchar();
        }

        if (algoChoice == 9) continue;
        if (algoChoice == 0) break;

        // --- RESUMEN Y EJECUCIÓN ---
        clearScreen();
        printHeader("INICIANDO OPERACIÓN");
        
        // Definir nombre de salida
        if (actionChoice == 1) snprintf(outputPath, sizeof(outputPath), "%s.huff", inputPath);
        else snprintf(outputPath, sizeof(outputPath), "%s_descomprimido", inputPath);
        
        printf("  - Entrada: %s\n", inputPath);
        printf("  - Salida:  %s\n", outputPath);
        printf("  - Método:  %s\n\n", algoChoice == 1 ? "Serial" : (algoChoice == 2 ? "Fork" : "Pthread"));
        
        long long start_time = getCurrentTimeMs();

        if (actionChoice == 1) { // COMPRIMIR
            if (isInputDirectory) {
                if (algoChoice == 1) compressDirectory(inputPath, outputPath);
                if (algoChoice == 2) compressDirectoryFork(inputPath, outputPath);
                if (algoChoice == 3) compressDirectoryPthread(inputPath, outputPath);
            } else {
                // Para archivos individuales, usamos la versión paralela que es segura
                compressFileParallel(inputPath, outputPath);
            }
        } else { // DESCOMPRIMIR
             if (isInputDirectory) {
                if (algoChoice == 1) decompressDirectory(inputPath, outputPath);
                if (algoChoice == 2) decompressDirectoryFork(inputPath, outputPath);
                if (algoChoice == 3) decompressDirectoryPthread(inputPath, outputPath);
            } else {
                decompressFile(inputPath, outputPath);
            }
        }

        long long end_time = getCurrentTimeMs();
        printf("\n--------------------------------------------\n");
        printf("✓ Operación completada en %lld ms\n", end_time - start_time);
        printf("--------------------------------------------\n");
        printf("\nPresiona Enter para volver al menú principal.");
        getchar();
    }

    clearScreen();
    printf("¡Hasta luego!\n");
    return 0;
}

// --- Implementación de Funciones Auxiliares ---

void clearScreen() {
    system("clear || cls");
}

void printHeader(const char* title) {
    printf("============================================\n");
    printf("    COMPRESOR HUFFMAN - MENÚ INTERACTIVO\n");
    printf("--------------------------------------------\n");
    printf(":: %s\n", title);
    printf("============================================\n\n");
}

int getMenuChoice() {
    char buffer[10];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        return atoi(buffer);
    }
    return -1;
}

long long getCurrentTimeMs() {
    return (long long)((double)clock() * 1000 / CLOCKS_PER_SEC);
}