
/*==========================================================================

  dump.c - Export channel data based on channel ID or data type.

 Copyright (c) 2015 Micah Ferrill

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met:

   * Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may 
     be used to endorse or promote products derived from this software 
     without specific prior written permission.

 This software is provided by the copyright holders and contributors 
 "as is" and any express or implied warranties, including, but not 
 limited to, the implied warranties of merchantability and fitness for 
 a particular purpose are disclaimed. In no event shall the copyright 
 owner or contributors be liable for any direct, indirect, incidental, 
 special, exemplary, or consequential damages (including, but not 
 limited to, procurement of substitute goods or services; loss of use, 
 data, or profits; or business interruption) however caused and on any 
 theory of liability, whether in contract, strict liability, or tort 
 (including negligence or otherwise) arising in any way out of the use 
 of this software, even if advised of the possibility of such damage.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <intrin.h>

#include "irig106ch10.h"
#include "dump_args.c"
#include "common.h"

// Byteswap a buffer.
void swap(char *p, int len) {
    char tmp;
    for (int i = 0; i < ((len / 2)); i++) {
        tmp = p[i * 2];
        p[i * 2] = p[(i * 2) + 1];
        p[(i * 2) + 1] = tmp;
    }
}

int main(int argc, char ** argv){
    DocoptArgs args = docopt(argc, argv, 1, "1");
    int input_handle;
    SuI106Ch10Header header;
    int packets = 0;
    char * buffer = malloc(24);
    char * ts = malloc(188);
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
            out[header.uChID] = fopen(filename, "wb");

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

        // Ignore first 4 bytes (CSDW)
        int datalen = header.ulDataLen - 4;
        if (0x3F < header.ubyDataType < 0x43){
            for (int i = 0; i < (datalen / 188); i++){
                memcpy(ts, buffer + 4 + (i * 188), 188);
                swap(ts, 188);
                fwrite(ts, 1, 188, out[header.uChID]);
            }
        }
        else {
            fwrite(buffer + 4, 1, datalen, out[header.uChID]);
        }

    }

    return quit(0);
}
