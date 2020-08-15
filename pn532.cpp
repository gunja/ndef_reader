#include "pn532.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char PN532::wakeup[] = "\x55\x55\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const char PN532::messageHeader[]="\x00\x00\xff";
PN532::PN532() : fd(-1), receivedBytes(0)
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
    return false;
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
    int offset = sizeof(messageHeader);
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
            if (isAckOrNack())
            {
                receivedBytes = 0;
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
}

bool PN532::isAckOrNack()
{
    return false;
}

