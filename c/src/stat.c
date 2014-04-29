
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "irig106ch10.h"
#include "stat_args.c"

typedef struct {
	unsigned int type;
	int id;
	int packets;
} ChanSpec;

typedef struct {
	char filename[256];
	int byte_size;
	int packets;
	ChanSpec * channels[0x10000];
} FileSpec;

int quit(int status){
	printf("\n\nPress ENTER to exit");
	getchar();
	return status;
}

int error(char msg[]){
	printf(msg);
	return quit(1);
}

int print_results(char filename[], float byte_size, int packets, ChanSpec * channels[]){

	// Show channels
	printf("Channel ID      Data Type%35sPackets\n", "");
	printf("--------------------------------------------------------------------------------\n");
	int i = 0;
	while (channels[i] != NULL){
		printf("Channel%3d", channels[i]->id);
		printf("%6s", "");
		printf("0x%-34x", channels[i]->type);
		printf("%7d packets", channels[i]->packets);
		printf("\n");
		i++;
	}

	printf("--------------------------------------------------------------------------------\n");
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

	printf("    Size: %.*f%s\n", 2, byte_size, unit);
	printf("    Packets: %d\n", packets);
	printf("    Channels: %d\n", i);

	// Wait for explicit exit.
	return quit(0);
}

int get_channel_index(ChanSpec * channels[], unsigned int id, unsigned int type){
	for (int i = 0; i < 0x10000; i++){

		// Create new channel listing.
		if (channels[i] == NULL){
			channels[i] = (ChanSpec *) malloc(sizeof(ChanSpec));
			memset(channels[i], 0, sizeof(ChanSpec));
			channels[i]->id = id;
			channels[i]->type = type;
			channels[i]->packets = 0;
			return i;
		}

		// Return existing listing index.
		if (channels[i]->id == id && channels[i]->type == type){
			return i;
		}
	}
}

int main(int argc, char ** argv){

	// Parse args and declare variables.
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;
	int packets = 0;
	float byte_size = 0.0;
	static ChanSpec * channels[0x10000];

	// Open file for reading.
	char filename[] = "C:/Users/mcferrill/ATAC/Data/d0001.c10";
	EnI106Status status = enI106Ch10Open(&input_handle, filename, I106_READ);
	if (status != I106_OK){
		char msg[200] = "Error opening file ";
		strcat(msg, filename);
		return error(msg);
	}

	// Parse loop.
	while (1){
		status = enI106Ch10ReadNextHeader(input_handle, &header);
		if (status != I106_OK){
			return print_results(filename, byte_size, packets, channels);
		}

		int i = get_channel_index(channels, header.uChID, header.ubyDataType);
		channels[i]->packets++;

		byte_size += header.ulPacketLen;
		packets++;
	}

}