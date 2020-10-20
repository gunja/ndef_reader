/**
    Rework of Java version of reader
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "pn532.h"

#include "generated.h"

int keepWork;

void sigTermHandler(int)
{
    keepWork = 0;
}

void showHelp()
{
    printf("This app accepts following arguments:\n");
    printf("-h to display this help and exit\n");
    printf("-d directory to set directory into which files will be generated (./ in case of nothing provided and /tmp if creation of files will fail\n");
    printf("-p portFile   to override serial device to be opened (/dev/ttyS0 if nothing is provided, or requested serial device was not opened\n");
    printf("-v show version info of this particular build\n");
    return;
}

void showVersionInfo()
{
    printf("Build branch: %s\n", BRANCH);
    printf("Build commit: %s\n", REVISION);
    printf("Repo state on build was: %s\n", IS_DIRTY);
}

int main(int argc, char *argv[])
{
    ssize_t r;
    int fd;
    keepWork = 1;
    PN532 pn532;
    char *requestedDir = NULL;
    char *requestedPort = NULL;

    int opt;
    while((opt = getopt(argc, argv, "hd:p:v")) != -1)
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
            case 'p':
                requestedPort = optarg;
                break;
            case 'v':
                showVersionInfo();
                exit(EXIT_SUCCESS);
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

    // Ctrl-C from keyboard, or SIGTERM from systemd
    signal(SIGINT, sigTermHandler);
    signal(SIGTERM, sigTermHandler);

    while(keepWork)
    {
        if( pn532.openSerial(requestedPort))
        {
            bool communicationResult = false;
            if (pn532.startCommunication())
            {
                communicationResult = pn532.handleCommunication(&keepWork);
            }
            // we can get here if handleCommunication failed
            // or even startCommunication failed.
            if (not communicationResult) {
                pn532.closeSerial();
                pn532.delayCycle();
            }
        } else {
	        fprintf(stderr, "Failed to open Serial port\n");
        	sleep(1);
        }
    }

    return 0;
}

