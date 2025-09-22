#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h> //Por si acaso.
#include <locale.h>
#include "tree.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>


//--------------------------------------------------------------------------//
//                                                                          //
//                      Funciones auxiliares pequeñas:                      //
//                                                                          //
//--------------------------------------------------------------------------//

//Arreglos sencillos globales
struct letter alphabet [256];
struct letter *alphabetresults;

//Función para inicializar el alfabeto. Primera cosa que hacer antes que nada.
void initGlobalAlphabet(){
    for(int i = 0; i < 256; i++){
        alphabet[i].frequency = 0;
        alphabet[i].letter = i;
    }
}

struct letter* initAlphabet(struct letter alphabet[256] ){
    for(int i = 0; i < 256; i++){
        alphabet[i].frequency = 0;
        alphabet[i].letter = i;
    }
    return alphabet;
}

//Busca la dirección de la letra y le suma 1
void addLetterFrequency(const unsigned char letter){
    int index = (int) letter;
    alphabet[index].frequency += 1;
}

/*
    Terrible sort
    --------------------------------------------------------
    1- Count the amount of values in the alphabet array that aren't 
    set to 0, that's the required space for the result array.
    
    Then in a while true:

    2- Get the index of the most frequent value in the array

    3- Place it at the first possible position in the result array

    4- set the value of the index in the first array to 0

    5- Repeat steps 2-3-4 until all values in the first array are 0
        Then break.

    -------------------------------------------------------
    You may say it's awful but it's linear, kinda

*/
struct letter* terribleSort()
{
    struct letter* resultList; //Paso 1
    int amount = 0;
    for(int i = 0; i<256; i++){
        if(alphabet[i].frequency != 0)
            amount ++;
    }
    resultList = (struct letter*)malloc(amount * sizeof(struct letter));
    int resultArrayLeftIndex = 0;
    

    while(true){
        int counter = 0;
        struct letter biggestYet;
        biggestYet.frequency = 0;
        biggestYet.letter = 0;
        
        //
        for(int i = 0; i < 256; i++){
            long long unsigned int currentFreq = alphabet[i].frequency;
            if(currentFreq > biggestYet.frequency)
            {
                biggestYet.frequency = currentFreq;
                biggestYet.letter = alphabet[i].letter;
            }
        }
        if(biggestYet.frequency == 0) break;

        alphabet[biggestYet.letter].frequency = 0;

        resultList[resultArrayLeftIndex] = biggestYet;
        resultArrayLeftIndex ++;

        //if(biggestYet.frequency == 0) break; Se manda el malloc usar esto
        if(resultArrayLeftIndex > 255) break;
    }

    return resultList;
}


//Leer un archivo, para comenzar la compresión.
void readTextFile(const char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    int charAsciiValue; 
    if (file == NULL) { perror("Error, file not found\n"); return; }

    while((charAsciiValue = fgetc(file)) != EOF) {
        unsigned char character = (unsigned char)charAsciiValue;
        addLetterFrequency(character);
    }

    fclose(file);
}


//Resetear los array globales, lo dejo por si es necesario después
void resetGlobalVariables(){
    free(alphabetresults);
    initGlobalAlphabet();
}


//Para comprobar que se hicieron las frecuencias bien
void printFrequencies(){
    printf("------------------------------------\n Frequency of letters: \n");
    for(int i = 0; i < 256; i++){
        char character = (char) i;
        if(alphabet[i].frequency != 0)
            printf("%c appears %d times\n", character, alphabet[i].frequency);
    }
    printf("------------------------------------\n");
}

//Obtener la cantidad de caracteres NO nulos
int countSize(){
    int amount = 0;
    for(int i = 0; i < 256; i++)
        if(alphabet[i].frequency != 0)
            amount+=1;
    return amount;
}

//... Para descomprimir, va a ser bien feo.
void readCompressedFile(const char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    int charAsciiValue;
    
    fclose(file);
}





//--------------------------------------------------------------------------//
//                                                                          //
//      Ahora sí, abajo las funciones que realmente importan.               //
//                                                                          //
//--------------------------------------------------------------------------//

/**
 * @brief Obtener los codigos para encriptar a un archivo
 * @param fileName El nombre del archivo en un string.
 * @param treeNode El arbol, apuntará al arbol que se usó si se ocupa luego.
 * @returns char** huffanCodes[256];
 *
 */
char** obtenerCodigos(const char *fileName){
    initGlobalAlphabet(); 
    readTextFile(fileName);
    //printFrequencies();

    int size = countSize();
    alphabetresults = terribleSort(); //Segundo organizar los elementos y hacer el arbol
    struct treeNode* huffmanRoot = buildHuffmanTree(alphabetresults, size);

    //Tercero escribir los códigos de cada caracter:
    char** huffman_codes = (char**)calloc(256, sizeof(char*));
    char path[256] = { 0 }; //Buffer sencillo para ir escribiendo los caracteres en el algoritmo
    generateCodes(huffmanRoot, path, 0, huffman_codes);

    return huffman_codes;
}

//Para acceder al código de un caracter de un diccionario
char* getCodeOfACharacter(char** huffmanCodes, unsigned char symbol){
    return (huffmanCodes[symbol]);
}

/**
 * @brief los parametros y el proposito de este procedimiento se explican solos.
 */
void compressFile(const char *inputFileName, const char* outputFileName)
{
    FILE *inputFile;
    FILE *outputFile;
    inputFile = fopen(inputFileName, "r");
    outputFile = fopen(outputFileName, "wb");

    //... Ok, vamos a poner algo básico para errores
    if (inputFile == NULL) {perror("Error, file not found\n"); return;}
    if (outputFile == NULL) { perror("Error, output file is wrong somehow\n"); return;}

    //TotalChars es la cantidad total de caracteres
    //El numero sirve para saber cuando parar con la descompresión.
    long long total_chars = 0;
    unsigned long long frequencies[256];
    initGlobalAlphabet();
    readTextFile(inputFileName);

    for(int i=0; i<256; i++){
        frequencies[i] = alphabet[i].frequency;
        total_chars += alphabet[i].frequency;
    }

    fwrite(&total_chars, sizeof(long long), 1, outputFile);
    fwrite(frequencies, sizeof(unsigned long long), 256, outputFile);

    //Ok, a escribir el archivo.
    char** codigos = obtenerCodigos(inputFileName);
    int charAsciiValue;//El caracter actual
    unsigned char byteBuffer;//El caracter comprimido a escribir
    int bitCount = 0;//Contar los BITS, si, BITS que se van llenando en el char

    rewind(inputFile); 
        while((charAsciiValue = fgetc(inputFile)) != EOF) {
            unsigned char character = (unsigned char)charAsciiValue;
            char* code = codigos[character]; 
            if (code == NULL) {perror("Lost a char worth of data, mission failed :(\n"); return;}

            //Un for para ir escribiendo en el buffer los bits
            for (int i = 0; code[i] != '\0'; i++) {
                byteBuffer <<= 1;
                if (code[i] == '1') {
                    byteBuffer |= 1; // Operación OR. 0010110 | 0000001 = 0010111
                }
                bitCount++;
                if (bitCount == 8) {
                    fputc(byteBuffer, outputFile);//Poner el caracter
                    //Reset values
                    bitCount = 0;
                    byteBuffer = 0;
                }
            }
        }
    //Resto
    if (bitCount > 0) {
        byteBuffer <<= (8 - bitCount);
        fputc(byteBuffer, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}


/**
 * @brief Obtiene el número total de caracteres que habían en el archivo comprimido
 * @param fileName El nombre del archivo comprimido.
 * @return El número total de caracteres, o -1 si hay un error.
 */
long long obtenerCantidadDeCaracteres(const char* fileName) {
    FILE* file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return -1; // Devuelve un valor de error
    }

    long long total_chars = 0;
    // Lee 1 elemento del tamaño de un long long
    if (fread(&total_chars, sizeof(long long), 1, file) != 1) {
        fprintf(stderr, "Error al leer el conteo de caracteres del encabezado.\n");
        fclose(file);
        return -1; // Devuelve un valor de error
    }

    fclose(file);
    return total_chars;
}


/**
 * @brief Obtiene la tabla de frecuencias del encabezado del archivo.
 * @param fileName El nombre del archivo comprimido.
 * @param frequencies Un puntero a un array de 256 'unsigned long long' donde se guardarán las frecuencias.
 * @return 'true' si la operación fue exitosa, 'false' si hubo un error.
 */
bool obtenerTablaDeFrecuencias(const char* fileName, unsigned long long* frequencies) {
    FILE* file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return false;
    }

    // SALTA los primeros bytes que corresponden al conteo de caracteres
    // fseek(archivo, desplazamiento, desde_donde);
    // SEEK_SET significa "desde el inicio del archivo".
    if (fseek(file, sizeof(long long), SEEK_SET) != 0) {
        fprintf(stderr, "Error al buscar la posición de la tabla de frecuencias.\n");
        fclose(file);
        return false;
    }

    // Lee los 256 elementos de la tabla de frecuencias
    if (fread(frequencies, sizeof(unsigned long long), 256, file) != 256) {
        fprintf(stderr, "Error al leer la tabla de frecuencias del encabezado.\n");
        fclose(file);
        return false;
    }

    fclose(file);
    return true; // Éxito
}

/**
 * @brief Esta cosa obtiene los codigos desde un archivo.
 * @param compressedFileName es el archivo comprimido
 * @returns char** huffmanCodes, añlocados y todo.
 */
char** reconstruirCodigos(const char* compressedFileName) {
    unsigned long long frequencies[256];
    if (!obtenerTablaDeFrecuencias(compressedFileName, frequencies)) {
        fprintf(stderr, "Error: No se pudo obtener la tabla de frecuencias. Abortando.\n");
        return NULL; // Devuelve NULL para indicar un fallo.
    }

    int size = 0;//Cantidad total de elementos mayores a 0
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            size++;
        }
    }

    // Asignamos memoria para ese número exacto de caracteres.
    struct letter* letters = (struct letter*)malloc(size * sizeof(struct letter));
    if (letters == NULL) {
        perror("Fallo al asignar memoria para la lista de letras");
        return NULL;
    }

    // Llenamos la lista con los caracteres y sus frecuencias.
    int currentIndex = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            letters[currentIndex].letter = (unsigned char)i;
            letters[currentIndex].frequency = frequencies[i];
            currentIndex++;
        }
    }

    // --- Pasos finales: Reconstruir el árbol de Huffman a partir de las frecuencias ---
    struct treeNode* huffmanRoot = buildHuffmanTree(letters, size);
    char** huffman_codes = (char**)calloc(256, sizeof(char*));

    if (huffman_codes == NULL) {
        perror("Fallo al asignar memoria para la tabla de códigos");
        // Limpieza antes de salir
        free(letters);
        freeTree(huffmanRoot);
        return NULL;
    }
    char path[256] = {0};
    generateCodes(huffmanRoot, path, 0, huffman_codes);

    // --- Limpiar ---
    free(letters);      
    freeTree(huffmanRoot); // Usar esta función para borrar el arbol!


    return huffman_codes;
}
/** 
 * @brief para limpiar la memoria de la tabla de codigos, importante cuando tenga que paralelizarse todo después.
*/
void liberarCodigos(char** codes){
    if (codes == NULL) { return; }

    for (int i = 0; i < 256; i++) {
        if (codes[i] != NULL) {
            free(codes[i]); // Libera la memoria del string
        }
    }
    free(codes);
}

/**
 * @brief Descomprime un archivo que fue comprimido con el algoritmo de Huffman
 * @param compressedFileName El nombre del archivo comprimido (.bin)
 * @param outputFileName El nombre del archivo de salida descomprimido
 * @return true si la descompresión fue exitosa, false en caso contrario
 */
bool decompressFile(const char* compressedFileName, const char* outputFileName) {
    FILE* compressedFile = fopen(compressedFileName, "rb");
    FILE* outputFile = fopen(outputFileName, "w");
    
    // Verificar que los archivos se abrieron correctamente
    if (compressedFile == NULL) {
        perror("Error al abrir el archivo comprimido");
        return false;
    }
    if (outputFile == NULL) {
        perror("Error al crear el archivo de salida");
        fclose(compressedFile);
        return false;
    }
    
    // 1. Leer los metadatos del archivo
    long long total_chars = obtenerCantidadDeCaracteres(compressedFileName);
    if (total_chars <= 0) {
        fprintf(stderr, "Error: Cantidad de caracteres inválida\n");
        fclose(compressedFile);
        fclose(outputFile);
        return false;
    }
    
    // 2. Reconstruir los códigos de Huffman
    char** huffman_codes = reconstruirCodigos(compressedFileName);
    if (huffman_codes == NULL) {
        fprintf(stderr, "Error: No se pudieron reconstruir los códigos\n");
        fclose(compressedFile);
        fclose(outputFile);
        return false;
    }
    
    // 3. Crear el árbol de decodificación inverso
    struct treeNode* decodingTree = createDecodingTree(huffman_codes);
    if (decodingTree == NULL) {
        fprintf(stderr, "Error: No se pudo crear el árbol de decodificación\n");
        liberarCodigos(huffman_codes);
        fclose(compressedFile);
        fclose(outputFile);
        return false;
    }
    
    // 4. Posicionarse al inicio de los datos comprimidos
    long offset = sizeof(long long) + 256 * sizeof(unsigned long long);
    fseek(compressedFile, offset, SEEK_SET);
    
    // 5. Decodificar bit por bit
    struct treeNode* currentNode = decodingTree;
    long long chars_decoded = 0;
    int byte;
    
    while (chars_decoded < total_chars && (byte = fgetc(compressedFile)) != EOF) {
        // Procesar cada bit del byte (de izquierda a derecha)
        for (int bit_pos = 7; bit_pos >= 0 && chars_decoded < total_chars; bit_pos--) {
            // Extraer el bit en la posición bit_pos
            int bit = (byte >> bit_pos) & 1;
            
            // Navegar por el árbol
            if (bit == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }
            
            // Si llegamos a una hoja, encontramos un carácter
            if (currentNode->left == NULL && currentNode->right == NULL) {
                fputc(currentNode->value, outputFile);
                chars_decoded++;
                currentNode = decodingTree; // Volver a la raíz
            }
        }
    }
    
    // 6. Limpieza
    liberarCodigos(huffman_codes);
    freeTree(decodingTree);
    fclose(compressedFile);
    fclose(outputFile);
    
    printf("Descompresión completada: %lld caracteres decodificados\n", chars_decoded);
    return true;
}

/**
 * @brief Crea un árbol de decodificación a partir de los códigos de Huffman
 * @param huffman_codes Array de códigos de Huffman
 * @return Puntero a la raíz del árbol de decodificación, o NULL si hay error
 */

struct treeNode* createDecodingTree(char** huffman_codes) {
    struct treeNode* root = createNode('$', 0); // Nodo raíz interno
    if (root == NULL) {
        return NULL;
    }
    
    // Para cada carácter que tiene código
    for (int i = 0; i < 256; i++) {
        if (huffman_codes[i] != NULL) {
            char* code = huffman_codes[i];
            struct treeNode* currentNode = root;
            
            // Navegar/crear el camino en el árbol según el código
            for (int j = 0; code[j] != '\0'; j++) {
                if (code[j] == '0') {
                    //izquierda
                    if (currentNode->left == NULL) {
                        currentNode->left = createNode('$', 0); //temporal
                    }
                    currentNode = currentNode->left;
                } else if (code[j] == '1') {
                    //derecha
                    if (currentNode->right == NULL) {
                        currentNode->right = createNode('$', 0); //temporal
                    }
                    currentNode = currentNode->right;
                } else {
                    // Código inválido
                    fprintf(stderr, "Error: Código inválido encontrado\n");
                    freeTree(root);
                    return NULL;
                }
            }
            
            // Al final del código, establecer el carácter en la hoja
            currentNode->value = (unsigned char)i;
        }
    }
    
    return root;
}

/**
 * @brief Verifica si un archivo es un archivo de texto regular
 * @param filepath Ruta completa del archivo
 * @return true si es un archivo regular, false en caso contrario
 */
bool isRegularFile(const char* filepath) {
    struct stat path_stat;
    if (stat(filepath, &path_stat) != 0) {
        return false;
    }
    return S_ISREG(path_stat.st_mode);
}

/**
 * @brief Comprime todos los archivos de texto de un directorio en un solo archivo binario
 * @param inputDir Directorio de entrada
 * @param outputFile Archivo binario de salida
 * @return true si la compresión fue exitosa, false en caso contrario
 */
bool compressDirectory(const char* inputDir, const char* outputFile) {
    DIR* dir;
    struct dirent* entry;
    FILE* output;
    
    // Abrir el directorio
    dir = opendir(inputDir);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return false;
    }
    
    // Crear el archivo de salida
    output = fopen(outputFile, "wb");
    if (output == NULL) {
        perror("Error al crear el archivo de salida");
        closedir(dir);
        return false;
    }
    
    // Primera pasada: contar archivos regulares
    int fileCount = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Saltar . y ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Construir ruta completa
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", inputDir, entry->d_name);
        
        // Verificar si es archivo regular
        if (isRegularFile(filepath)) {
            fileCount++;
        }
    }
    
    printf("Archivos encontrados: %d\n", fileCount);
    
    // Escribir el número de archivos al inicio
    fwrite(&fileCount, sizeof(int), 1, output);
    
    // Reiniciar el directorio para segunda pasada
    rewinddir(dir);
    
    // Segunda pasada: comprimir cada archivo
    int processedFiles = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Saltar . y ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Construir ruta completa
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", inputDir, entry->d_name);
        
        // Verificar si es archivo regular
        if (!isRegularFile(filepath)) {
            continue;
        }
        
        printf("Comprimiendo: %s\n", entry->d_name);
        
        // 1. Escribir longitud del nombre del archivo
        int nameLength = strlen(entry->d_name);
        fwrite(&nameLength, sizeof(int), 1, output);
        
        // 2. Escribir nombre del archivo
        fwrite(entry->d_name, sizeof(char), nameLength, output);
        
        // 3. Comprimir el archivo a un archivo temporal
        char tempCompressedFile[1024];
        snprintf(tempCompressedFile, sizeof(tempCompressedFile), "/tmp/temp_compressed_%d.bin", processedFiles);
        
        compressFile(filepath, tempCompressedFile);
        
        // 4. Leer el archivo comprimido temporal y copiarlo al archivo final
        FILE* tempFile = fopen(tempCompressedFile, "rb");
        if (tempFile == NULL) {
            fprintf(stderr, "Error al abrir archivo temporal: %s\n", tempCompressedFile);
            continue;
        }
        
        // Obtener tamaño del archivo comprimido
        fseek(tempFile, 0, SEEK_END);
        long long compressedSize = ftell(tempFile);
        fseek(tempFile, 0, SEEK_SET);
        
        // 5. Escribir tamaño del archivo comprimido
        fwrite(&compressedSize, sizeof(long long), 1, output);
        
        // 6. Copiar contenido del archivo comprimido
        char buffer[4096];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), tempFile)) > 0) {
            fwrite(buffer, 1, bytesRead, output);
        }
        
        fclose(tempFile);
        
        // Eliminar archivo temporal
        remove(tempCompressedFile);
        
        processedFiles++;
        printf("  Archivo %d/%d completado\n", processedFiles, fileCount);
    }
    
    closedir(dir);
    fclose(output);
    
    printf("\n¡Compresión de directorio completada!\n");
    printf("Archivos procesados: %d\n", processedFiles);
    printf("Archivo de salida: %s\n", outputFile);
    
    return true;
}

/**
 * @brief Lista todos los archivos que se van a comprimir (función auxiliar para debugging)
 * @param inputDir Directorio a analizar
 */
void listFilesToCompress(const char* inputDir) {
    DIR* dir;
    struct dirent* entry;
    
    dir = opendir(inputDir);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }
    
    printf("=== Archivos a comprimir en %s ===\n", inputDir);
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", inputDir, entry->d_name);
        
        if (isRegularFile(filepath)) {
            printf("  - %s\n", entry->d_name);
        }
    }
    
    closedir(dir);
    printf("===============================\n\n");
}
