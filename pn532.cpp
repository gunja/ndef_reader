#include "pn532.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char PN532::wakeup[] = "\x55\x55\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const char PN532::messageHeader[]="\x00\x00\xff";
const char PN532::ack[]  ="\x00\x00\xFF\x00\xff\x00";
const char PN532::nack[] ="\x00\x00\xFF\xff\x00\x00";
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

    warmUp();
    //SAMConfiguration
    
    sendReceivePayload("\xd4\x14\x01", 3);
    //Diagnose
    sendReceivePayload("\xd4\x00\x00\x6c\x69\x62\x6e\x66\x63", 9);
    //GetFirmwareVersion
    sendReceivePayload("\xd4\x02", 2);
    //SetParameters
    sendReceivePayload("\xd4\x12\x14", 3);
    //ReadRegister
    sendReceivePayload("\xd4\x06\x63\x02\x63\x03\x63\x0d\x63\x38\x63\x3d", 12);
    //WriteRegister
    sendReceivePayload("\xd4\x08\x63\x02\x80\x63\x03\x80", 8);
    //RFConfiguration
    sendReceivePayload("\xd4\x32\x01\x00", 4);
    //RFConfiguration
    sendReceivePayload("\xd4\x32\x01\x01", 4);
    //RFConfiguration
    sendReceivePayload("\xd4\x32\x05\xff\xff\xff", 6);
    //ReadRegister
    sendReceivePayload("\xd4\x06\x63\x02\x63\x03\x63\x05\x63\x38\x63\x3c\x63\x3d", 14);
    //WriteRegister
    sendReceivePayload("\xd4\x08\x63\x05\x40\x63\x3c\x10", 8);
    // InAutoPoll
    sendReceivePayload("\xd4\x60\x14\x02\x20\x10\x03\x11\x12\x04", 10);
    return true;
}

bool PN532::handleCommunication()
{
    do {
        char * message = readMessage();
	printf("received a message\n");
	for(int i=0; i < latestMessageLength; ++i)
		printf("0x%02x ", latestMessage[i]);
	printf("\n");
        
    } while( receivedBytes > 0);
    return false;
}

void PN532::dumpMessage()
{
    printf("received a message\n");
    for(int i=0; i < latestMessageLength; ++i)
        printf("0x%02x ", latestMessage[i]);
    printf("\n");
}

void PN532::warmUp()
{
    int r = 0;
    while( r < sizeof(wakeup))
    {
        int s = write(fd, wakeup + r,
            sizeof(wakeup) -r);
        if (s > 0)
            r += s;
        if (s == 0)
        {
            //TODO strange situation
        }
        else {
            break;
        }
    }
}

bool PN532::sendDeterministic(const char *payload, int sz)
{
    char *sendBuffer = new char[sz + 7];
    int sendSize = wrapPayloadToBuffer(sendBuffer, payload, sz);

    int r = 0;
    bool rv = false;
    while (r < sendSize)
    {
        int s = write(fd, sendBuffer + r, sendSize - r);
        if (s < 0) {
            break;
        }
        r += s;
    }
    delete[] sendBuffer;
    return (r >= sendSize);
}

int PN532::wrapPayloadToBuffer(char *sendBuffer,
        const char *payload, int sz)
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
                receivedBytes -= sizeof(ack) - 1;
		memmove(receivedBuffer, receivedBuffer + sizeof(ack) - 1,
				receivedBytes);
                return true;
            }
        }
    } while (r>0);
    return false;
}

char * PN532::sendReceivePayload(const char *payload, int sz)
{
    sendDeterministic(payload, sz);
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
        if (canOpenEnvelope())
        {
            removeEnvelope();
            return latestMessage;
        }
    } while (r>0);
    return NULL;
    
}

bool PN532::isAckOrNack()
{
    if (receivedBytes < sizeof(ack))
        return false;
    isAck = (memcmp(ack, receivedBuffer, sizeof(ack)) == 0);
    isNack = (memcmp(nack, receivedBuffer, sizeof(ack)) == 0);

    return (isAck || isNack);
}

bool PN532::canOpenEnvelope()
{
    static const char preamble[3] ={0x00, 0x00, 0xff};
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
        return true;
    }
    receivedBytes -= len + 6;
    return false;
}

bool PN532::canOpenLongMessageEnvelope()
{
    return false;
}

