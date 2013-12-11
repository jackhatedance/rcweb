/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };
//IPAddress ip(192,168,0, 177);


int paramCount = 0;
char keys[8][32];
char values[8][32];

char processMessage[64]="okvghfjghjhgjhghjghjghg";


// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);





//IR 38k.
IRsend irsend;


void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  httpServer();
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
/**
 * Command dispatcher
 */
int processCommand(char* url) {
  
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
            irsend.sendNEC(code,bits);  
          //sprintf(printBuffer,"send NEC code:%lu, bits: %d\n",code,bits);          
          //Serial.println(printBuffer);    
        }else
        {
          result = 1;
          //mystrcpy(processMessage, "unknown code or bits");
        }
      }else{
         result = 1;
         //mystrcpy(processMessage, "unknow type");
      }
    }
    else{    
      result = 1;
      //mystrcpy(processMessage, "unknow command\n");
    }
  }
  else
  {
    result = 1;
    //mystrcpy(processMessage, "no command");
  }
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

void mystrcpy(char* dest, char* src)
{
  int i=0;
  for(i=0;i<sizeof(dest);i++)
  {
     char c = src[i];
     dest[i] = c;
     
     if(c=='\0' || c=='\n')
     {
       dest[i+1]='\0';
       break;     
     }
  }
  
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

  c.println(    "<hr>");
  c.println(    "<a href=\"http://code.google.com/p/rc-switch/\">http://code.google.com/p/rc-switch/</a>");
  c.println("</body>");
  c.println("</html>");
}

void httpResponse(EthernetClient c, char* msg) {
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
  c.println(msg);
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
void httpServer() {
  
  EthernetClient client = server.available();
  
  if (client) {
    char sReturnCommand[128+1];
    int nCommandPos=-1;
    sReturnCommand[0] = '\0';
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
        if ((c == '\n') || (c == ' ' && nCommandPos>-1)) {
          sReturnCommand[nCommandPos] = '\0';
          if (strcmp(sReturnCommand, "\0") == 0) {
            httpResponseHome(client);
          } else {
            int result = processCommand(sReturnCommand);
            Serial.print("result:");
            Serial.println(result);
            //httpResponseRedirect(client);
            if(result==0)
              httpResponse(client, "OK");
            else
              httpResponse(client, "error");
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
    
    return;
  }
 
}
