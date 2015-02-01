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

//IR and RC
#include <IRremote.h>
#include <RCSwitch.h>

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

//URL parameters
int paramCount = 0;
int maxParamSize = 6;
int maxParamLength = 20;
char keys[6][16];
char** values;

//execution result
char processMessage[10]="ok";
//raw data of IR signal
//unsigned int irrawCodes[RAWBUF]; // The durations if raw
unsigned int* irrawCodes;
int rawCodeLen; // The length of the code

//IR 38k.
IRsend irsend;

RCSwitch mySwitch = RCSwitch();

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
  char* message = (char*)malloc(length+1);
  // Copy the payload to the new buffer
  memcpy(message,payload,length);
  message[length]='\0';
  
  int processResult = processCommand(message);
  
  char* result="fail";  
  if(processResult==0)
    result="ok";
    
  client.publish(responseToServerTopic, result);
 
  
  free(message);
  //Serial.print(p); 
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  if (client.connect(clientId, willTopic, 1, 0, willMessage)) {
    client.publish(requestToServerTopic,"online");
    client.subscribe(subTopic);
  }
    
  //init array  
  values = (char**)malloc(maxParamSize * sizeof(char*));
  for (int i = 0; i < maxParamSize-1; i++ )
  {
    values[i] = (char*) malloc(maxParamLength * sizeof(char));
  }
  //only the last param is extra long. (Use extra memory only when required.) it is for IR raw data.
  values[maxParamSize-1] = (char*) malloc(20 * sizeof(char));
  
  //irrawCodes = (unsigned int*) malloc(50 * sizeof(unsigned int));
  
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);
}

void loop()
{
  client.loop();
}


void parseUrl(char* s) {
	int i = 0;
	int paramIndex = 0;
	char* currentString;
	int charIndex = 0;

	char c;

	int urlLen = strlen(s);

	currentString = keys[paramIndex];
	while (i < urlLen) {

		c = s[i];
		if (c == '&') {

			//end the current string
			currentString[charIndex] = '\0';

			//go to next param
			paramIndex++;
			charIndex = 0;
			i++;

			currentString = keys[paramIndex];
			
		} else {

			if (c == '=') {
                                currentString[charIndex]='\0';
				//key to value
				currentString = values[paramIndex];
				charIndex = 0;
			} else {
				currentString[charIndex] = c;

				charIndex++;
			}

			i++;

		}
	}
        //append the last string terminator
        currentString[charIndex] = '\0';

	paramCount = paramIndex + 1;


}

void printParam() {
        Serial.println("printParam:");
  
	int i;

	for (i = 0; i < paramCount; i++) {
        Serial.print(keys[i]);
    Serial.print(":");
  Serial.println(values[i]);
		
	}

}

int getKeyIndex(const char * key){
	int i;
	for(i=0;i<paramCount;i++)
	{
		if(strcmp(key,keys[i])==0)
			return i;
	}

	return -1;
}

char* getValueByKey(const char * key){
	int idx = getKeyIndex(key);
	if(idx!=-1)
		return values[idx];
	else
		return NULL;
}

/*
convert raw data from URL to uint array
*/
void processIRRawData(char* rawDataStr)
{
  Serial.println("start process");
  //Serial.println(rawDataStr);
  
  int i=0;
  int j=0;
  rawCodeLen=0;
  char num[6];
  char c;
  int dataStrlen = strlen(rawDataStr); 
  while(true)
  {
    c = rawDataStr[i];
    //Serial.println(c);
    
    if(c==',' || c=='\0')
    {
      num[j]='\0';      
      
      unsigned int i_num = atoi(num);

      
      //Serial.print(rawCodeLen);
      //Serial.print(":");
      irrawCodes[rawCodeLen]=i_num;
      Serial.print(irrawCodes[rawCodeLen],DEC);      
      Serial.print(",");
      rawCodeLen++;      
      
      j=0;
      
      if(c=='\0' )
      {
         break; 
      }
    }
    else
    {
      num[j] = c;
      j++;      
    }   
    
    i++;
  }
  Serial.print(". codeLen:");  
  Serial.println(rawCodeLen);
  for(int ii=0;ii<rawCodeLen;ii++)
          {
            Serial.print(irrawCodes[ii],DEC);
            Serial.print(" ");            
          }
}

/**
 * Command dispatcher
 */
int processCommand(char* url) {
  
  //no error
  int result = 0;
  
  Serial.print("url:");
  Serial.println(url);
    
  parseUrl(url); 
  
  
  printParam();
  
//RC type: 38k,315m,433m.
  char* cmd = getValueByKey("cmd");
  
  if(cmd!=NULL)
  {
  
    if(strcmp(cmd,"IRRemote")==0)
    {
      char* type = getValueByKey("type");      
      if(type!=NULL && strcmp(type,"NEC")==0)
      {
        char* codeStr = getValueByKey("code");
        char* bitsStr = getValueByKey("bits");
  
        if(codeStr!=NULL && bitsStr!=NULL)
        {
          unsigned long code = strtoul (codeStr, NULL, 0);
          int bits= atoi(bitsStr);
            irsend.sendNEC(code,bits);  
          //sprintf(printBuffer,"send NEC code:%lu, bits: %d\n",code,bits);          
          //Serial.println(printBuffer);    
        }else
        {
          result = 1;
          //mystrcpy(processMessage, "unknown code or bits");
        }
      }
      else if(type!=NULL && strcmp(type,"Sharp")==0)
      {
        char* codeStr = getValueByKey("code");
        char* bitsStr = getValueByKey("bits");
        char* repeatStr = getValueByKey("repeat");        
  
        int repeat=0;
        if(repeatStr!=NULL)
        {
          repeat= atoi(repeatStr);
        }
        
        if(codeStr!=NULL && bitsStr!=NULL)
        {
          unsigned long code = strtoul (codeStr, NULL, 0);
          int bits= atoi(bitsStr);
          
          for(int i=0;i<repeat+1;i++)
            irsend.sendSharp(code,bits);  
          //sprintf(printBuffer,"send NEC code:%lu, bits: %d\n",code,bits);          
          //Serial.println(printBuffer);    
        }else
        {
          result = 1;
          //mystrcpy(processMessage, "unknown code or bits");
        }
      }else if(type!=NULL && strcmp(type,"raw")==0)
      {
        char* frequencyStr = getValueByKey("freq");
        char* codeStr = getValueByKey("code");
        if(frequencyStr!=NULL && codeStr!=NULL)
        {
          int frequency= atoi(frequencyStr);
          processIRRawData(codeStr);
          Serial.print("send raw IR, ");
          //Serial.print("codeLen");
          //Serial.print(rawCodeLen);
          //Serial.print(frequency);
          /*
          for(int ii=0;ii<rawCodeLen;ii++)
          {
            Serial.print(irrawCodes[ii],DEC);
            Serial.print(" ");            
          }
          */
          //irsend.sendRaw(irrawCodes, rawCodeLen, frequency);          
        }else
        {
          result = 1;          
        }  

      }
      else{
         result = 1;
         //mystrcpy(processMessage, "unknow type");
      }
    }
    else if(strcmp(cmd,"RCSwitch")==0)
    {
      char* pulseLengthStr = getValueByKey("pulseLength");
      char* codeStr = getValueByKey("code");
      char* bitsStr = getValueByKey("bits");

      if(pulseLengthStr!=NULL && codeStr!=NULL && bitsStr!=NULL)
      {
        int pulseLength= atoi(pulseLengthStr);
        unsigned long code = strtoul (codeStr, NULL, 0);
        int bits= atoi(bitsStr);
        
        mySwitch.setPulseLength(pulseLength);
        mySwitch.send(code,bits);  
        //sprintf(printBuffer,"send NEC code:%lu, bits: %d\n",code,bits);          
        //Serial.println(printBuffer);    
      }else
      {
        result = 1;
        //mystrcpy(processMessage, "unknown code or bits");
      }
      
    }
    else if(strcmp(cmd,"isOn")==0)
    {
      //ok
    }
    else{    
      result = 1;
      //mystrcpy(processMessage, "unknow command");
    }
  }
  else
  {
    result = 1;
    //mystrcpy(processMessage, "no command");
  }
  return result;
  

}

void mystrcpy(char* dest, char* src)
{
  int i=0;
  while(true)
  {
     char c = src[i];
     dest[i] = c;
     
     if(c=='\0')
       break;     
    
    i++;
  }
  
}


