#define _DEFAULT_SOURCE  // ¿? Cosa rara de stack overflow para solucionar un problema de una variable
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>     
#include <unistd.h>      
#include <dirent.h>     
#include <sys/stat.h> 

#include "tree.h"

// --- Datos para los Hilos ---
typedef struct {
    int thread_id;
    char** file_list;
    char** temp_file_list;
    int total_files;
    pthread_mutex_t* mutex;
    int* next_file_index;
} thread_data_t;

// --- Esto hace cada thread ---
/**
 * @brief Función que ejecuta cada hilo para comprimir archivos.
 */
void* compress_worker(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;

    while (true) {
        int current_file_index;

        // Sección crítica: obtener el índice del próximo archivo a procesar
        pthread_mutex_lock(data->mutex);
        current_file_index = *(data->next_file_index);
        if (current_file_index >= data->total_files) {
            pthread_mutex_unlock(data->mutex);
            break; // No hay más archivos, el hilo termina
        }
        (*(data->next_file_index))++;
        pthread_mutex_unlock(data->mutex);

        // Obtener las rutas de los archivos
        const char* input_path = data->file_list[current_file_index];
        const char* temp_output_path = data->temp_file_list[current_file_index];

        printf("[Hilo %d] Comprimiendo: %s -> %s\n", data->thread_id, input_path, temp_output_path);

        // Llamar a la función de compresión segura para hilos
        compressFile(input_path, temp_output_path);
    }

    return NULL;
}


// --- Función Principal de Compresión con Pthreads ---

/**
 * @brief Comprime un directorio usando un pool de hilos (pthreads).
 * @param inputDir Directorio de entrada.
 * @param outputFile Archivo binario de salida.
 * @return true si la compresión fue exitosa, false en caso contrario.
 */
bool compressDirectoryPthread(const char* inputDir, const char* outputFile) {
    // 1. Obtener el número de núcleos de CPU para definir el número de hilos
    long num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) {
        num_threads = 2; // Valor por defecto si falla la detección
    }
    printf("Iniciando compresión con %ld hilos...\n", num_threads);

    // 2. Escanear directorio y listar archivos a comprimir
    DIR* dir = opendir(inputDir);
    if (dir == NULL) {
        perror("Error al abrir directorio");
        return false;
    }

    int file_count = 0;
    struct dirent* entry;
    // Contar archivos primero para asignar memoria
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Si es un archivo regular
            file_count++;
        }
    }
    rewinddir(dir);

    if (file_count == 0) {
        printf("No se encontraron archivos en el directorio.\n");
        closedir(dir);
        return false;
    }

    // Asignar memoria para las listas de archivos
    char** file_list = malloc(file_count * sizeof(char*));
    char** temp_file_list = malloc(file_count * sizeof(char*));
    char** original_filenames = malloc(file_count * sizeof(char*));

    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < file_count) {
        if (entry->d_type == DT_REG) {
            char full_path[1024];
            char temp_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", inputDir, entry->d_name);
            snprintf(temp_path, sizeof(temp_path), "/tmp/huffman_temp_%d_%d.bin", getpid(), index);

            file_list[index] = strdup(full_path);
            temp_file_list[index] = strdup(temp_path);
            original_filenames[index] = strdup(entry->d_name);
            index++;
        }
    }
    closedir(dir);

    // 3. Inicializar hilos y estructuras de sincronización
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int next_file_index = 0;

    // 4. Lanzar el pool de hilos
    for (long i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].file_list = file_list;
        thread_data[i].temp_file_list = temp_file_list;
        thread_data[i].total_files = file_count;
        thread_data[i].mutex = &mutex;
        thread_data[i].next_file_index = &next_file_index;
        pthread_create(&threads[i], NULL, compress_worker, &thread_data[i]);
    }

    // 5. Esperar a que todos los hilos terminen
    for (long i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // 6. Combinar los archivos temporales en el archivo final
    printf("Combinando %d archivos comprimidos...\n", file_count);
    FILE* final_output = fopen(outputFile, "wb");
    fwrite(&file_count, sizeof(int), 1, final_output);

    for (int i = 0; i < file_count; i++) {
        FILE* temp_file = fopen(temp_file_list[i], "rb");
        if (temp_file) {
            // Escribir metadatos
            int name_len = strlen(original_filenames[i]);
            fwrite(&name_len, sizeof(int), 1, final_output);
            fwrite(original_filenames[i], sizeof(char), name_len, final_output);

            // Escribir tamaño y contenido del archivo comprimido
            fseek(temp_file, 0, SEEK_END);
            long long compressed_size = ftell(temp_file);
            rewind(temp_file);
            fwrite(&compressed_size, sizeof(long long), 1, final_output);

            char buffer[4096];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), temp_file)) > 0) {
                fwrite(buffer, 1, bytes_read, final_output);
            }
            fclose(temp_file);
            remove(temp_file_list[i]); // Borrar archivo temporal
        }
    }
    fclose(final_output);

    // 7. Limpieza de memoria
    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
        free(temp_file_list[i]);
        free(original_filenames[i]);
    }
    free(file_list);
    free(temp_file_list);
    free(original_filenames);
    pthread_mutex_destroy(&mutex);

    printf("¡Compresión con Pthreads completada!\n");
    return true;
}

// --- AÑADE ESTO AL FINAL DE readFile_pthread.c ---

/**
 * @brief Función que ejecuta cada hilo para descomprimir archivos.
 */
void* decompress_worker(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;

    while (true) {
        int current_file_index;

        // Sección crítica para obtener el próximo trabajo
        pthread_mutex_lock(data->mutex);
        current_file_index = *(data->next_file_index);
        if (current_file_index >= data->total_files) {
            pthread_mutex_unlock(data->mutex);
            break; // No hay más trabajo
        }
        (*(data->next_file_index))++;
        pthread_mutex_unlock(data->mutex);

        const char* temp_input_path = data->temp_file_list[current_file_index];
        const char* final_output_path = data->file_list[current_file_index];

        printf("[Hilo %d] Descomprimiendo: %s -> %s\n", data->thread_id, temp_input_path, final_output_path);

        // La función de descompresión de un solo archivo ya es segura
        decompressFile(temp_input_path, final_output_path);

        // Borrar el archivo temporal
        remove(temp_input_path);
    }
    return NULL;
}

/**
 * @brief Descomprime un directorio usando un pool de hilos (pthreads).
 * @param compressedFile Archivo .bin a descomprimir.
 * @param outputDir Directorio de salida.
 * @return true si la descompresión fue exitosa, false en caso contrario.
 */
bool decompressDirectoryPthread(const char* compressedFile, const char* outputDir) {
    long num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) num_threads = 2;
    printf("Iniciando descompresión con %ld hilos...\n", num_threads);

    FILE* input = fopen(compressedFile, "rb");
    if (!input) {
        perror("Error al abrir archivo comprimido");
        return false;
    }
    createDirectoryIfNotExists(outputDir);

    int file_count;
    fread(&file_count, sizeof(int), 1, input);

    char** temp_file_list = malloc(file_count * sizeof(char*));
    char** final_file_list = malloc(file_count * sizeof(char*));

    // 1. Extraer los datos de cada archivo a un temporal
    for (int i = 0; i < file_count; i++) {
        int name_len;
        fread(&name_len, sizeof(int), 1, input);
        char filename[256];
        fread(filename, sizeof(char), name_len, input);
        filename[name_len] = '\0';

        long long compressed_size;
        fread(&compressed_size, sizeof(long long), 1, input);
        
        char temp_path[1024], final_path[1024];
        snprintf(temp_path, sizeof(temp_path), "/tmp/huffman_decomp_temp_%d_%d.bin", getpid(), i);
        snprintf(final_path, sizeof(final_path), "%s/%s", outputDir, filename);

        temp_file_list[i] = strdup(temp_path);
        final_file_list[i] = strdup(final_path);

        FILE* temp_file = fopen(temp_path, "wb");
        char buffer[4096];
        long long bytes_remaining = compressed_size;
        while(bytes_remaining > 0){
            size_t bytes_to_read = (bytes_remaining < sizeof(buffer)) ? bytes_remaining : sizeof(buffer);
            size_t bytes_read = fread(buffer, 1, bytes_to_read, input);
            fwrite(buffer, 1, bytes_read, temp_file);
            bytes_remaining -= bytes_read;
        }
        fclose(temp_file);
    }
    fclose(input);

    // 2. Lanzar hilos para descomprimir los archivos temporales
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int next_file_index = 0;

    for (long i = 0; i < num_threads; i++) {
        thread_data[i] = (thread_data_t){
            .thread_id = i,
            .file_list = final_file_list,
            .temp_file_list = temp_file_list,
            .total_files = file_count,
            .mutex = &mutex,
            .next_file_index = &next_file_index
        };
        pthread_create(&threads[i], NULL, decompress_worker, &thread_data[i]);
    }

    // 3. Esperar y limpiar
    for (long i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < file_count; i++) {
        free(temp_file_list[i]);
        free(final_file_list[i]);
    }
    free(temp_file_list);
    free(final_file_list);
    pthread_mutex_destroy(&mutex);

    printf("¡Descompresión con Pthreads completada!\n");
    return true;
}