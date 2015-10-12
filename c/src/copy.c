
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "irig106ch10.h"
#include "copy_args.c"
#include "common.h"

int main(int argc, char ** argv){

    // Get commandline args.
    DocoptArgs args = docopt(argc, argv, 1, "1");
    if (argc < 3){
        printf(args.usage_pattern);
        return quit(1);
    }

    // Open input and output files.
    int input_handle;
    EnI106Status status = enI106Ch10Open(&input_handle, argv[1], I106_READ);
    if (status != I106_OK){
        char msg[200] = "Error opening source file: ";
        strcat(msg, argv[1]);
        return error(msg);
    }
    FILE * output = fopen(argv[2], "wb");
    if (output == NULL){
        return error("Couldn't open destination file.");
    }

    SuI106Ch10Header header;
    void * buffer = malloc(24);

    // Copy TMATS
    status = enI106Ch10ReadNextHeader(input_handle, &header);
    if (status != I106_OK){
        printf("Finished");
        return quit(0);
    }
    buffer = realloc(buffer, header.ulPacketLen);
    status = enI106Ch10ReadDataFile(input_handle, header.ulPacketLen, buffer);
    if (status != I106_OK){
        printf("Error reading TMATS.");
        return quit(0);
    }
    int header_len = 24;
    if (header.ubyPacketFlags & (0x1 << 7)){
        header_len = 36;
    }
    fwrite(&header, header_len, 1, output);
    fwrite(buffer, header.ulPacketLen - header_len, 1, output);

    // Iterate over packets based on args.
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

        // Copy packet to new file.
        buffer = realloc(buffer, header.ulPacketLen);
        status = enI106Ch10ReadDataFile(input_handle, header.ulPacketLen, buffer);
        if (status != I106_OK){
            printf("Error reading packet.");
            continue;
        }
        header_len = 24;
        if (header.ubyPacketFlags & (0x1 << 7)){
            header_len = 36;
        }
        fwrite(&header, header_len, 1, output);
        fwrite(buffer, header.ulPacketLen, 1, output);
    }

}
