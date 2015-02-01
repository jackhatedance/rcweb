/*
 Publishing in the callback 
 
  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"
  
  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the 
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.
  
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03 };
byte server[] = { 192, 168, 0, 210 };
char* serverName = "snapshot-driverstack-com.dingjianghao.home";

char* clientId = "rc-mqtt-jack";
char* requestToServerTopic = "request/to/yunos/from/123/1";
char* responseToServerTopic = "response/to/yunos/from/123/1";
char* subTopic = "request/to/123/from/+/+";

//last will
char* willTopic = "will/to/yunos/from/123/1";
char* willMessage = "offline";

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(serverName, 1883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  
  // Allocate the correct amount of memory for the payload copy
  char* result="true";  
  client.publish(responseToServerTopic, result);
  Serial.print("received topic:");
  Serial.println(topic);
  
  //Serial.print(p); 
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac);
   // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  
  if (client.connect(clientId, willTopic, 1, 0, willMessage)) {
    client.publish(requestToServerTopic,"online");
    client.subscribe(subTopic);
  }
}

void loop()
{
  client.loop();
}
