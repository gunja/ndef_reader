#include "pn532.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DEBUG_DUMP
#define WIDE_INFORM

const unsigned char PN532::wakeup[] = "\x55\x55\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const unsigned char PN532::messageHeader[]="\x00\x00\xff";
const unsigned char PN532::ack[]  ={0x00, 0x00, 0xFF, 0x00, 0xff, 0x00};
const unsigned char PN532::nack[] ={0x00,0x00,0xFF,0xff,0x00,0x00};
PN532::PN532() : fd(-1), receivedBytes(0)
   , isAck(false), isNack(false)
{
}

PN532::~PN532()
{
    if (fd >=0)
        close(fd);
    fd = -1;
    return;
}

bool PN532::openSerial(const char *deviceName)
{
    char buff[200];
    sprintf(buff, "/dev/%s", deviceName);
    fd = open(buff, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd >= 0)
    {
        //TODO set serial parameters if needed
    }
    return (fd >= 0);
}

bool PN532::startCommunication()
{
    if (fd < 0)
        return false;

    if (not warmUp())
        return false;
    //SAMConfiguration
    
#ifdef WIDE_INFORM
    printf("sam configuration\n");
#endif
    sendReceivePayload("\xd4\x14\x01", 3);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //Diagnose
#ifdef WIDE_INFORM
    printf("diagnose\n");
#endif
    sendReceivePayload("\xd4\x00\x00\x6c\x69\x62\x6e\x66\x63", 9);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //GetFirmwareVersion
    sendReceivePayload("\xd4\x02", 2);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //SetParameters
#ifdef WIDE_INFORM
    printf("set parameters\n");
#endif
    sendReceivePayload("\xd4\x12\x14", 3);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //ReadRegister
#ifdef WIDE_INFORM
    printf("read register\n");
#endif
    sendReceivePayload("\xd4\x06\x63\x02\x63\x03\x63\x0d\x63\x38\x63\x3d", 12);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //WriteRegister
#ifdef WIDE_INFORM
    printf("write register\n");
#endif
    sendReceivePayload("\xd4\x08\x63\x02\x80\x63\x03\x80", 8);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //RFConfiguration
#ifdef WIDE_INFORM
    printf("rf configuration\n");
#endif
    sendReceivePayload("\xd4\x32\x01\x00", 4);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //RFConfiguration
#ifdef WIDE_INFORM
    printf("rf configuration\n");
#endif
    sendReceivePayload("\xd4\x32\x01\x01", 4);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //RFConfiguration
#ifdef WIDE_INFORM
    printf("rf configuration\n");
#endif
    sendReceivePayload("\xd4\x32\x05\xff\xff\xff", 6);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //ReadRegister
#ifdef WIDE_INFORM
    printf("read register\n");
#endif
    sendReceivePayload("\xd4\x06\x63\x02\x63\x03\x63\x05\x63\x38\x63\x3c\x63\x3d", 14);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    //WriteRegister
#ifdef WIDE_INFORM
    printf("write register\n");
#endif
    sendReceivePayload("\xd4\x08\x63\x05\x40\x63\x3c\x10", 8);
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    // InAutoPoll
#ifdef WIDE_INFORM
    printf("in auto poll\n");
#endif
    sendCMD_InAutoPoll();
#ifdef WIDE_INFORM
    dumpMessage();
#endif
    // TODO here should be result of all returned TRUE, or earlyer exit
    return true;
}

struct timeval operator-(const struct timeval &a, const struct timeval &b)
{
        struct timeval rv;
        rv.tv_sec = a.tv_sec - b.tv_sec;
        rv.tv_usec = a.tv_usec - b.tv_usec;
        if (rv.tv_usec < 0) {
                rv.tv_sec--;
                rv.tv_usec += 1000000;
        }
        return rv;
}

bool operator>(const struct timeval &a, const struct timeval &b)
{
        if (a.tv_sec > b.tv_sec)
                return true;
        if (a.tv_sec < b.tv_sec )
                return false;
        return (a.tv_usec > b.tv_usec);
}

bool PN532::handleCommunication()
{
    // TODO handle errors on write (even if we dont' actually even write from here)
    struct timeval tv;
    struct timeval reInitInPollTimeout{100, 0};

    bool noErrors = true;

    do {
        gettimeofday(&tv, NULL);
        char * message = readMessage();
#ifdef DEBUG_DUMP
    if (latestMessageLength) {
    printf("received a message\n");
    for(int i=0; i < latestMessageLength; ++i)
        printf("0x%02x ", latestMessage[i]);
    printf("\n");
    }
#endif
        if (latestMessageLength) {
            gettimeofday(&messageReceivedLastStamp, NULL);
            treatInformational();
        }
        else {
            if (tv - messageReceivedLastStamp > reInitInPollTimeout){
#ifdef WIDE_INFORM
			printf("timeout. reinit\n");
#endif
                        sendAck();
                        confirmAck();
                // TODO check return value
                sendCMD_InAutoPoll();
                        gettimeofday(&messageReceivedLastStamp, NULL);
            }
        }
    } while( noErrors);
    return false;
}

void PN532::dumpMessage()
{
    printf("received a message\n");
    for(int i=0; i < latestMessageLength; ++i)
        printf("0x%02x ", latestMessage[i]);
    printf("\n");
}

bool PN532::warmUp()
{
    int r = 0;
    while( r < sizeof(wakeup))
    {
        int s = write(fd, wakeup + r,
            sizeof(wakeup) -r);
        if (s > 0)
            r += s;
	else { if (s == 0)
        {
            //TODO strange situation
        }
        else {
            return false;
        }}
    }
    return true;
}

bool PN532::sendAck()
{
	unsigned char buff[]={0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    int r = 0;
    bool rv = false;
    int sendSize = sizeof(buff);
    while (r < sizeof(buff))
    {
        int s = write(fd, buff + r, sendSize - r);
        if (s < 0) {
            break;
        }
        r += s;
    }
    printf("send Ack\n");
    for(int i=0; i < r; ++i) {
	    printf("%02x ", buff[i]);
    }
    printf("\n");
    return (r == sendSize);
}

bool PN532::sendDeterministic(const unsigned char *payload, int sz)
{
    char *sendBuffer = new char[sz + 7];
    int sendSize = wrapPayloadToBuffer(sendBuffer, payload, sz);

    int r = 0;
    bool rv = false;
    while (r < sendSize)
    {
        int s = write(fd, sendBuffer + r, sendSize - r);
        if (s < 0) {
            return false;
        }
        r += s;
    }
#ifdef WIDE_INFORM
    printf("send Deterministic\n");
    for(int i=0; i < r; ++i) {
	    printf("%02x ", sendBuffer[i]);
    }
    printf("\n");
#endif
    delete[] sendBuffer;
    return (r >= sendSize);
}

int PN532::wrapPayloadToBuffer(char *sendBuffer,
        const unsigned char *payload, int sz)
{
    memcpy(sendBuffer, messageHeader, sizeof(messageHeader));
    int offset = sizeof(messageHeader) -1;
    sendBuffer[offset++] = static_cast<unsigned char>(sz);
    sendBuffer[offset++] = static_cast<unsigned char>(-sz);
    memcpy(sendBuffer + offset, payload, sz);
    offset += sz;
    unsigned char sum = 0;
    for(size_t i=0; i < sz; ++i)
    {
        sum += payload[i];
    }
    sendBuffer[offset++] = -sum;
    sendBuffer[offset++] = 0;
    return offset;
}

bool PN532::confirmAck()
{
    fd_set mask;
    struct timeval timeout = { .tv_sec = 0,
            .tv_usec = 10 * 1000 };
    int r, n;

    FD_ZERO(&mask);
    FD_SET(fd, &mask);

    do
    {
        r = select(fd+1, &mask, NULL, NULL, &timeout);
        if (r > 0)
        {
            r =  read(fd, receivedBuffer + receivedBytes, 600 - receivedBytes);
            if (r > 0)
                receivedBytes +=r;
            if (isAckOrNack())
            {
                receivedBytes -= sizeof(ack) ;
        memmove(receivedBuffer, receivedBuffer + sizeof(ack) ,
                receivedBytes);
                return true;
            }
        }
    } while (r>0);

    if (isAckOrNack())
    {
        receivedBytes -= sizeof(ack) ;
        memmove(receivedBuffer, receivedBuffer + sizeof(ack) ,
                receivedBytes);
        return true;
    }
    return false;
}

char * PN532::sendReceivePayload(const char *payload, int sz)
{
    sendDeterministic(reinterpret_cast<const unsigned char*>(payload), sz);
    confirmAck();
    return readMessage();
}

char * PN532::readMessage()
{
    // there are possibilities to exit this function:
    // all required data received  -> returned payload w/o envelope
    // no new byte received in some amount of time -> error
    fd_set mask;
    struct timeval timeout = { .tv_sec = 0,
            .tv_usec = 10 * 1000 };
    int r, n;

    FD_ZERO(&mask);
    FD_SET(fd, &mask);

    do
    {
        r = select(fd+1, &mask, NULL, NULL, &timeout);
        if (r > 0)
        {
            r =  read(fd, receivedBuffer + receivedBytes, 600 - receivedBytes);
            if (r > 0)
                receivedBytes +=r;
        }
    confirmAck();
        if (canOpenEnvelope())
        {
            removeEnvelope();
            return latestMessage;
        }
    } while (r>0);
    latestMessageLength = 0;
    return NULL;
}

bool PN532::isAckOrNack()
{
    if (receivedBytes < sizeof(ack))
        return false;
    isAck = (memcmp(ack, receivedBuffer, sizeof(ack)) == 0);
    isNack = (memcmp(nack, receivedBuffer, sizeof(ack)) == 0);

    if (isAck || isNack) { printf("      ack or Nack received\n");
	    printf("buffer remains:\n");
	    for(int i=0; i < receivedBytes; ++i)
		    printf("%02x ", receivedBuffer[i]);
	    printf("\n");
    };


    return (isAck || isNack);
}

bool PN532::canOpenEnvelope()
{
    static const unsigned char preamble[3] ={0x00, 0x00, 0xff};
    if (receivedBytes < 5)
        return false;

    if ( memcmp(receivedBuffer, preamble, sizeof(preamble)) != 0)
        return false;
    int len = static_cast<unsigned char>(receivedBuffer[3]);
    int lenConfirm = static_cast<unsigned char>(receivedBuffer[4]);

    if (len == 0xff && lenConfirm == 0xff)
    {
        return canOpenLongMessageEnvelope();
    }
    if (len + lenConfirm != 0x100)
        return false;

    if (receivedBytes < len + 7)
        return false;

    return true;
}

bool PN532::removeEnvelope()
{
    int len = static_cast<unsigned char>(receivedBuffer[3]);
    //if we are here, we a definitive, that there's enough data
    memmove(latestMessage, receivedBuffer + 5, len + 2);
    receivedBytes -= 5;

    // let's sum up bytes in a message
    unsigned char sum = 0;
    for(int i=0; i <= len; ++i)
    {
        sum += latestMessage[i];
    }
    if ( sum == 0)
    {
    latestMessageLength = len;
    memmove(receivedBuffer, receivedBuffer + len + 7,
            receivedBytes - (len + 2));
        receivedBytes -= len + 2;
	printf("in removeEnvelope\n"); dumpMessage();
	gettimeofday(&messageReceivedLastStamp, NULL);
        return true;
    }
    receivedBytes -= len + 6;
    return false;
}

bool PN532::canOpenLongMessageEnvelope()
{
    return false;
}

void PN532::treatInformational()
{
   if (latestMessage[0] == 0xd5 && latestMessage[1] == 0x41)
       return treatBlockData();
   if (latestMessage[0] == 0xd5 && latestMessage[1] == 0x61)
     treatAutoPollReply();

    fprintf(stderr, "Unhadle message\n");
    dumpMessage();
    fprintf(stderr, "Forcing Auto-Poll again\n");
    sendCMD_InAutoPoll();
}

bool PN532::treatAutoPollReply()
{
   uint8_t tagsCount = latestMessage[2];
   uint8_t tagType = latestMessage[3];

   if (tagsCount > 1)
   {
       printf("several tags found (%d). can not read blocks \n", tagsCount);
            sendCMD_InAutoPoll();
        return true;
   }
    if (tagsCount == 0)
   {
	   struct timeval now;
	   gettimeofday(&now, NULL);
           printf(" %lu.%06ld After polling got 0 tags found. Re-initializing\n", now.tv_sec, now.tv_usec);
        sendCMD_InAutoPoll();
        return true;
   }

   if (tagType == 0x11) {
        dumpFelicaCardInfo();
   }
   if (tagType == 0x10) {
        dumpMifareTypeInfo();
       currentBlock = 0;
       return sendCMD_readBlock(currentBlock);
   }

    printf("Card with type =0x%02x not supported\n", tagType);
    sendCMD_InAutoPoll();

    return true;
}

bool PN532::sendCMD_readBlock(int blockNum)
{
    unsigned char exchange[] = {0xd4, 0x40, 0x01, 0x30, 0x00};
    exchange[4] = blockNum;
    return sendDeterministic(exchange, 5);
}

void PN532::treatBlockData()
{
    printf("current Block =%d\n", currentBlock);
    for(int i=0; i < latestMessageLength; ++i)
    {
        printf("%02x ", latestMessage[i]);
    }
    printf("\n");
    if (currentBlock == 0) {
        currentBlock += 4;
        sendCMD_readBlock(currentBlock);
        return;
    }
    if(currentBlock > 3 && currentBlock < 8)
    {
        // here should be search for NDEF start
        int ndefStartData = 0;
        int index = 3;
        while(ndefStartData == 0 && index < latestMessageLength)
        {
                switch(static_cast<unsigned int>(latestMessage[index]))
                {
                        case 0:
                                index++;
                                continue;
                                break;
                        case 1: case 2: case 0xFD:
#ifdef WIDE_INFORM
                                printf("ignorable TLV found\n");
#endif
                                index++; index += static_cast<unsigned int>(latestMessage[index]);
                                break;
                        case 3:
#ifdef WIDE_INFORM
                                printf("found NDEF start at %d\n", index);
#endif
                                ndefStartData = index;
                                break;
                };
        }
    // d5 41 00 03 0b d1 01 07 54 00 32 30 30 30 30 37 fe 00 00
        if (ndefStartData > 0)
        {
       int ndefLenByteLoc = ndefStartData + 1;
        int langCodeLen = latestMessage[ndefStartData+6]& 0x3f;
        int ndefDataLen = latestMessage[ndefStartData+1] - 5 - langCodeLen;
        int wellKnownType = latestMessage[ndefStartData + 5];
        printf("found NDEF: length =%d  record type = %02x\n", ndefDataLen,
                wellKnownType);
        int terminatorPosition = ndefStartData + latestMessage[ndefStartData+1]+2;
        if (terminatorPosition > latestMessageLength)
        {
#ifdef WIDE_INFORM
            fprintf(stderr, "Message expands beyong these blocks. Just ignoring\n");
#endif
            sendCMD_InAutoPoll();
            return;
        }
        if (static_cast<unsigned int>(latestMessage[ndefStartData + 
                    latestMessage[ndefStartData+1]+2]) == 0xFE)
        {
            printf("End of message found\nCreating file");
            char *buffer = new char[ndefDataLen +1];
            memset(buffer, 0, ndefDataLen +1);
            memcpy(buffer, latestMessage+ndefStartData + 5 + langCodeLen + 2,
                    ndefDataLen);
            FILE * c = fopen(buffer, "w");
            fprintf(c, "c");
            fclose(c);
            delete[] buffer;
            sendCMD_InAutoPoll();
        }


        }
        else {
            sendCMD_InAutoPoll();
        }
    }
}

bool PN532::sendCMD_InAutoPoll()
{
    return sendReceivePayload("\xd4\x60\x14\x02\x20\x10\x03\x11\x12\x04", 10);
}

void PN532::dumpFelicaCardInfo()
{
   uint8_t ln1 = latestMessage[4];
       printf("Felica type\n");
       printf("length = %d\n", ln1);
       printf("logical number = %x\n", static_cast<int>( latestMessage[5]));
       printf("POL_RES = %x\n", latestMessage[6]);
       printf("Response code = %x\n", latestMessage[7]);
       printf("message rest = ");
       for(int i =0; i < ln1 - 1; ++i) printf(" %02x", latestMessage[8+i]);
       printf("\n");
}

void PN532::dumpMifareTypeInfo()
{
   uint8_t ln1 = latestMessage[4];
       printf("Mifare type\n");
       printf("length = %d\n", ln1);
       printf("logical number = %x\n", static_cast<int>( latestMessage[5]));
       printf("SENS_RES = %04x\n", 256 *latestMessage[6] + latestMessage[7]);
       printf("SEL_RES = %02x\n", latestMessage[8]);
       printf("NFCID1t length = %d\n", latestMessage[9]);
       printf("NFCID1t = ");
       for(int i=0; i < latestMessage[9]; ++i) printf(" %02x", latestMessage[10 + i]);
       printf("\n");
}
