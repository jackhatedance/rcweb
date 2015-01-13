/*

  a MQTT version of RC device.
  
*/

#include <SPI.h>
#include <Ethernet.h>

//from https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03 };
//byte server[] = { 192, 168, 0, 210 };
char* server = "qa-driverstack-com";

char* onlineTopic = "/online";

//a unique device ID on target server
char* DEVICE_ID = "jack-rc-mqtt-1";



// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  
  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length+1);
  p[length]=0;
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  client.publish("outTopic", p, length);
  Serial.print("topic:");
  Serial.println(topic);
  
  Serial.print("message:");
  Serial.println((char*)p);
  
  // Free the memory
  free(p);
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac);
  if (client.connect("arduinoClient")) {
    client.publish("/hello","hello world");
    client.subscribe("inTopic");
  }
}

void loop()
{
  client.loop();
}

