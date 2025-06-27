#include "StudentNode.h"

StudentNode::StudentNode(uint16_t sensorNode, char *name, int channel)
    : Node(channel, NODE_BASE + sensorNode), _sensorNode(sensorNode) {
        strcpy(_name, name);
    }

/// @brief Initializes the student node.
/// @details This function initializes the student node by setting up the RF24Network.
/// It also sends an ID request to the sensor node to get a new node ID.
void StudentNode::init()
{
    delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice

    log(F(": Initial node ID set to "), _node);
    setupRF24Network();
    uint16_t temp = _node;
    while (_node == temp) // wait until the node ID changes
    {
        log(F(": New ID request sent to "), _sensorNode);
        Node::sendPayload(_sensorNode, SELF_ID_REQUEST, _name);
        receivePayload();
        delay(ID_REQUEST_DELAY); // retry after 5s
    }
}

/// @brief Performs the essential operations for the student node.
/// @details This function sends a keep alive message to the sensor node and restarts the connection if the message fails.
void StudentNode::performEssentialOperations()
{
    sendKeepAlive(KEEP_ALIVE_INTERVAL);
    restart();
}

/// @brief Receives a payload from the sensor node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void StudentNode::receivePayload()
{
    network.update(); // Pump the network regularly
    while (network.available())
    {                             // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        network.peek(header);

        if (header.type == SELF_ID_REQUEST)
        {
            network.read(header, &_node, sizeof(_node));
            setupRF24Network();
            log(F(": New node ID received "), _node, F(" from "), header.from_node);
        }
        if (header.type == ID_REQUEST)
        {
            network.read(header, &nodeID, sizeof(nodeID));
            log(F(": Node ID received "), nodeID, F(" from "), header.from_node);
        }
        if (header.type == ALERT_REQUEST)
        {
            uint8_t buffer[3];
            network.read(header, &buffer, sizeof(buffer));
            Alert_Request temp = deserializeAlert(buffer);
            log(F(": Sensor alert received from "), header.from_node, F(" - [type: "), temp.type, F("; value: "), temp.value, F("]"));
        }
        // if (header.type == READINGS_REQUEST)
//         {
//             uint8_t buffer[NAME_LENGTH + 4];
//             network.read(header, &buffer, sizeof(buffer));
//             Sensor_Node temp = deserializeSensorNode(buffer);
//             log(F(": Sensor readings received from "), header.from_node, F(" - [temp: "), temp.temperature, F("; light: "), temp.phototransistor, F("]"));
//         }
        else
            break;
    }
}

// /// @brief Sends a readings request to the sensor node.
// void StudentNode::sendReadingsRequestToSensorNode()
// {
//     log(F(": Readings request sent to "), _sensorNode);
//     bool ok = Node::sendPayload(_sensorNode, READINGS_REQUEST, 0);
//     countFailedMessages = ok ? 0 : countFailedMessages + 1;
// }


/// @brief Sends an alert request to the sensor node.
/// @param type The type of the alert. It can be 'T' for temperature or 'L' for light.
/// @param value The value of the alert.
void StudentNode::sendAlertRequestToSensorNode(char type, int value)
{
    Alert_Request message;
    message.type = type;
    message.value = value;
    message.time = 0;

    uint8_t buffer[3];
    serializeAlert(message, buffer);

    log(F(": Alert activation sent to "), _sensorNode, F(" - [type: "), type, F("; value: "), value, F("]"));
    bool ok = Node::sendPayload(_sensorNode, ALERT_REQUEST, buffer);
    countFailedMessages = ok ? 0 : countFailedMessages + 1;
}

/// @brief Serializes an Alert_Request into a buffer.
/// @param temp The Alert_Request that is going to be serialized.
/// @param buffer The buffer it got from the Alert_Request
void StudentNode::serializeAlert(const Alert_Request &temp, uint8_t *buffer)
{
    buffer[0] = temp.type;

    buffer[1] = temp.value & 0xFF;
    buffer[2] = (temp.value >> 8) & 0xFF;

    buffer[3] = temp.time & 0xFF;
    buffer[4] = (temp.time >> 8) & 0xFF;
    buffer[5] = (temp.time >> 16) & 0xFF;
    buffer[6] = (temp.time >> 24) & 0xFF;
}

/// @brief Deserializes a buffer into an Alert_Request.
/// @param buffer The buffer that is going to be deserialized
/// @return The Alert_Request it got from the buffer
Alert_Request StudentNode::deserializeAlert(uint8_t *buffer)
{
    Alert_Request temp;
    temp.type = buffer[0];
    temp.value = buffer[1] | (buffer[2] << 8);
    temp.time = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16) | (buffer[6] << 24);
    return temp;
}

/// @brief Sends an alert deactivation to the sensor node.
void StudentNode::sendAlertDeactivationToSensorNode()
{
    log(F(": Alert deactivation sent to "), _sensorNode);
    bool ok = Node::sendPayload(_sensorNode, ALERT_DEACTIVATION, 0);
    countFailedMessages = ok ? 0 : countFailedMessages + 1;
}

/// @brief Gets the node ID of a node with a specific name.
/// @param name_pointer The name of the node.
/// @return The node ID of the node with the specific name.
uint16_t StudentNode::getNodeID(char *name_pointer)
{
    char name[NAME_LENGTH];
    strcpy(name, name_pointer);
    // log(F(": ID request sent to "), _sensorNode, F("(with name "), name, F(")"));
    bool ok = Node::sendPayload(_sensorNode, ID_REQUEST, name);
    countFailedMessages = ok ? 0 : countFailedMessages + 1;
    receivePayload();

    if (nodeID == 0) // If the node ID is not found, return
    {
        log(F(": Node ID not found"));
        return 0;
    }

    return nodeID;
}

/// @brief Sends a keep alive message to the sensor node at regular intervals.
/// @param interval The interval at which to send the keep alive message.
void StudentNode::sendKeepAlive(const unsigned long interval)
{
    unsigned long now = millis();
    if (now - last_sent_keep_alive >= interval)
    { // If it's time to send a message, send it!
        last_sent_keep_alive = now;
        log(F(": Keep alive sent to "), _sensorNode);
        bool ok = Node::sendPayload(_sensorNode, KEEP_ALIVE, 0);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
    }
}

/// @brief Restarts the node connection if the number of failed messages exceeds the maximum limit.
void StudentNode::restart()
{
    if (countFailedMessages >= MAX_FAILED_MESSAGES)
    {
        log(F(": Restarting node connection"));
        _node = NODE_BASE + _sensorNode;
        countFailedMessages = 0;
        init();
    }
}
