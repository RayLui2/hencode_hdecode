#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <inttypes.h>
#include "hufftools.h"

#define BYTESIZE 1
#define ASCII_VALS 256
#define MAXBITS 8

/* make pre declarations of functions so main recognizes them */

void compress(int inputFile, char lookuptable[][ASCII_VALS], int outputFile,
	      unsigned char byte);
void createNumsubOne(int uniqchars, int outfd); 

int main(int argc, char *argv[]) {

	/* initializing variables and structures */

	int *histogram;
	int i;
	int j;
	char lookuptable[ASCII_VALS][ASCII_VALS];
	char code[ASCII_VALS];
	int traversed;
	unsigned char byte = 0;
	int uniqchars = 0;
	uint32_t count;
	uint32_t hostcount;

	/* test to see if we can open input file */

	int inputFile = open(argv[1], O_RDONLY);
	if (inputFile == -1) {

		perror("Cannot open file");
		exit(1);

	}

	/* set outputfile */

	const char* outFile = argv[2];
	int outfd;

	/* if outfile provided, print to it, if not, print to STDOUT */

	if (argc == 3)  {
	
			outfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC,
			 					   0644);

		}

		else {

			outfd = STDOUT_FILENO;
		}

		if (outfd == -1) {

			perror("Error opening out file");
			exit(1);

		}


	traversed = 0;	
	i = 0;
	j = 0;
	
	/* build histo */

	histogram = (int* )construct_hist(inputFile);

	close(inputFile);
	
	struct LinkedList* list = createLinkedList();

	/* insert Nodes that have at least Freq 1 into LinkedList sorted */

	while (i < ASCII_VALS) {

		if (histogram[i] > 0) {

			uniqchars = uniqchars + 1;
			struct Node* node = createNode(histogram[i],
			 (unsigned char)i);
			insertsorted(list, node);
	
		}

		i = i + 1;
	}

	/*return nothing if empty file */

	if (list->head == NULL) {

		return 0;

	}

	/* if there is only 1 char */	

	else if (list->size == 1)  {
		
		/* create the beginning of header */

		createNumsubOne(uniqchars, outfd);
		ssize_t byte_written = write(outfd, &list->head->byte,
					       sizeof(unsigned char));

		if (byte_written != sizeof(unsigned char)) {

			perror("Error writing to the out file");
			close(outfd);
			exit(1);

		}

		/* write the count to output */

		count = (uint32_t)list->head->freq;
		hostcount = ntohl(count);		
		ssize_t count_written = write(outfd, &hostcount,
					      sizeof(uint32_t));

		if (count_written != sizeof(uint32_t)) {

			perror("Error writing to the out file");
			close(outfd);
			exit(1);

		}


	}

	else {
	
		struct Node* root = buildHuffTree(list);

		/*clear lookuptable and array for storing the code*/

		clearcodetable(code);
		clearlookuptable(lookuptable);

		/* traverses tree, stores code in lookuptable */

		DFSAndBuildCode(root, lookuptable, traversed, code);
	

		int inputfile = open(argv[1], O_RDONLY);

		if (inputfile == -1) {

			perror("Error Opening File");
			exit(1);

		}
				
		/* writes num - 1 to output file */

		createNumsubOne(uniqchars, outfd);

	
		i = 0;
		/* writes char and freq to output file */

		while (i < ASCII_VALS) {

			if (histogram[i] > 0) {

				/* bit mask so we only have last byte */

				byte = (unsigned char)(i & 0xFF);

				/* write the char ascii value to out */

				ssize_t byte_written = write(outfd, &byte,
						    sizeof(unsigned char));
				
				if (byte_written != sizeof(unsigned char)) {
					perror("Error writing to the out");
					close(outfd);
					exit(1);
				}
				
				/* write the freq as a 32 bit uint to out */

				count = (uint32_t)histogram[i];
				hostcount = ntohl(count);
				
				ssize_t count_written = write(outfd,
					 &hostcount, sizeof(uint32_t));
				
				if (count_written != sizeof(uint32_t)) {

					perror("Error writing to the out");
					close(outfd);
					exit(1);

				}
			
				/* reset the byte */

				byte = 0;
			}

			i = i + 1;
		}

		/* compress file, write to outfd */

		compress(inputfile, lookuptable, outfd, byte);

		close(inputfile);
		close(outfd);

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

/* this function first writes the header to out file, 
    then bit shifts and writes the body to the output file */

void compress(int inputFile, char lookuptable[][ASCII_VALS], int outfd, 
						unsigned char byte) {

	unsigned char character;
	int bitcount = 0;
	int letter_index;
	int i;
	int shifts;
	int j = 0;

	/* move cursor to beginning of input file */

	lseek(inputFile, 0, SEEK_SET);	

	/* until we read all chars from file */

	while (read(inputFile, &character, BYTESIZE) > 0) {

		i = 0;
		letter_index = (int)character;

		/* until we have read the full code */

		while (lookuptable[letter_index][i] != '\0') {
			
			/* if we have shifted 8 bits */

			if (bitcount == MAXBITS) {

				/* write to output */

				ssize_t byte_written = write(outfd, &byte,
						   sizeof(unsigned char));
				
				if (byte_written != sizeof(unsigned char)) {

					perror("Error writing to the file");
					close(outfd);
					exit(1);
				}

				/* reset byte so we can shift 0's again */

				byte = 0;
				bitcount = 0;
			}

			/* if code is a 1, shift bit 1 left */

			if (lookuptable[letter_index][i] == '1') {

				byte = byte << 1;
				byte |= 1;
				bitcount = bitcount + 1;
			
			}

			/* shift bit 0 to the left */

			else {

				byte = byte << 1;
				bitcount = bitcount + 1;
			
			}

			i = i + 1;
		}
	}
	
	shifts = MAXBITS - bitcount;

	/* pad 0's to end of byte */

	while (j < shifts) {

		byte = byte << 1;
		j = j + 1;

	}
	
	/* write final byte to the output file */

	ssize_t byte_written = write(outfd, &byte, sizeof(unsigned char));
				
	if (byte_written != sizeof(unsigned char)) {

		perror("Error writing to the file");
		close(outfd);
		exit(1);

	}

	return;
		
}

void createNumsubOne(int uniqchars, int outfd) {

	unsigned char byte;
	byte = 0;

	/* creating num -1 */

	uniqchars = uniqchars - 1;
	byte = (unsigned char)(uniqchars & 0xFF);

	/* write byte to the output file */

	ssize_t byte_written = write(outfd, &byte, sizeof(unsigned char));
				
	if (byte_written != sizeof(unsigned char)) {

		perror("Error writing to the file");
		close(outfd);
		exit(1);

	}	
	
	return;	
}
