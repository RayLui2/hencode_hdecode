#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include "hufftools.h"

#define ASCIIVALS 256

int *construct_hist(int inputFile) {

	int letter_index, i;
	unsigned char character;
	int* histogram = (int*)malloc(ASCIIVALS * sizeof(int));
	
	/* set all frequencies to 0 in the beginning */

	for (i = 0; i < ASCIIVALS; i++){

		histogram[i] = 0;

	}

	/* check if we have enough memory */

	if (histogram == NULL) {

		perror("INSUFFICIENT MEMORY");
		exit(1);

	}
	
	/* get characters and set frequencies in array */

	while (read(inputFile, &character, 1) > 0) {

		letter_index = (int)character;
		histogram[letter_index] += 1;

	}

	return histogram;
}

struct Node* createNode(int freq, unsigned char byte) {

	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	
	/* check to see if we can create node */

	if (newNode == NULL) {

		perror("Memory Allocation Failed");
		exit(1);

	}

	newNode->freq = freq;
	newNode->byte = byte;
	newNode->next = NULL;
	newNode->left = NULL;
	newNode->right = NULL;

	return newNode;
}

struct Node* compareNode(struct Node *newnode, struct Node *oldnode) {

	/* check if both nodes are valid nodes */

	if ((newnode == NULL) || (oldnode == NULL)) {

		perror("Node is pointed to NULL");
		exit(1);

	}	

	/* return oldnode if freq is less than newnode freq */

	if (newnode->freq > oldnode->freq) {

		return oldnode;

	}

	/* return newnode if freq is less than oldnode freq*/

	else if (newnode->freq < oldnode->freq) {

		return newnode;
	}

	/* if freq's are equal*/

	else {

		if (newnode->byte == '\0' || oldnode->byte == '\0') {

			return newnode;

		}

		/* compare char ASCII values */

		else if ((int)newnode->byte > (int)oldnode->byte) {

			return oldnode;

		}	
	
		else {

			/* if newnode does not have a byte and has same freq,
 *  			 return newnode make sure new node is always the node 
 *  			  			 with no byte */

			return newnode;
		}
	}
}

struct LinkedList* createLinkedList() {

	struct LinkedList* list;
	list = (struct LinkedList*)malloc(sizeof(struct LinkedList));
	
	/* check to see if we can create LinkedList */

	if (list == NULL) {

		perror("Memory Allocation Failed");
		exit(1);

	}

	list->head = NULL;
	list->size = 0;

	return list;
}

void insertsorted(struct LinkedList *list, struct Node *node) {

		/* if empty list or node's freq is less than head's freq */

		if ((list->head == NULL) || 
		    (compareNode(node, list->head) == node)) {

			node->next = list->head;
			list->head = node;
			list->size ++;
		}
		
		else {

			struct Node* currentnode = list->head;
			/* loop until we are at end of list
 *  			or node we insert is less than current node's next */

			while ((currentnode->next != NULL) && 
			      (compareNode(node, currentnode->next)
			        == currentnode->next)) {

				currentnode = currentnode->next;

			}
			/* end of list */

			if (currentnode->next == NULL) {

				currentnode->next = node;
				node->next = NULL;
				list->size ++;

			}

			/* node is less than current node's next */
			else {

				node->next = currentnode->next;

				/* place node in between current
 *  				 and current's next */

				currentnode->next = node;
				list->size ++;	
			
			}
		}
}

struct Node* poplist(struct LinkedList *list) {

	/* can't pop empty list */

	if (list->head == NULL) {

		perror("List is empty\n");
		exit(1);

	}

	/* pop first node off the list */

	struct Node* popnode = list->head;
	list->head = list->head->next;
	popnode->next = NULL;
	list->size --;

	return popnode;
}

struct Node* buildHuffTree(struct LinkedList *list) {

	/* if there are no Nodes in the list */	

	if (list->head == NULL) {

		perror("No Nodes in the list");
		exit(1);

	}

	/* if there is only one Node, it is the parent */

	if (list->size == 1) {

		return list->head;
	}

	/* keep looping until we pop the last 2 nodes in the list */
	
	while (list->size > 1) {
		
		struct Node* node1 = poplist(list);
		struct Node* node2 = poplist(list);
		/* build parent node for popped nodes */
		/* frequency is node's freq's combined, set byte to null */
		struct Node* newNode;
		newNode = createNode((node1->freq + node2->freq), '\0');
		/* set the left node to first popped node */
		newNode->left = node1;
		/* set the right node to second popped node */
		newNode->right = node2;
		/* insert new parent node back into sorted list */
		insertsorted(list, newNode);

	}

	/* this is the parent node */
	return list->head;
}

void clearcodetable(char code[ASCIIVALS]) {

	int i;
	i = 0;
	
	while (i < ASCIIVALS) {

		code[i] = '\0';
		i = i + 1;

	}
}

void clearlookuptable(char lookuptable[][ASCIIVALS]) {

	int i;
	i = 0;
	/* clear our lookup table */

	while (i < ASCIIVALS) {

		lookuptable[i][0] = '\0';
		i = i + 1;

	}
}

bool isLeaf(struct Node *node) {

	return (node->left == NULL && node->right == NULL);
}

void DFSAndBuildCode(struct Node *root, char lookuptable[][ASCIIVALS],
	             int traversed, char code[ASCIIVALS]) {

	int i;
	int letterindex;
	
	/* return if not a valid node */
	if (root == NULL) {

		return;

	}



	/* if we found a leaf, add path to our lookuptable */
	if (isLeaf(root)) {

		letterindex = (int)root->byte;

		for (i = 0; i < traversed; i++) {

			lookuptable[letterindex][i] = code[i];

		}

		lookuptable[letterindex][i] = '\0';
	}

	/* if there is a left node, add 0 to path */

	if (root->left) {

		code[traversed] = '0';
		/* call recursive func on left node */

		DFSAndBuildCode(root->left, lookuptable, traversed + 1, code);

	}
	
	/* if there is a right node, add 1 to path */

	if (root->right) {

		code[traversed] = '1';
		/* call recursice func on right node */

		DFSAndBuildCode(root->right, lookuptable, traversed + 1, code);

	}

	return;
}

