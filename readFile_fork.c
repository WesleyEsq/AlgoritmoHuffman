#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>      // Para fork(), getpid()
#include <sys/wait.h>    // Para wait(), waitpid()
#include <sys/stat.h>    // Para mkdir, stat
#include <dirent.h>      // Para opendir, readdir
#include <time.h>        // Para medir tiempo
#include "tree.h"

//--------------------------------------------------------------------------//
//                                                                          //
//                    Funciones auxiliares para fork()                     //
//                                                                          //
//--------------------------------------------------------------------------//


/**
 * @brief Imprime información del proceso
 * @param message Mensaje a mostrar
 */
void printProcessInfo(const char* message) {
    printf("[PID %d] %s\n", getpid(), message);
    fflush(stdout);
}

/**
 * @brief Cuenta archivos regulares en un directorio
 * @param inputDir Directorio a analizar
 * @return Número de archivos regulares
 */
int countFilesInDirectory(const char* inputDir) {
    DIR* dir = opendir(inputDir);
    if (dir == NULL) {
        return -1;
    }
    
    struct dirent* entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", inputDir, entry->d_name);
        
        if (isRegularFile(filepath)) {
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

//--------------------------------------------------------------------------//
//                                                                          //
//              Compresión de directorio con fork()                        //
//                                                                          //
//--------------------------------------------------------------------------//

/**
 * @brief Comprime todos los archivos de un directorio usando procesos paralelos con fork()
 * @param inputDir Directorio de entrada
 * @param outputFile Archivo binario de salida
 * @return true si la compresión fue exitosa, false en caso contrario
 */
bool compressDirectoryFork(const char* inputDir, const char* outputFile) {
    printProcessInfo("Iniciando compresión de directorio con fork()");
    long long startTime = getCurrentTimeMs();
    
    DIR* dir;
    struct dirent* entry;
    FILE* output;
    
    // Abrir el directorio
    dir = opendir(inputDir);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return false;
    }
    
    // Contar archivos
    int fileCount = countFilesInDirectory(inputDir);
    if (fileCount <= 0) {
        printf("No se encontraron archivos para comprimir\n");
        closedir(dir);
        return false;
    }
    
    printf("Archivos encontrados: %d\n", fileCount);
    
    // Crear arrays para almacenar información de los archivos
    char fileNames[fileCount][256];
    char tempFiles[fileCount][1024];
    pid_t childPids[fileCount];
    int fileIndex = 0;
    
    // Primera pasada: recopilar nombres de archivos
    rewinddir(dir);
    while ((entry = readdir(dir)) != NULL && fileIndex < fileCount) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", inputDir, entry->d_name);
        
        if (isRegularFile(filepath)) {
            strcpy(fileNames[fileIndex], entry->d_name);
            snprintf(tempFiles[fileIndex], sizeof(tempFiles[fileIndex]), 
                    "/tmp/temp_compressed_fork_%d_%d.bin", getpid(), fileIndex);
            fileIndex++;
        }
    }
    closedir(dir);
    
    printf("Creando %d procesos hijos para compresión paralela...\n", fileCount);
    
    // Crear procesos hijos para comprimir archivos en paralelo
    for (int i = 0; i < fileCount; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // PROCESO HIJO: comprimir un archivo específico
            char inputFilePath[1024];
            snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", inputDir, fileNames[i]);
            
            printProcessInfo("Comprimiendo archivo");
            printf("[PID %d] Archivo: %s -> %s\n", getpid(), fileNames[i], tempFiles[i]);
            
            // Comprimir el archivo
            compressFile(inputFilePath, tempFiles[i]);
            
            printProcessInfo("Compresión completada");
            exit(0); // El hijo termina aquí
            
        } else if (pid > 0) {
            // PROCESO PADRE: guardar PID del hijo
            childPids[i] = pid;
            printf("[PID %d] Creado proceso hijo [PID %d] para: %s\n", 
                   getpid(), pid, fileNames[i]);
            
        } else {
            // Error en fork()
            perror("Error en fork()");
            return false;
        }
    }
    
    // PROCESO PADRE: esperar a que todos los hijos terminen
    printProcessInfo("Esperando a que terminen todos los procesos hijos...");
    for (int i = 0; i < fileCount; i++) {
        int status;
        waitpid(childPids[i], &status, 0);
        printf("[PID %d] Proceso hijo [PID %d] terminado (archivo: %s)\n", 
               getpid(), childPids[i], fileNames[i]);
    }
    
    // Crear el archivo final combinando todos los archivos temporales
    printProcessInfo("Combinando archivos comprimidos...");
    output = fopen(outputFile, "wb");
    if (output == NULL) {
        perror("Error al crear el archivo de salida");
        return false;
    }
    
    // Escribir número de archivos
    fwrite(&fileCount, sizeof(int), 1, output);
    
    // Combinar todos los archivos temporales
    for (int i = 0; i < fileCount; i++) {
        int nameLength = strlen(fileNames[i]);
        fwrite(&nameLength, sizeof(int), 1, output);
        fwrite(fileNames[i], sizeof(char), nameLength, output);
        
        // Leer archivo temporal y obtener su tamaño
        FILE* tempFile = fopen(tempFiles[i], "rb");
        if (tempFile == NULL) {
            fprintf(stderr, "Error al abrir archivo temporal: %s\n", tempFiles[i]);
            continue;
        }
        
        fseek(tempFile, 0, SEEK_END);
        long long compressedSize = ftell(tempFile);
        fseek(tempFile, 0, SEEK_SET);
        
        // Escribir tamaño y contenido
        fwrite(&compressedSize, sizeof(long long), 1, output);
        
        char buffer[4096];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), tempFile)) > 0) {
            fwrite(buffer, 1, bytesRead, output);
        }
        
        fclose(tempFile);
        remove(tempFiles[i]);
        
        printf("Archivo combinado: %s (%lld bytes)\n", fileNames[i], compressedSize);
    }
    
    fclose(output);
    
    long long endTime = getCurrentTimeMs();
    long long totalTime = endTime - startTime;
    
    printf("\n=== COMPRESIÓN CON FORK() COMPLETADA ===\n");
    printf("Archivos procesados: %d\n", fileCount);
    printf("Tiempo total: %lld ms\n", totalTime);
    printf("Archivo de salida: %s\n", outputFile);
    
    return true;
}

//--------------------------------------------------------------------------//
//                                                                          //
//              Descompresión de directorio con fork()                     //
//                                                                          //
//--------------------------------------------------------------------------//

/**
 * @brief Descomprime un directorio usando procesos paralelos con fork()
 * @param compressedFile Archivo comprimido
 * @param outputDir Directorio de salida
 * @return true si fue exitoso, false en caso contrario
 */
bool decompressDirectoryFork(const char* compressedFile, const char* outputDir) {
    printProcessInfo("Iniciando descompresión de directorio con fork()");
    long long startTime = getCurrentTimeMs();
    
    FILE* input = fopen(compressedFile, "rb");
    if (input == NULL) {
        perror("Error al abrir el archivo comprimido");
        return false;
    }
    
    // Crear directorio de salida
    if (!createDirectoryIfNotExists(outputDir)) {
        fclose(input);
        return false;
    }
    
    // Leer número de archivos
    int fileCount;
    if (fread(&fileCount, sizeof(int), 1, input) != 1) {
        fprintf(stderr, "Error al leer el número de archivos\n");
        fclose(input);
        return false;
    }
    
    printf("Descomprimiendo %d archivos con fork()...\n", fileCount);
    
    // Arrays para almacenar información
    char fileNames[fileCount][256];
    char tempCompressedFiles[fileCount][1024];
    char outputFilePaths[fileCount][1024];
    pid_t childPids[fileCount];
    
    // Extraer todos los archivos comprimidos a archivos temporales
    for (int i = 0; i < fileCount; i++) {
        int nameLength;
        fread(&nameLength, sizeof(int), 1, input);
        fread(fileNames[i], sizeof(char), nameLength, input);
        fileNames[i][nameLength] = '\0';
        
        long long compressedSize;
        fread(&compressedSize, sizeof(long long), 1, input);
        
        // Crear archivo temporal
        snprintf(tempCompressedFiles[i], sizeof(tempCompressedFiles[i]), 
                "/tmp/temp_extract_fork_%d_%d.bin", getpid(), i);
        snprintf(outputFilePaths[i], sizeof(outputFilePaths[i]), 
                "%s/%s", outputDir, fileNames[i]);
        
        FILE* tempFile = fopen(tempCompressedFiles[i], "wb");
        if (tempFile == NULL) {
            fprintf(stderr, "Error al crear archivo temporal para: %s\n", fileNames[i]);
            continue;
        }
        
        // Copiar datos comprimidos
        char buffer[4096];
        long long bytesRemaining = compressedSize;
        while (bytesRemaining > 0) {
            size_t bytesToRead = (bytesRemaining < sizeof(buffer)) ? bytesRemaining : sizeof(buffer);
            size_t bytesRead = fread(buffer, 1, bytesToRead, input);
            fwrite(buffer, 1, bytesRead, tempFile);
            bytesRemaining -= bytesRead;
        }
        
        fclose(tempFile);
        printf("Extraído archivo temporal: %s\n", fileNames[i]);
    }
    
    fclose(input);
    
    // Crear procesos hijos para descomprimir en paralelo
    printf("Creando %d procesos hijos para descompresión paralela...\n", fileCount);
    
    for (int i = 0; i < fileCount; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // PROCESO HIJO: descomprimir un archivo específico
            printProcessInfo("Descomprimiendo archivo");
            printf("[PID %d] Archivo: %s -> %s\n", getpid(), fileNames[i], outputFilePaths[i]);
            
            if (decompressFile(tempCompressedFiles[i], outputFilePaths[i])) {
                printProcessInfo("Descompresión completada");
            } else {
                printProcessInfo("Error en descompresión");
            }
            
            // Limpiar archivo temporal
            remove(tempCompressedFiles[i]);
            exit(0);
            
        } else if (pid > 0) {
            // PROCESO PADRE: guardar PID
            childPids[i] = pid;
            printf("[PID %d] Creado proceso hijo [PID %d] para: %s\n", 
                   getpid(), pid, fileNames[i]);
                   
        } else {
            perror("Error en fork()");
            return false;
        }
    }
    
    // PROCESO PADRE: esperar a todos los hijos
    printProcessInfo("Esperando a que terminen todos los procesos hijos...");
    for (int i = 0; i < fileCount; i++) {
        int status;
        waitpid(childPids[i], &status, 0);
        printf("[PID %d] Proceso hijo [PID %d] terminado (archivo: %s)\n", 
               getpid(), childPids[i], fileNames[i]);
    }
    
    long long endTime = getCurrentTimeMs();
    long long totalTime = endTime - startTime;
    
    printf("\n=== DESCOMPRESIÓN CON FORK() COMPLETADA ===\n");
    printf("Archivos procesados: %d\n", fileCount);
    printf("Tiempo total: %lld ms\n", totalTime);
    printf("Directorio de salida: %s\n", outputDir);
    
    return true;
}