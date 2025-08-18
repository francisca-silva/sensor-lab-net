#if defined(ESP32)

#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include "Node.h"
#include "SerialUSBMessaging.h"

const int MAX_STUDENT_NODES = 24;   // Maximum number of publishers in rosserial, if not for this limitation, it could be 124
const unsigned long NODE_CONNECTION_CHECK_INTERVAL = 10000;
const unsigned long NETWORK_STATUS_SEND_INTERVAL = 10000;

const int MAX_ALERT_PER_STUDENT = 1;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const unsigned long SENSOR_DATA_UPDATE_INTERVAL = 5000;
const unsigned long NODE_ALERT_CHECK_INTERVAL = 5000;


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
    SensorNode(uint16_t node, char *name, int channel, int maxNumBots);

    void init() override;
    void receivePayload() override;
    void checkNodesConnection();
    void sendKeepAlive();
    void sendNetworkStatus();
    void checkAlerts();
    void checkSerialUSBMessaging();

    template <typename... Args>
    void log(Args... args)
    {
        String info = String(millis()) + F(": ");
        using expander = int[];
        (void)expander{0, (info += String(args), 0)...};
        messager.sendFromNodeX(LOG, info.c_str());
    }

private:
    Sensor_Node sensorData;
    Active_Node active_nodes[MAX_STUDENT_NODES];

    SerialUSBMessaging messager;

    unsigned long last_sent_keep_alive;
    int maxNumBots;

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
    void sendArrayOfActiveNodes();
    void serializeAlert(const Alert_Request &request, uint8_t *buffer);


    class Action {
        public:
        String type; // Type of action (e.g., "add", "remove")
        uint16_t nodeID; // Node ID associated with the action
        String content;
        time_t timestamp; // Timestamp when the action was initiated
        Action(const String& t, const uint16_t id, const String& c = "") 
            : type(t), nodeID(id), content(c), timestamp(millis()) {};
    };
    std::vector<Action> actionsWaitingForConfirmation;
    unsigned long actionTimeout = 5000; // Timeout in milliseconds for waiting for the confirmation of the action

    void addAction(const String type, const uint16_t nodeID, const String content = "");
    void receiveActionConfirmation(const uint16_t nodeID, const String type);
    void checkForActionConfirmation();
};

#endif // SENSOR_NODE_H

#endif // ESP32