#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h> //Por si acaso.
#include <locale.h>
#include "tree.h"



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
