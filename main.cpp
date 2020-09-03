/**
    Rework of Java version of reader
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "pn532.h"

int keepWork;

void sigTermHandler(int)
{
    keepWork = 0;
}

void showHelp()
{
    printf("This app accepts -h and -d directory arguments\n");
    return;
}

int main(int argc, char *argv[])
{
    ssize_t r;
    int fd;
    keepWork = 1;
    PN532 pn532;
    char *requestedDir = NULL;

    int opt;
    while((opt = getopt(argc, argv, "hd:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
                break;
            case 'd':
                requestedDir = optarg;
                break;
            default:
                showHelp();
                exit(EXIT_FAILURE);
        }
    }
    if (requestedDir != NULL)
    {
        pn532.setOutputDirectory(requestedDir);
    } else {
        pn532.setOutputDirectory("./");
    }

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

