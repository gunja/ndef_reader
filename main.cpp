/**
    Rework of Java version of reader
*/
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "pn532.h"

char  SAMConf[] = "\x00\x00\xff\x03\xfd\xd4\x14\x01\x17\x00";


int main(int argc, char *argv[])
{
    ssize_t r;
    int fd;
    int keepWork = 1;
    PN532 pn532;

    while(keepWork)
    {
        if( pn532.openSerial("ttyS0"))
        {
            if (pn532.startCommunication())
            {
                pn532.handleCommunication();
            }
        } else {
	        fprintf(stderr, "Failed to open Serial port\n");
        	sleep(1);
	}
    }

    return 0;
}

