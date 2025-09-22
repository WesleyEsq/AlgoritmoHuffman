#ifndef TREE_H
#define TREE_H

//Primer struct: La letra
struct letter {
    unsigned char letter;//El simbolo
    long long unsigned int frequency;// Por si acaso.
};


// Nodo para el arbol de huffman
struct treeNode{
    unsigned char value;
    unsigned long long int frequency;
    struct treeNode *left;
    struct treeNode *right;
};

// Nodos para lista de prioridad
struct NodeList {
    struct treeNode* node;
    struct NodeList* next;
};


// Para tree
struct treeNode* createNode(unsigned char value, unsigned long long int frequency);
void printTree(struct treeNode* root, int level);
struct treeNode* buildHuffmanTree(struct letter* letters, int size);
void generateCodes(struct treeNode* root, char* path, int depth, char** huffman_codes);
void freeTree(struct treeNode* root);


// Para lo de archivos
void liberarCodigos(char** codes);
void generateCodes(struct treeNode* root, char* path, int depth, char** huffman_codes);
char** reconstruirCodigos(const char* compressedFileName);
bool obtenerTablaDeFrecuencias(const char* fileName, unsigned long long* frequencies);
long long obtenerCantidadDeCaracteres(const char* fileName);
void compressFile(const char *inputFileName, const char* outputFileName);
struct letter* terribleSort();
bool decompressFile(const char* compressedFileName, const char* outputFileName);
struct treeNode* createDecodingTree(char** huffman_codes);
bool compressDirectory(const char* inputDir, const char* outputFile);
void listFilesToCompress(const char* inputDir);
bool isRegularFile(const char* filepath);

#endif // TREE_H
