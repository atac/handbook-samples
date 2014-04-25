
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "irig106ch10.h"
#include "stat_args.c"

int pause(int status){
	printf("\n\nPress ENTER to exit");
	getchar();
	return status;
}

void print_results(char filename[], int byte_size, int packets){

	printf("--------------------------------------------------------------------------------");
	printf("Summary for %s:\n", filename);

	// Make size more readable.
	char *unit = "b";
	if (byte_size > 1024){
		byte_size /= 1024;
		unit = "kb";
	}
	if (byte_size > 1024){
		byte_size /= 1024;
		unit = "mb";
	}
	if (byte_size > 1024){
		byte_size /= 1024;
		unit = "gb";
	}

	printf("    Size: %d%s\n", byte_size, unit);
	printf("    Packets: %d\n", packets);
	printf("    Channels:\n");

	// Wait for explicit exit.
	pause(0);
}

int main(int argc, char ** argv){

	// Parse args and declare variables.
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;
	int packets = 0;
	int byte_size = 0;

	// Open file for reading.  
	char filename[] = "C:/Users/mcferrill/ATAC/Data/test2.c10";
	EnI106Status status = enI106Ch10Open(&input_handle, filename, I106_READ);
	if (status != I106_OK){
		printf("Error opening file %s", args.file);
		return 1;
	}

	// Parse loop.
	while (1){
		status = enI106Ch10ReadNextHeader(input_handle, &header);
		if (status != I106_OK){
			print_results(filename, byte_size, packets);
			return 0;
		}

		byte_size += header.ulPacketLen;
		packets++;
	}

}