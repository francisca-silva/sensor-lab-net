#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include "Node.h"

const int MAX_STUDENT_NODES = 124;
const unsigned long NODE_CONNECTION_CHECK_INTERVAL = 10000;
const unsigned long NETWORK_STATUS_SEND_INTERVAL = 10000;

const int MAX_ALERT_PER_STUDENT = 1;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const unsigned long SENSOR_DATA_UPDATE_INTERVAL = 5000;
const unsigned long NODE_ALERT_CHECK_INTERVAL = 5000;

const char SELF_ID_REQUEST = 'N';
const char ID_REQUEST = 'I';
const char ALERT_REQUEST = 'A';
const char ALERT_DEACTIVATION = 'D';

struct Student_Node
{
  uint16_t nodeID;
  char name[NAME_LENGTH];
};

struct Active_Node {
    Student_Node node;
    Alert_Request alerts[MAX_ALERT_PER_STUDENT];
    bool status = false;
    long time;
};

class SensorNode : public Node {
public:
    SensorNode(uint16_t node, char *name, int channel);

    void init() override;
    void receivePayload() override;
    void checkNodesConnection();
    void sendNetworkStatus();
    void checkAlerts();

private:
    Sensor_Node sensorData;
    Active_Node active_nodes[MAX_STUDENT_NODES];

    unsigned long last_sent_keep_alive;

    void receiveKeepAlive(RF24NetworkHeader &header);
    void populateActiveNodesArray();
    int octalToDecimal(uint16_t octalNumber);
    uint16_t receiveNodeIDRequest(RF24NetworkHeader &header);
    void sendNextAvailableNodeID(uint16_t to, uint16_t id);
    uint16_t receiveNodeIDRequestFromName(RF24NetworkHeader &header);
    void sendNodeID(uint16_t to, uint16_t id);
    void receiveAlertRequest(RF24NetworkHeader &header);
    Alert_Request deserializeAlert(uint8_t *buffer);
    void receiveAlertDeactivationRequest(RF24NetworkHeader &header);
    void cleanAlertArray(uint16_t to);
    void receiveReadingsRequest(RF24NetworkHeader &header);
    void sendReadings(uint16_t to);
    // void serializeSensorNode(uint8_t *buffer);
    // Sensor_Node deserializeSensorNode(uint8_t* buffer);
    // int decimalToOctal(uint16_t octalNumber);
    void sendBeginFlagArray();
    void sendArrayOfActiveNodes();
    void serializeAlert(const Alert_Request &request, uint8_t *buffer);
};

#endif