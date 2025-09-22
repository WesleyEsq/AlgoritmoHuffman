# Algoritmo de Huffman - Compresión y Descompresión

---

## Documentación tecnica:

---

### 0- Como compilarlo

gcc main.c readFile.c tree.c -o main

### 1- Archivos

Existen 4 archivos importantes, estos serían:

- **tree.c**
  - Contiene funciones para hacer funcionar el arbol del algoritmo de huffman
  - También tiene un min heap para organizar los nodos para comprimirlo
  - Funciones principales: `buildHuffmanTree()`, `generateCodes()`, `createNode()`, `freeTree()`
- **readFile.c**
  - Contiene funciones para leer y comprimir y descomprimir archivos
  - Manejo de archivos individuales y directorios completos
  - Funciones principales: `compressFile()`, `decompressFile()`, `compressDirectory()`, `decompressDirectory()`
- **tree.h**
  - Header para los dos archivos de arriba.
  - Contiene las definiciones de structs usados en el algoritmo
  - Declaraciones de todas las funciones públicas
- **main.c**
  - Aquí iría la ejecución del algoritmo, hacer un programa sencillo de consola
  - Incluye pruebas para archivos individuales y directorios completos

### 2- Funciones del algoritmo de compresión

#### 2.1 Compresión de archivos individuales

- **`compressFile(inputFile, outputFile)`:** Comprime un archivo individual usando el algoritmo de Huffman. La compresión incluye:

  - Primero: la cantidad total de caracteres del archivo original (para saber cuándo parar en la descompresión)
  - Segundo: la tabla de frecuencias de los caracteres del archivo original (para reconstruir los códigos)
  - Tercero: los datos comprimidos bit por bit

- **`decompressFile(compressedFile, outputFile)`:** Descomprime un archivo que fue comprimido con `compressFile()`. El proceso:
  - Lee los metadatos (cantidad de caracteres y frecuencias)
  - Reconstruye el árbol de Huffman
  - Decodifica bit por bit navegando por el árbol
  - Genera el archivo original

#### 2.2 Compresión de directorios completos

- **`compressDirectory(inputDir, outputFile)`:** Comprime todos los archivos de texto de un directorio en un solo archivo binario:

  - Escanea el directorio buscando archivos regulares
  - Comprime cada archivo individualmente
  - Almacena metadatos de cada archivo (nombre, tamaño comprimido)
  - Genera un archivo binario con todo el contenido

- **`decompressDirectory(compressedFile, outputDir)`:** Descomprime un directorio completo:
  - Lee los metadatos del archivo comprimido
  - Crea el directorio de salida si no existe
  - Extrae y descomprime cada archivo individual
  - Recrea la estructura original del directorio

#### 2.3 Funciones auxiliares importantes

- **`obtenerCantidadDeCaracteres(fileName)`:** Obtiene el número total de caracteres del encabezado
- **`obtenerTablaDeFrecuencias(fileName, frequencies)`:** Obtiene la tabla de frecuencias del encabezado
- **`reconstruirCodigos(compressedFile)`:** Genera los códigos de Huffman a partir de un archivo comprimido
- **`terribleSort()`:** Algoritmo de ordenamiento para organizar caracteres por frecuencia
- **`createDecodingTree(huffman_codes)`:** Crea el árbol de decodificación para la descompresión
- **`liberarCodigos(codes)`:** Libera la memoria de la tabla de códigos

### 3- ¿Como uso los codigos?

#### 3.1 Formato de los códigos

Los códigos se almacenan en una variable de tipo:

```c
char** reconstructed_codes = reconstruirCodigos(compressed_file);
```

#### 3.2 Acceder al código de un carácter

```c
// Para acceder al código de un carácter específico
char* getCodeOfACharacter(char** huffmanCodes, unsigned char symbol){
    return (huffmanCodes[symbol]);
}
```

#### 3.3 Explicación del sistema de indexación

Los códigos funcionan como un diccionario/tabla de hash:

- Hay 256 caracteres posibles en ASCII (0-255)
- El array `reconstructed_codes` tiene exactamente 256 elementos
- Cada posición corresponde al código ASCII del carácter: https://www.ascii-code.com/
- Para acceder al código de un carácter, usas el carácter directamente como índice

**Ejemplo:**

```c
char c = 'A';  // ASCII 65
char* codigo = reconstructed_codes[c];  // Obtiene el código de 'A'
// O equivalentemente:
char* codigo = reconstructed_codes[65];
```

### 4- Estructura de archivos comprimidos

#### 4.1 Archivo individual comprimido

```
[long long: total de caracteres]
[unsigned long long[256]: tabla de frecuencias]
[bytes: datos comprimidos bit por bit]
```

#### 4.2 Directorio comprimido

```
[int: número de archivos]
[Para cada archivo:]
  - [int: longitud del nombre del archivo]
  - [char[]: nombre del archivo]
  - [long long: tamaño del archivo comprimido]
  - [bytes: datos del archivo comprimido (formato individual)]
```

### 5- Uso del programa

#### 5.1 Compilación

```bash
gcc main.c readFile.c tree.c -o main
```

#### 5.2 Ejecución

El programa actual ejecuta automáticamente las siguientes pruebas:

1. **Prueba de archivo individual:**

   - Comprime `example.txt` → `example.bin`
   - Muestra los códigos de Huffman generados
   - Descomprime `example.bin` → `example_decompressed.txt`

2. **Prueba de directorio:**
   - Lista archivos en `./test_files/`
   - Comprime el directorio → `directorio_comprimido.bin`
   - Muestra contenido del archivo comprimido
   - Descomprime → `./test_files_extracted/`

### 6- Funciones de debugging y utilidades

- **`listFilesToCompress(inputDir)`:** Lista todos los archivos que serán comprimidos en un directorio
- **`listCompressedDirectoryContents(compressedFile)`:** Muestra el contenido de un archivo de directorio comprimido
- **`printFrequencies()`:** Muestra las frecuencias de caracteres calculadas
- **`printTree(root, level)`:** Imprime la estructura del árbol de Huffman (para debugging)

### 7- Consideraciones técnicas

#### 7.1 Manejo de memoria

- Todas las funciones manejan adecuadamente la liberación de memoria
- Usar `liberarCodigos()` después de usar tablas de códigos
- Usar `freeTree()` para liberar árboles de Huffman
