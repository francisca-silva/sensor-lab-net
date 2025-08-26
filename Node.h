#ifndef NODE_H
#define NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const int RF24_PA_LEVEL = RF24_PA_HIGH;

#ifdef ESP32
const int RADIO_CE_PIN = 2;
const int RADIO_CSN_PIN = 4;
#else   // For Arduino or other boards
const int RADIO_CE_PIN = 7;
const int RADIO_CSN_PIN = 8;
#endif

const int NAME_LENGTH = 7;
const unsigned long INIT_DELAY = 2000;


// Header types for RF24Network
const char READINGS_REQUEST = 'R';      // 82
const char KEEP_ALIVE = 'K';            // 75
const char SEND_INFO = 'F';             // 70
const char PATH_INFO = 'P';             // 80
const char BEGIN_FLAG = 'B';            // 66
const char PAUSE_FLAG = 'A';            // 65

const char SELF_ID_REQUEST = 'N';       // 78
const char ID_REQUEST = 'I';            // 73


struct Sensor_Node
{
    char name[NAME_LENGTH];
};

struct Alert_Request {
    char type = '\0';
    int16_t value;
    long time;
};

class Node
{
public:
    /// @brief  Sends a payload to a specific node.
    /// @tparam T The type of the payload.
    /// @param to The node ID to send the payload to.
    /// @param type The type of the payload.
    /// @param payload The payload to send.
    /// @return True if the message is sent successfully, false otherwise.
    template <typename T>
    bool sendPayload(uint16_t to, char type, const T &payload)
    {
        // Make sure CE is LOW and CSN is HIGH before new commands
        digitalWrite(10, HIGH); // SS pin
        
        // If you powered it down, wake it up:
        radio.powerUp();
        delay(5); // t_pd2stby = 1.5ms minimum, 5ms is safe
        
        // If you disabled CE manually, set it back:
        radio.ce(HIGH);
        delayMicroseconds(150); // Allow radio to enter standby
        

        network.update(); // keep the network updated
        RF24NetworkHeader header(to, type);
        bool ok = network.write(header, &payload, sizeof(payload));
        // log(ok ? F(" (status = 1)") : F(" (status = 0)"));
        return ok;
    }

    /// @brief Logs a message to the serial monitor.
    /// @tparam ...Args This is a variadic template that accepts any number of arguments.
    /// @param ...args This is a parameter pack that accepts any number of arguments.
    template <typename... Args>
    void log(Args... args)
    {
        Serial.println();
        String info = String(millis()) + ": ";
        using expander = int[];
        (void)expander{0, (info += String(args), 0)...};
        Serial.print(info);
        Serial.flush();
    }

protected:
    RF24Network network;
    uint16_t _node;

    Node(int channel, uint16_t node);
    void setupRF24Network();

private:
    RF24 radio;
    int _channel;

    virtual void init() = 0;
    virtual void receivePayload() = 0;
};

#endif