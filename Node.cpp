#include "Node.h"

Node::Node(int channel, uint16_t node)
    : radio(RADIO_CE_PIN, RADIO_CSN_PIN), network(radio), _channel(channel), _node(node) {}

/// @brief Sets up the RF24Network for the main node.
/// @details This function initializes the SPI and the radio hardware.
void Node::setupRF24Network()
{
    SPI.begin();
    if (!radio.begin())
    {
        log(F("Radio hardware not responding!"));
        while (1)
        {
            // hold in infinite loop
        }
    }
    radio.setPALevel(RF24_PA_LEVEL);
    radio.setChannel(_channel);
    network.begin(_node);
}