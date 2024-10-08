#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include "hufftools.h"

/* make pre declarations of functions so main recognizes them */

void decimalToHex(char lookuptable[][256]);


int main(int argc, char *argv[]) {

	/* initializing variables and structures */
	int *histogram;
	int i;
	int j;
	char lookuptable[256][256];
	char code[256];
	int traversed;
	int uniqchars = 0;

	int inputFile = open(argv[1], O_RDONLY);
	if (inputFile == -1) {
		perror("Cannot open file");
		exit(1);
	}

	traversed = 0;	
	i = 0;
	j = 0;
	
	histogram = (int* )construct_hist(inputFile);
	close(inputFile);
	
	struct LinkedList* list = createLinkedList();

	/* insert Nodes that have at least Freq 1 into LinkedList sorted */
	while (i < 256) {

		if (histogram[i] > 0) {
		
			uniqchars = uniqchars + 1;
			struct Node* node = createNode(histogram[i],
			 (unsigned char)i);
			insertsorted(list, node);	
		}

		i = i + 1;
	}
	/* if there is only 1 char */	
	if (list->size == 1)  {
		
		printf("0x%02x: \n", (int)list->head->byte);
		
	}

	else {
	
		struct Node* root = buildHuffTree(list);
		/*clear lookuptable and array for storing the code*/
		clearcodetable(code);
		clearlookuptable(lookuptable);
		/* traverses tree, stores code in lookuptable */
		DFSAndBuildCode(root, lookuptable, traversed, code);
		decimalToHex(lookuptable);

		int inputfile = open(argv[1], O_RDONLY);
		if (inputfile == -1) {
			perror("Error Opening File");
			exit(1);
		}	

		/* temp structs to free all nodes in list */
		struct Node* temp;
		struct Node* temp2;
		temp2 = list->head;

		while (j < list->size) {

			temp = temp2->next;
			free(temp2);
			temp2 = temp;
			j = j + 1;	
		}
	}


	free(list);
	free(histogram);

        return 0;

        }

void decimalToHex(char lookuptable[][256]) {
	int i;
	i = 0;

	while (i < 256) {
		
		/* %x turns to hex */
		if (lookuptable[i][0] != '\0') { 
			printf("0x%02x: %s\n", i, lookuptable[i]);

		} 

		i = i + 1;
	}
}

