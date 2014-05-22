
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <intrin.h>

#include "irig106ch10.h"
#include "dump_args.c"
#include "common.h"

int main(int argc, char ** argv){
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;
	int packets = 0;
	void * buffer = malloc(24);
	FILE *out[0x10000];

	// Initialize out to NULLs.
	for (int i = 0; i < 0x10000; i++){
		out[i] = NULL;
	}

	// Validate arguments and offer help.
	if (argc < 2){
		printf(args.usage_pattern);
		return quit(1);
	}

	// Open file for reading.
	EnI106Status status = enI106Ch10Open(&input_handle, argv[1], I106_READ);
	if (status != I106_OK){
		char msg[200] = "Error opening source file: ";
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

		// Get the correct filename for this channel.
		char filename[10000];
		strcpy(filename, args.output);
		char channel[5];
		sprintf(channel, "/%d", header.uChID);
		strcat(filename, channel);

		// Check for video (byte-swap required)
		if (0x3F < header.ubyDataType < 0x43){
			strcat(filename, ".mpg");
		}

		// Ensure an output file is open for this channel.
		if (out[header.uChID] == NULL){
			out[header.uChID] = fopen(filename, "w");

			if (out[header.uChID] == NULL){
				printf("Error opening output file: %s", filename);
				return quit(1);
			}
		}

		// Read packet data.
		buffer = realloc(buffer, header.ulPacketLen);
		status = enI106Ch10ReadDataFile(input_handle, header.ulPacketLen, buffer);
		if (status != I106_OK){
			printf("Error reading packet.");
			continue;
		}

		unsigned long(*array)[150] = (unsigned long(*)[150]) buffer;

		// Write packet to file.
		fwrite(&buffer, header.ulPacketLen, 1, out[header.uChID]);

	}

	return quit(0);
}