#ifndef PN532_H
#define PN532_H

class PN532
{
    int fd;
    static const char wakeup[];
    static const char messageHeader[];
    void warmUp();
    bool sendDeterministic(const char *payload, int sz);
    int wrapPayloadToBuffer(char *sendBuffer, const char *payload, int sz);
    char *sendReceivePayload(const char *payload, int);
    bool confirmAck();
    char receivedBuffer[600];
    int receivedBytes;
    char * readMessage();
    bool isAckOrNack();
  public:
    PN532();
    ~PN532();
    bool openSerial(const char *);
    bool startCommunication();
    bool handleCommunication();
};

#endif
