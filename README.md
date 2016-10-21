this is a simple web server for Arduino and Ethernet shield.
it accept command from HTTP request and transmit radio sigals to remote-controllabled devices, such as TV, air conditioner, etc.

Currently it support IR(38khz) and 315/433Mhz radio frequency. respectively, IRRemote and RCSwitch libraries are used.


The MQTT version need the lib from https://github.com/knolleary/pubsubclient.

Under Linux environment, install mqtt software(mosquitto) and observe the messages, once the arduino board is started, it will send message to mqtt server(broker) as below,

$ mosquitto_sub -t "#" -v
ds/request/to/yunos/from/tianhu-rc-1/1 online
ds/will/to/yunos/from/tianhu-rc-1/1 offline
ds/request/to/yunos/from/tianhu-rc-1/1 online
