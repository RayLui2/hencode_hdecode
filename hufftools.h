#ifndef HUFFTOOLS_H
#define HUFFTOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define ASCIIVALS 256

/* Node Structure for Huffman Tree */
struct Node {
	int freq;
	unsigned char byte;
	struct Node* next;
	struct Node* left;
	struct Node* right;
};

/* LinkedList Structure for Ordered Freq List */
struct LinkedList {
	struct Node* head;
	int size;
};

int *construct_hist(int inputFile); 

struct Node* createNode(int freq, unsigned char byte);

struct Node* compareNode(struct Node *newnode, struct Node *oldnode); 

struct LinkedList* createLinkedList(); 

void insertsorted(struct LinkedList *list, struct Node *node);

struct Node* poplist(struct LinkedList *list);

struct Node* buildHuffTree(struct LinkedList *list); 

void clearcodetable(char code[ASCIIVALS]);

void clearlookuptable(char lookuptable[][ASCIIVALS]); 

bool isLeaf(struct Node *node);

void DFSAndBuildCode(struct Node *root, char lookuptable[][ASCIIVALS],
	             int traversed, char code[ASCIIVALS]); 

#endif
