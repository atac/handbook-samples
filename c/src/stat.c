
/*==========================================================================

  stat.c - Display some basic information about the channels within a Chapter 10
    file.

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

#include "irig106ch10.h"
#include "stat_args.c"
#include "common.h"

typedef struct {
    unsigned int type;
    int id;
    int packets;
} ChanSpec;


int main(int argc, char ** argv){

    // Get commandline args.
    DocoptArgs args = docopt(argc, argv, 1, "1");
    if (args.help){
        printf(args.help_message);
        return 1;
    }
    else if (argc < 2){
        printf(args.usage_pattern);
        return 1;
    }

    SuI106Ch10Header header;
    int packets = 0, input_handle;
    float byte_size = 0.0;
    static ChanSpec * channels[0x10000];

    // Open the source file.
    EnI106Status status = enI106Ch10Open(&input_handle, argv[1], I106_READ);
    if (status != I106_OK){
        char msg[200] = "Error opening file ";
        strcat(msg, argv[1]);
        return error(msg);
    }

    // Iterate over selected packets (based on args).
    while (1){
        status = enI106Ch10ReadNextHeader(input_handle, &header);

        // Exit once file ends.
        if (status != I106_OK){
            break;
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

        // Increment overall size and packet counts.
        byte_size += header.ulPacketLen;
        packets++;

        // Find the channel info based on ID and type.
        int i = 0;
        for (i; i < 0x10000; i++){

            // Create a listing if none exists.
            if (channels[i] == NULL){
                channels[i] = (ChanSpec *)malloc(sizeof(ChanSpec));
                memset(channels[i], 0, sizeof(ChanSpec));
                channels[i]->id = (unsigned int)header.uChID;
                channels[i]->type = (unsigned int)header.ubyDataType;
                channels[i]->packets = 0;
            }

            if (channels[i]->id == (unsigned int)header.uChID && channels[i]->type == (unsigned int)header.ubyDataType){
                break;
            }
        }

        // Increment the counter.
        channels[i]->packets++;

    }

    // Print details for each channel.
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

    // Find a more readable size unit than bytes.
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

    // Print file summary.
    printf("--------------------------------------------------------------------------------\n");
    printf("Summary for %s:\n", argv[1]);
    printf("    Size: %.*f%s\n", 2, byte_size, unit);
    printf("    Packets: %d\n", packets);
    printf("    Channels: %d\n", i);

    return quit(0);
}
