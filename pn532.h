#ifndef PN532_H
#define PN532_H

#include <stdint.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

class PN532
{
    int fd;
    static const unsigned char wakeup[];
    static const unsigned char messageHeader[];
    static const unsigned char ack[];
    static const unsigned char nack[];
    struct termios prev_sets;

    bool setSerialParams();
    bool warmUp();
    bool sendDeterministic(const unsigned char *payload, int sz);
    int wrapPayloadToBuffer(char *sendBuffer, const unsigned char *payload, int sz);
    char *sendReceivePayload(const char *payload, int);
    bool confirmAck();
    bool sendAck();

    char receivedBuffer[600];
    int receivedBytes;
    char latestMessage[300];
    char blockInformation[400];
    int blockBytesReceived;
    int latestMessageLength;
    bool isAck;
    bool isNack;
    char * dirPath;
    int currentBlock;

    enum {
            NO_ERROR,
            SEND_FAILURE,
            ACK_RECEIVE_FAILURE,
            RECEIVE_FAILURE,
            RECEIVE_TIMEOUT,
            CRC_FAILURE
    } internalErrorCode;

    char * readMessage();
    bool isAckOrNack();
    bool canOpenEnvelope();
    bool canOpenLongMessageEnvelope();
    bool removeEnvelope();

    void dumpMessage();
    void treatInformational();
    bool sendCMD_readBlock(int);
    void treatBlockData();

    struct timeval messageReceivedLastStamp;

    bool sendCMD_InAutoPoll();
    bool treatAutoPollReply();

    void dumpFelicaCardInfo();
    void dumpMifareTypeInfo();

    bool createPathIfNeeded(const char *dirName);
  public:
    PN532();
    ~PN532();
    bool openSerial(const char *);
    bool startCommunication();
    bool handleCommunication(int *keep = NULL);
    bool setOutputDirectory(const char *);
    void closeSerial();
    void delayCycle();
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
