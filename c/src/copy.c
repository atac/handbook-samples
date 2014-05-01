
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "irig106ch10.h"
#include "copy_args.c"
#include "common.h"

int main(int argc, char ** argv){
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;

	// Validate arguments and offer help.
	if (argc < 3){
		printf(args.usage_pattern);
		return quit(1);
	}

	// Open file for reading.
	EnI106Status status = enI106Ch10Open(&input_handle, argv[1], I106_READ);
	if (status != I106_OK){
		char msg[200] = "Error opening file ";
		strcat(msg, argv[1]);
		return error(msg);
	}

	// Parse loop.
	while (1){

		// Read next header or exit.
		status = enI106Ch10ReadNextHeader(input_handle, &header);
		if (status != I106_OK){
			printf("Finished");
			return quit(0);
		}

		// Ensure that we're interested in this particular packet.
		if (args.exclude && match(header.uChID, args.exclude)){
			continue;
		}
		else if (args.channel && !match(header.uChID, args.channel)){
			continue;
		}
		else if (args.type && !match(header.ubyDataType, args.type)){
			continue;
		}
	}

}