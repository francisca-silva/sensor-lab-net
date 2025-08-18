#if not defined(ESP32)

#ifndef STUDENT_NODE_H
#define STUDENT_NODE_H

#include "Node.h"
#include "movementHandler.h"

const uint16_t NODE_BASE = 001;
const int MAX_FAILED_MESSAGES = 5;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const unsigned long ID_REQUEST_DELAY = 5000;


template <typename T>
struct Message
{
    char type = '\0';
    T content;
};

class StudentNode : public Node
{

public:
    /// @brief Receive a simple message
    /// @tparam T The type of the message content
    /// @return The message received
    template <typename T>
    Message<T> receiveSimpleMessage()
    {
      network.update(); // Pump the network regularly
      Message<T> message;
      while (network.available())
      {                           // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        network.read(header, &message.content, sizeof(message.content));
        message.type = header.type;
        log(F(": Message received from "), header.from_node, F(" type "), (char)header.type);
      }
      return message;
    }
    
    StudentNode(uint16_t sensorNode, char *name, int channel);

    void init() override;
    void receivePayload() override;

    void performEssentialOperations();
    // void sendAlertRequestToSensorNode(char type, int value);
    // void sendAlertDeactivationToSensorNode();
    uint16_t getNodeID(char *name_pointer);

    /// @brief Override of the sendPayload method to only allow certain message types.
    /// @param to The node ID to send the message to.
    /// @param type The type of the message.
    /// @param payload The payload to send.
    /// @return True if the message was sent successfully, false otherwise.
    template <typename T>
    bool sendPayload(uint16_t to, char type, const T &payload)
    {

        if (type == SELF_ID_REQUEST || type == ID_REQUEST || type == READINGS_REQUEST)
        {
            log(F(": Message types 'I', 'N', and 'R' are reserved"));
            return false;
        }

        bool ok = Node::sendPayload(to, type, payload);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
        return ok;
    }

    /// @brief Sends a radio message to a specific node.
    /// @details This function first sends an ID request to the sensor node to get the destination ID.
    /// It then waits to receive the node ID and sends the radio message to that node.
    /// @param name_pointer The name of the destination node.
    /// @param type The type of the message.
    /// @param message The message to send.
    template <typename T>
    void sendMessage(char *name_pointer, char type, const T &message)
    {
        if (type == SELF_ID_REQUEST || type == ID_REQUEST || type == READINGS_REQUEST)
        {
            log(F(": Message types 'I', 'N', and 'R' are reserved"));
            return;
        }

        char name[NAME_LENGTH];
        strcpy(name, name_pointer);
        // log(F(": ID request sent to "), _sensorNode, F("(with name "), name, F(")"));
        bool ok = Node::sendPayload(_sensorNode, ID_REQUEST, name);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
        receivePayload();

        log(F(": Message sent to "), nodeID);
        ok = Node::sendPayload(nodeID, type, message);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
    }


protected:
    char _name[NAME_LENGTH];

    uint16_t _sensorNode;

    unsigned long last_sent_keep_alive; // When did we send the last keep alive?
    int countFailedMessages = 0;
    uint16_t nodeID = 0;

    void sendKeepAlive(const unsigned long interval);
    // Sensor_Node deserializeSensorNode(uint8_t* buffer);
    // void serializeAlert(const Alert_Request &temp, uint8_t *buffer);
    // Alert_Request deserializeAlert(uint8_t* buffer);
    void restart();

    MovementHandler movementHandler;

};

#endif // STUDENT_NODE_H

#endif // ESP32