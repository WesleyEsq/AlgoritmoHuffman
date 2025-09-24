#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h> // Para stat()
#include <time.h>     // Para clock()

// Incluimos el header con todas nuestras funciones
#include "tree.h"

// --- Prototipos de funciones del menú ---
void clearScreen();
void printHeader(const char* title);
int getMenuChoice();
long long getCurrentTimeMs();

// --- Función Principal ---
int main() {
    char inputPath[1024];
    char outputPath[1024];

    while (true) {
        // --- ETAPA 1: ELEGIR ACCIÓN ---
        clearScreen();
        printHeader("ETAPA 1: ELEGIR ACCIÓN");
        printf("¿Qué deseas hacer?\n\n");
        printf("1. Comprimir (un archivo o directorio)\n");
        printf("2. Descomprimir (un archivo .huff)\n");
        printf("--------------------\n");
        printf("0. Salir del programa\n\n");
        printf("Opción: ");
        
        int actionChoice = getMenuChoice();

        if (actionChoice == 0) break;
        if (actionChoice != 1 && actionChoice != 2) {
            printf("\nOpción no válida. Presiona Enter para intentar de nuevo.");
            getchar();
            continue;
        }

        // --- ETAPA 2: SELECCIONAR RUTA DE ENTRADA ---
        bool validInput = false;
        while (!validInput) {
            clearScreen();
            printHeader("ETAPA 2: SELECCIONAR ENTRADA");
            
            if (actionChoice == 1) printf("Introduce la ruta del ARCHIVO o DIRECTORIO que quieres COMPRIMIR.\n");
            else printf("Introduce la ruta del ARCHIVO (.huff) que quieres DESCOMPRIMIR.\n");
            
            printf("Escribe 'buscar' para usar el explorador de archivos.\n");
            printf("Escribe 'volver' para regresar al menú anterior.\n\n");
            printf("Ruta: ");

            if (fgets(inputPath, sizeof(inputPath), stdin) == NULL) continue;
            inputPath[strcspn(inputPath, "\n")] = 0;

            if (strcmp(inputPath, "volver") == 0) break;

            // --- Lógica de Búsqueda Adaptativa ---
            if (strcmp(inputPath, "buscar") == 0) {
                const char* tempFile = "/tmp/huffman_path_selection.txt";
                char command[512];
                clearScreen();
                printHeader("EXPLORADOR DE ARCHIVOS");

                if (actionChoice == 1) { // Comprimir: puede ser archivo o dir
                    printf("Puedes seleccionar un ARCHIVO ('Enter') o un DIRECTORIO ('q').\n\n");
                    printf("1. Seleccionar un Archivo\n");
                    printf("2. Seleccionar un Directorio\n\n");
                    printf("Opción: ");
                    int search_choice = getMenuChoice();
                    
                    if (search_choice == 1) snprintf(command, sizeof(command), "ranger --choosefile=%s", tempFile);
                    else snprintf(command, sizeof(command), "ranger --choosedir=%s", tempFile);

                } else { // Descomprimir: solo puede ser un archivo
                    printf("Navega hasta el ARCHIVO (.huff) y presiona 'Enter' para seleccionarlo.\n");
                    snprintf(command, sizeof(command), "ranger --choosefile=%s", tempFile);
                }
                
                printf("\nPresiona Enter para lanzar 'ranger'...");
                getchar();
                clearScreen();
                system(command);

                FILE* f = fopen(tempFile, "r");
                if (f && fgets(inputPath, sizeof(inputPath), f)) {
                    inputPath[strcspn(inputPath, "\n")] = 0;
                } else {
                    printf("\nNo se seleccionó nada. Presiona Enter para volver.");
                    getchar();
                    if(f) fclose(f);
                    remove(tempFile);
                    continue;
                }
                fclose(f);
                remove(tempFile);
            }

            // --- Validación de la ruta ---
            struct stat path_stat;
            if (stat(inputPath, &path_stat) != 0) {
                printf("\nError: La ruta '%s' no existe. Presiona Enter para intentar de nuevo.", inputPath);
                getchar();
                continue;
            }
            if (actionChoice == 2 && S_ISDIR(path_stat.st_mode)) {
                printf("\nError: Para descomprimir, debes seleccionar un ARCHIVO, no un directorio. Presiona Enter.");
                getchar();
                continue;
            }
            validInput = true;
        }

        if (!validInput) continue; // Si el usuario escribió 'volver'

        // --- ETAPA 3: ELEGIR ALGORITMO ---
        int algoChoice = 0;

        while(true){
             clearScreen();
            printHeader("ETAPA 3: ELEGIR MÉTODO");
            printf("Entrada: %s\n", inputPath);
            printf("Acción: %s\n\n", actionChoice == 1 ? "Comprimir" : "Descomprimir");
            printf("1. Serial (Secuencial)\n");
            printf("2. Fork (Procesos Paralelos)\n");
            printf("3. Pthread (Hilos Paralelos)\n");
            printf("--------------------\n");
            printf("9. Volver\n");
            printf("0. Salir\n\n");
            printf("Opción: ");
            algoChoice = getMenuChoice();
            if ((algoChoice >= 1 && algoChoice <= 3) || algoChoice == 9 || algoChoice == 0) break;
            printf("\nOpción no válida. Presiona Enter.");
            getchar();
        }

        if(algoChoice == 9) continue;
        if(algoChoice == 0) break;


        // --- EJECUCIÓN ---
        clearScreen();
        printHeader("INICIANDO OPERACIÓN");

        struct stat path_stat;
        stat(inputPath, &path_stat);
        bool isInputDirectory = S_ISDIR(path_stat.st_mode);

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
                compressFile(inputPath, outputPath);
            }
        } else { // DESCOMPRIMIR
            // Para descomprimir, siempre se llama a la versión de directorio,
            // ya que el formato del archivo binario es el mismo.
            // La función interna sabrá si hay uno o muchos archivos.
            // Yo que sé
            if (algoChoice == 1) decompressDirectory(inputPath, outputPath);
            if (algoChoice == 2) decompressDirectoryFork(inputPath, outputPath);
            if (algoChoice == 3) decompressDirectoryPthread(inputPath, outputPath);
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