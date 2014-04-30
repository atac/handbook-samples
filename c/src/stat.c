
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

// Pause and then exit the program.
int quit(int status){
	printf("\n\nPress ENTER to exit");
	getchar();
	return status;
}

// Exit with an error message.
int error(char msg[]){
	printf(msg);
	return quit(1);
}

// Show the contents of a file.
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

	return quit(0);
}

// Find or create a spec for a given channel and data type combination.
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

// Test a CSV string "value" for matches to int "key".
int match(int key, const char value[]){
	long *val;
	char **num;
	char s1[50];
	strcpy(s1, value);
	char *t1 = strtok(s1, ",");
	while (t1 != NULL){
		val = strtol(t1, &num, 0);
		if (val == key){
			return 1;
		}
		t1 = strtok(NULL, ",");
	}
	return 0;
}

int main(int argc, char ** argv){

	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;
	int packets = 0;
	float byte_size = 0.0;
	static ChanSpec * channels[0x10000];

	// Validate arguments and offer help.
	if (args.help){
		printf(args.help_message);
		return 1;
	}
	else if (argc < 2){
		printf(args.usage_pattern);
		return 1;
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
			return print_results(argv[1], byte_size, packets, channels);
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

		// Track this packet.
		int i = get_channel_index(channels, header.uChID, header.ubyDataType);
		channels[i]->packets++;

		// Increment top-level counters.
		byte_size += header.ulPacketLen;
		packets++;
	}

}