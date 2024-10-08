#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <inttypes.h>
#include "hufftools.h"

#define BITSIZE 1
#define BYTESIZE 4
#define NUMBITS 8
#define ASCIIVALS 256
#define BUFFER_SIZE 4096
#define NUMBITSsubone 7

/*pre declaration of function */

void read_body(struct Node *root, int numchars, int outfd, int infd,
					    struct LinkedList* list); 
int *build_hist(int infd);
 
int main(int argc, char *argv[]) {

	int *histogram;
	int i = 0;
	int numchars;
	unsigned char outchar;
	int freq;
	
	/* declaring input file */

	int infd = open(argv[1], O_RDONLY);

	if (infd == -1) {

		perror("Cannot open file");
		exit(1);

	}

	/* declaring output file */

	const char* outFile = argv[2];
	int outfd;
	
	if (argc == 3) {
		
		outfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	}

	else {

		outfd = STDOUT_FILENO;

	}

	histogram = build_hist(infd);
	numchars = 0;
	struct LinkedList* list = createLinkedList();

	while (i < ASCIIVALS) {

		if (histogram[i] > 0) {
			
			numchars = numchars + histogram[i];
			struct Node* node = createNode(histogram[i],
						  (unsigned char)i);
			insertsorted(list, node);
	
		}
		
		i = i + 1;

	}

	/* if there are no chars in the file */

	if (numchars == 0) {

		return 0;

	}

	/* if we only have one node in the list */

	if (list->size == 1) {

		outchar = (unsigned char)list->head->byte;
		freq = (int)list->head->freq;
		
		/* write to out file the char and freq times */

		while (freq > 0) {

			ssize_t char_written = write(outfd, &outchar, 
					      sizeof(unsigned char));

			if (char_written == -1) {

				perror("Error writing");
				exit(1);

			}		
	
			freq = freq - 1;	

		}	

	}

	/* if there is more than one char in the file */

	else {

		struct Node* root = buildHuffTree(list);		
		read_body(root, numchars,outfd, infd, list);

	}
	
		
	return 1;
	
}

int *build_hist(int infd) {
	
	int letter_index, i;
	unsigned char byteuniqchs, bytechar;
	int uniqchs, numSubOne;
	unsigned int byteint;
	int freq = 0;

	/* allocate space for all 256 ASCII chars */

	int* histogram = (int*)malloc(ASCIIVALS * sizeof(int));

	if (histogram == NULL) {
		
		perror("Insufficient Memory");
		exit(1);

	}

	/* set all freqs to 0 for ASCII values */

	for (i = 0; i < ASCIIVALS; i++) {

		histogram[i] = 0;

	}

	ssize_t uniqchsread = read(infd, &byteuniqchs, BITSIZE);

	if (uniqchsread == -1) {

		perror("Error reading file");
		exit(1);

	}
	
	numSubOne = (int)byteuniqchs;
	uniqchs = numSubOne + 1;

	/* fill our histogram */

	while (uniqchs > 0) {
		
		ssize_t charread = read(infd, &bytechar, BITSIZE);

		if (charread == -1) {

			perror("Error reading the file");
			exit(1);

		}

		letter_index = (int)bytechar;
		
		/* reset freq for all chars */

		freq = 0;

		/* add up the 4 bytes count's to get freq */

		ssize_t freqread = read(infd, &byteint, BYTESIZE);

		if (freqread == -1) {

			perror("Error reading the file");
			exit(1);

		}

		freq = ntohl(byteint);	

		/* set ASCII value in hist to the corresponding freq */
		/* build hist */

		histogram[letter_index] = freq;

		/* decrement loop */

		uniqchs = uniqchs - 1;
		
	}

	return histogram;	
	
}

void read_body(struct Node *root, int numchars, int outfd, int infd,
					 struct LinkedList* list) {

	char bitvalues[BUFFER_SIZE];
	unsigned char code[ASCIIVALS];
	int i, j;
	int codeindex = 0;
	int foundleaf = 0;

	/*grab a whole chunk of bytes */

	while ((read(infd, &code, BUFFER_SIZE)) > 0) {
			
			/* store all the binary values in array */

			for (j = 0; j < ASCIIVALS; j++){ 

				/* manipulate each byte 
 				to get binary values */

				for (i = NUMBITSsubone; i >= 0; i--) {

					/* shift '1' left, 
  					and with certain bit */

					bitvalues[codeindex] = 
					(code[j] & (1 << i)) ? '1' : '0';

					/* increment index in 
  					our code array */

					codeindex = codeindex + 1;

				}
			}

	}

	/* reset code index to use when traversing tree */

	codeindex = 0;

	/* keep going until we have found all chars */

	while (foundleaf < numchars) {

		if (isLeaf(root)) {

		/*	write char to outfile */

			if (write(outfd, &root->byte, 1) == -1) {

				perror("Error Writing to out file");
				exit(1);

			} 

			root = list->head;
			foundleaf = foundleaf + 1;

		}

		/* go right if we have a '1' */
		else if (bitvalues[codeindex] == '1') {

			root = root->right;
			codeindex = codeindex + 1;

		}

		/* go left if we have a '0' */
		else if (bitvalues[codeindex] == '0') {

			root = root->left;
			codeindex = codeindex + 1;
	
		}	
	
	}

	return;	

} 
