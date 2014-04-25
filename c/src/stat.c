
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>

#include "config.h"
#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_time.h"


#include "stat_args.c"

int pause(int status){
	printf("\n\nPress ENTER to exit");
	getchar();
	return status;
}


int main(int argc, char ** argv){

	// Parse args and declare variables.
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	//SuI106Ch10Header header;

	// Open file for reading.  
	//*
	char filename[] = "C:/Users/mcferrill/ATAC/Data/test2.c10";
	EnI106Status status = enI106Ch10Open(&input_handle, filename, I106_READ);
	if (status != I106_OK){
		printf("Error opening file %s", args.file);
		return 1;
	}

	// Parse loop.
	//while (1){
//		status = enI106Ch10ReadNextHeader()
//	}

	printf("success!");

	// Wait for explicit exit.
	return pause(0);
	// */
}