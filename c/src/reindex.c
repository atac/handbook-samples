
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "irig106ch10.h"
#include "reindex_args.c"
#include "common.h"

void gen_node(int64_t offset, SuI106Ch10Header * packets, int packets_c, int64_t offsets[], FILE * output, int seq){
	printf("Index node for %d packets\n", packets_c);

	// Packet header
	SuI106Ch10Header index_header;
	index_header.uSync = 0xeb25;
	index_header.uChID = 0;
	index_header.ulPacketLen = 36 + (18 * packets_c);
	index_header.ulDataLen = 12 + (18 * packets_c);
	index_header.ubyHdrVer = 0x06;
	index_header.ubySeqNum = seq;
	index_header.ubyPacketFlags = 0;
	index_header.ubyDataType = 0x03;
	index_header.uChecksum = 0;
	uint64_t sums = 0xeb25 + ((12 + (18 * packets_c)) * 2);
	sums += 24 + 6 + seq + 3;
	for (int i = 0; i <= 6; i++){
		sums += index_header.aubyRefTime[i];
	}
	sums &= 0xffff;
	index_header.uChecksum = (uint16_t)sums;
	fwrite(&index_header, 24, 1, output);

	// CSDW
	int32_t csdw = 0;
	csdw &= (1 << 31);
	csdw &= (1 << 30);
	fwrite(&csdw, 4, 1, output);

	// File size
	fwrite(&offset, 8, 1, output);

	// Packets
	for (int i = 0; i <= packets_c; i++){
		SuI106Ch10Header packet = packets[i];
		
		// IPTS
		fwrite(&index_header.aubyRefTime, 1, 6, output);

		// Index entry
		int8_t filler = 0;
		fwrite(&filler, 1, 1, output);
		fwrite(&packet.ubyDataType, 1, 1, output);
		fwrite(&packet.uChID, 2, 1, output);
		fwrite(&offsets[i], 8, 1, output);
	}
}

int increment(seq){
	seq++;
	if (seq > 0xFF){
		seq = 0;
	}
	return seq;
}

int main(int argc, char ** argv){
	DocoptArgs args = docopt(argc, argv, 1, "1");
	int input_handle;
	SuI106Ch10Header header;
	void * buffer = malloc(24);

	// Validate arguments and offer help.
	if (argc < 3){
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

	// Open output file.
	FILE * output = fopen(argv[2], "wb");
	if (output == NULL){
		return error("Couldn't open destination file.");
	}

	int packets_c = 0;
	SuI106Ch10Header packets_arr[20000];
	int64_t offsets[20000];
	SuI106Ch10Header last_packet;
	int last_packet_at;
	int node_seq = 0;
	int64_t offset;
	int *hdrptr;
	int hdrlen;

	// Parse loop.
	while (1){

		enI106Ch10GetPos(input_handle, &offset);

		// Read next header or exit.
		status = enI106Ch10ReadNextHeader(input_handle, &header);
		if (status != I106_OK){
			printf("Finished");
			return quit(0);
		}

		// Ensure that we're interested in this particular packet.
		if (header.ubyDataType == 0x3){
			continue;
		}

		// Read packet data.
		buffer = realloc(buffer, header.ulPacketLen);
		status = enI106Ch10ReadDataFile(input_handle, header.ulPacketLen, buffer);
		if (status != I106_OK){
			printf("Error reading packet.");
			continue;
		}

		if (header.ubyDataType != 0x03){
			hdrptr = &header;
			if (header.ubyPacketFlags & (0x1 << 7)){
				hdrlen = 36;
			}
			else {
				hdrlen = 24;
			}

			// Write packet to file.
			fwrite(hdrptr, hdrlen, 1, output);
			fwrite(buffer, header.ulDataLen, 1, output);

			last_packet = header;
			//enI106Ch10GetPos(input_handle, &last_packet_at);
			packets_c++;
			packets_arr[packets_c] = header;
			offsets[packets_c] = offset - header.ulPacketLen;
		}

		if (args.strip){
			continue;
		}

		if ((header.ubyDataType == 0x02 || header.ubyDataType == 0x11) || (packets_c > 20000)){
			// Generate node packet.
			gen_node(offset, &packets_arr, packets_c, offsets, output, node_seq);
			node_seq = increment(node_seq);
			packets_c = 0;
		}

	}

	if (args.strip){
		printf("Stripped existing indices.");
	}
}