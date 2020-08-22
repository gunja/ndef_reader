#ifndef PN532_H
#define PN532_H

#include <stdint.h>
#include <sys/time.h>

class PN532
{
    int fd;
    static const char wakeup[];
    static const char messageHeader[];
    static const char ack[];
    static const char nack[];
    void warmUp();
    bool sendDeterministic(const char *payload, int sz);
    int wrapPayloadToBuffer(char *sendBuffer, const char *payload, int sz);
    char *sendReceivePayload(const char *payload, int);
    bool confirmAck();
    bool sendAck();
    char receivedBuffer[600];
    int receivedBytes;
    char latestMessage[300];
    int latestMessageLength;
    char * readMessage();
    bool isAckOrNack();
    bool isAck;
    bool isNack;

    bool canOpenEnvelope();
    bool canOpenLongMessageEnvelope();
    bool removeEnvelope();

    void dumpMessage();
    void treatInformational();
    int currentBlock;
    void readBlock(int);
    void treatBlockData();

    struct timeval messageReceivedLastStamp;
  public:
    PN532();
    ~PN532();
    bool openSerial(const char *);
    bool startCommunication();
    bool handleCommunication();
};

#pragma pack(push, 1)
struct messageD561
{
	uint8_t tagsCount;
	uint8_t msgLen;
	uint8_t logicalNumber;

};
#pragma pack(pop)

#endif
