#ifndef PN532_H
#define PN532_H

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
  public:
    PN532();
    ~PN532();
    bool openSerial(const char *);
    bool startCommunication();
    bool handleCommunication();
};

#endif
