/*
  A simple RCSwitch/Ethernet/Webserver demo
  
  http://code.google.com/p/rc-switch/
*/

#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>
#include <RCSwitch.h>

// Ethernet configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC Address
byte ip[] = { 192,168,0, 41 };                        // IP Address
EthernetServer server(80);                           // Server Port 80

int paramCount = 0;
char keys[8][32];
char values[8][32];

char printBuffer[128];

//page visit counter
int pvCount=0;

//IR 38k.
IRsend irsend;

// RCSwitch configuration
RCSwitch mySwitch = RCSwitch();
int RCTransmissionPin = 7;

// More to do...
// You should also modify the processCommand() and 
// httpResponseHome() functions to fit your needs.



/**
 * Setup
 */
void setup() {
  Serial.begin(9600);
  
  
  Ethernet.begin(mac, ip);
  server.begin();
  mySwitch.enableTransmit( RCTransmissionPin );
}

/**
 * Loop
 */
void loop() {
  char* command = httpServer();
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
			//printf("%i\n", i);
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
	int i = 0;
	for (i = 0; i < paramCount; i++) {
		sprintf(printBuffer,"%d:%s : %s\n", i, keys[i],values[i]);
            Serial.println(printBuffer);
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
/**
 * Command dispatcher
 */
int processCommand(char* url) {
  pvCount++;
  
  //no error
  int result = 0;
  
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
  //          irsend.sendNEC(code,bits);  
          sprintf(printBuffer,"send NEC code:%lu, bits: %d\n",code,bits);          
          Serial.println(printBuffer);    
        }else
          result = 4;//unknown code or bits
      }else
        result =3;//unknow type
    }
    else
      result = 2;//unknow command
  }
  else
    result = 1;//no command
  
  return result;
  
  /*
  if        (strcmp(command, "1-on") == 0) {
    mySwitch.switchOn(1,1);
  } else if (strcmp(command, "1-off") == 0) {
    mySwitch.switchOff(1,1);
  } else if (strcmp(command, "2-on") == 0) {
    mySwitch.switchOn(1,2);
  } else if (strcmp(command, "2-off") == 0) {
    mySwitch.switchOff(1,2);
  }
  */
}

/**
 * HTTP Response with homepage
 */
void httpResponseHome(EthernetClient c) {
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/html");
  c.println();
  c.println("<html>");
  c.println("<head>");
  c.println(    "<title>RCSwitch Webserver Demo</title>");
  c.println(    "<style>");
  c.println(        "body { font-family: Arial, sans-serif; font-size:12px; }");
  c.println(    "</style>");
  c.println("</head>");
  c.println("<body>");
  c.println(    "<h1>RCSwitch Webserver Demo</h1>");
  c.println(    "<ul>");
  c.println(        "<li><a href=\"./?1-on\">Switch #1 on</a></li>");
  c.println(        "<li><a href=\"./?1-off\">Switch #1 off</a></li>");
  c.println(    "</ul>");
  c.println(    "<ul>");
  c.println(        "<li><a href=\"./?2-on\">Switch #2 on</a></li>");
  c.println(        "<li><a href=\"./?2-off\">Switch #2 off</a></li>");
  c.println(    "</ul>");
  c.println(    "<hr>");
  c.println(    "<a href=\"http://code.google.com/p/rc-switch/\">http://code.google.com/p/rc-switch/</a>");
  c.println("</body>");
  c.println("</html>");
}

void httpResponse(EthernetClient c, int result) {
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/html");
  c.println();
  c.println("<html>");
  c.println("<head>");
  c.println(    "<title>RCSwitch Webserver Demo</title>");
  c.println(    "<style>");
  c.println(        "body { font-family: Arial, sans-serif; font-size:12px; }");
  c.println(    "</style>");
  c.println("</head>");
  c.println("<body>");
  c.println(    "<p>");
  c.println(result);
  c.println("</p>");
  
  //c.println(    "<p>visit counter:");
  //c.println(pvCount);
  //c.println("</p>");
  c.println("</body>");
  c.println("</html>");
}

/**
 * HTTP Redirect to homepage
 */
void httpResponseRedirect(EthernetClient c) {
  c.println("HTTP/1.1 301 Found");
  c.println("Location: /");
  c.println();
}

/**
 * HTTP Response 414 error
 * Command must not be longer than 30 characters
 **/
void httpResponse414(EthernetClient c) {
  c.println("HTTP/1.1 414 Request URI too long");
  c.println("Content-Type: text/plain");
  c.println();
  c.println("414 Request URI too long");
}

/**
 * Process HTTP requests, parse first request header line and 
 * call processCommand with GET query string (everything after
 * the ? question mark in the URL).
 */
char*  httpServer() {
  EthernetClient client = server.available();
  if (client) {
    char sReturnCommand[32];
    int nCommandPos=-1;
    sReturnCommand[0] = '\0';
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if ((c == '\n') || (c == ' ' && nCommandPos>-1)) {
          sReturnCommand[nCommandPos] = '\0';
          if (strcmp(sReturnCommand, "\0") == 0) {
            httpResponseHome(client);
          } else {
            int result = processCommand(sReturnCommand);
            //httpResponseRedirect(client);
            httpResponse(client, result);
          }
          break;
        }
        if (nCommandPos>-1) {
          sReturnCommand[nCommandPos++] = c;
        }
        if (c == '?' && nCommandPos == -1) {
          nCommandPos = 0;
        }
      }
      if (nCommandPos > 128) {
        httpResponse414(client);
        sReturnCommand[0] = '\0';
        break;
      }
    }
    if (nCommandPos!=-1) {
      sReturnCommand[nCommandPos] = '\0';
    }
    // give the web browser time to receive the data
    delay(1);
    client.stop();
    
    return sReturnCommand;
  }
  return '\0';
}
