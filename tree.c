#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tree.h"

/*------------------------------------------------------

> Functions to work with the tree can go here.

------------------------------------------------------*/

struct treeNode* createNode(unsigned char value, unsigned long long int frequency) {
    struct treeNode* newNode = (struct treeNode*)malloc(sizeof(struct treeNode));
    if (!newNode) {
        perror("Failed to allocate memory for tree node");
        exit(EXIT_FAILURE);
    }
    newNode->value = value;
    newNode->frequency = frequency;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}


//Usar esto con el arbol
void printTree(struct treeNode* root, int level) {
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < level; i++) {
        printf("   |");
    }
    
    if (root->left == NULL && root->right == NULL) {
        printf("---[Value: '%c', Freq: %llu]\n", root->value, root->frequency);
    } else {
        printf("---[Internal Node, Freq: %llu]\n", root->frequency);
    }
    printTree(root->left, level + 1);
    printTree(root->right, level + 1);
}


// Function to insert a node into a sorted linked list
void insertSorted(struct NodeList** head, struct treeNode* newNode) {
    struct NodeList* newListItem = (struct NodeList*)malloc(sizeof(struct NodeList));
    if (!newListItem) {
        perror("Failed to allocate memory for list item");
        exit(EXIT_FAILURE);
    }
    newListItem->node = newNode;
    
    if (*head == NULL || (*head)->node->frequency >= newNode->frequency) {
        newListItem->next = *head;
        *head = newListItem;
    } else {
        struct NodeList* current = *head;
        while (current->next != NULL && current->next->node->frequency < newNode->frequency) {
            current = current->next;
        }
        newListItem->next = current->next;
        current->next = newListItem;
    }
}

// Function to get the node with the minimum frequency from the list
struct treeNode* getMin(struct NodeList** head) {
    if (*head == NULL) {
        return NULL;
    }
    struct NodeList* temp = *head;
    struct treeNode* minNode = temp->node;
    *head = (*head)->next;
    free(temp);
    return minNode;
}

struct treeNode* buildHuffmanTree(struct letter* letters, int size) {
    struct NodeList* priorityQueue = NULL;
    // Convertir a las letras en nodos para el arbol
    for (int i = 0; i < size; i++) {
        struct treeNode* newNode = createNode(letters[i].letter, letters[i].frequency);
        insertSorted(&priorityQueue, newNode);
    }

    // 2. Loop until only one node remains in the queue
    while (priorityQueue != NULL && priorityQueue->next != NULL) {
        // 3. Extract the two nodes with the minimum frequency
        struct treeNode* left = getMin(&priorityQueue);
        struct treeNode* right = getMin(&priorityQueue);

        // 4. Create a new internal node with these two nodes as children
        //    The frequency is the sum of the children's frequencies.
        unsigned long long int sum_freq = left->frequency + right->frequency;
        struct treeNode* parentNode = createNode('$', sum_freq);//$ as olaceholder for internal nodes
        
        parentNode->left = left;
        parentNode->right = right;

        //back to the priority queue
        insertSorted(&priorityQueue, parentNode);
    }

    // 6. The remaining node is the root of the Huffman tree
    return getMin(&priorityQueue);
}

// This function recursively traverses the tree to generate codes.
void generateCodes(struct treeNode* root, char* path, int depth, char** huffman_codes) {
    if (root == NULL) {
        return;
    }

    // If it's a leaf node, it contains a character.
    if (root->left == NULL && root->right == NULL) {
        path[depth] = '\0'; // Null-terminate the code string
        huffman_codes[root->value] = strdup(path); // Save a copy of the code
        return;
    }

    // Go left, append '0'
    path[depth] = '0';
    generateCodes(root->left, path, depth + 1, huffman_codes);

    // Go right, append '1'
    path[depth] = '1';
    generateCodes(root->right, path, depth + 1, huffman_codes);
}

//Para liberar un arbol
void freeTree(struct treeNode* root) {
    if (root == NULL) {
        return;
    }
    // Usa un recorrido post-orden para liberar: primero hijos, luego el padre.
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}
