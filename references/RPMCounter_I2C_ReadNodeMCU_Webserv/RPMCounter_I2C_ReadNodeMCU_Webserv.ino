
// This is a Master NodeMCU device that reads via I2C a RPM measurement
// done with an arduino or other IC on I2C adres 9
#include <Wire.h>
// NodeMCU file. This requires ESP8266WiFi.h to be installed.
// How to install:
//    Arduino IDE 1.6 or more
//    In Preferences add following URL in box for Additional Boards:
//             http://arduino.esp8266.com/stable/package_esp8266com_index.json 
//    Then, in Tools->Board, select the Board Manager. Scroll down and search 
//      esp8266 by esp8266 community bord, and click Install
// To actually program the NodeMCU, connect MicroUSB cable, set board to NodeMCU 1.0, 
//    and select correct port.
#include <ESP8266WiFi.h>

bool SERIAL_PRINT = false;
//serial connection baud rate
#define SERBAUD 115200

const byte SLAVE_ADDRESS = 9;  // RPM sensor slave device
const byte MESSAGE_LENGTH = 3;  // number of unsigned long of the message

// message we will obtain
// a union are variables which use the same memory !
union
 {
 unsigned long longData [MESSAGE_LENGTH];
 byte  rawData [MESSAGE_LENGTH * sizeof (long)];
 } myData;
 
unsigned long frqHz;
unsigned long frqRPM;
unsigned long periodRPM_micros;  //period from first tick to last tick according to gating time of the timer (around 25ms ??)
float correctedRPM;

const int sizelong = sizeof(long);

//Server Port Use 80 for http normal port.
#define SVRPORT 80   //9701

//the nodemcu is a station connecting to same netgear router as the Edison
// local wifi password if NodeMCU is a station connecting to your wifi
const char* ssid = "**********";          
const char* password = "*************";

// IP of the netgear is 192.168.1.1, we force a static IP for this station
IPAddress  stationIP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your

//different status and requests for the webserver
#define Get_RPM 1
#define INVALID_REQUEST 99

// Include API-Headers
extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "string.h"
#include "user_interface.h"
#include "cont.h"
#include "espconn.h"
#include "eagle_soc.h"
#include <pgmspace.h>
void * pvPortZalloc(int size,char *, int);

#ifndef __MEM_MANAGER_H__
#define __MEM_MANAGER_H__
#include "c_types.h"

/*------------------------±äÁ¿¶¨Òå------------------------*/

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#ifndef IOT_SIP_MODE
//#define configTOTAL_HEAP_SIZE      ( ( size_t ) ( 0x3fffc000 - (uint32)&_heap_start ) )//fix 16000 to 24000 on 14.2.26
#else
#define configTOTAL_HEAP_SIZE     ( ( size_t ) ( 8000 ) )
#endif
#define portBYTE_ALIGNMENT      8
#define pdFALSE 0
#define pdTRUE  1

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#if portBYTE_ALIGNMENT == 8
  #define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
  #define portBYTE_ALIGNMENT_MASK ( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
  #define portBYTE_ALIGNMENT_MASK ( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
  #define portBYTE_ALIGNMENT_MASK ( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
  #error "Invalid portBYTE_ALIGNMENT definition"
#endif

#define configUSE_MALLOC_FAILED_HOOK  1
#define portPOINTER_SIZE_TYPE unsigned int

#define heapMINIMUM_BLOCK_SIZE  ( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )

static unsigned char *ucHeap;

typedef struct A_BLOCK_LINK
{
  struct A_BLOCK_LINK *pxNextFreeBlock; //The next free block in the list.
  size_t xBlockSize;            //The size of the free block.
} xBlockLink;

static const unsigned short heapSTRUCT_SIZE = ( sizeof( xBlockLink ) + portBYTE_ALIGNMENT - ( sizeof( xBlockLink ) % portBYTE_ALIGNMENT ) );

static xBlockLink xStart, *pxEnd = NULL;

static void prvInsertBlockIntoFreeList( xBlockLink *pxBlockToInsert ) ;//ICACHE_FLASH_ATTR;

static void prvHeapInit( void ) ;//ICACHE_FLASH_ATTR;

void vApplicationMallocFailedHook( void ) ;//ICACHE_FLASH_ATTR;

// ESP BSP 2.1.0 conflict in mem.h void *pvPortMalloc( size_t xWantedSize ) ;//ICACHE_FLASH_ATTR;

// ESP BSP 2.1.0 conflict in mem.h void vPortFree( void *pv ) ;//ICACHE_FLASH_ATTR;

size_t xPortGetFreeHeapSize( void ) ;//ICACHE_FLASH_ATTR;

void vPortInitialiseBlocks( void ) ;//ICACHE_FLASH_ATTR;
/*-----------------------------------------------------------*/

#endif
}


//JSON string type
#define ONEJSON 0
#define FIRSTJSON 1
#define NEXTJSON 2
#define LASTJSON 3

#define URLSize 10
#define DATASize 10

typedef enum ProtocolType {
    GET = 0,
    POST,
    GET_FAVICON
} ProtocolType;

typedef struct URL_Param {
    enum ProtocolType Type;
    char pParam[URLSize][URLSize];
    char pParVal[URLSize][URLSize];
    int nPar;
} URL_Param;

long lastMsg = 0;
uint32_t state=0;
int initRx=0;
int stoprepub = 0;
char periodRPM_micros_s[20],correctedRPM_s[20],frqHz_s[20],frqRPM_s[20];
bool complete=false;
int lc=0;

/********************************************************
 * Local Function Prototypes
 ********************************************************/
void ArduinoWebServer_Init(void);
void ArduinoWebServer_Processor(void);
void ArduinoWebServer_KillClient(WiFiClient client, bool *busy);

//void util_printStatus(char * status, int s);
void util_startWIFI(void);

void jsonEncode(int pos, String * s, String key, String val);
void jsonAdd(String *s, String key,String val);

int Server_GetRequest(String payload);
String Server_ExecuteRequest(int request);

void ReadSensors(int interval);


/********************************************************
 * Utility Function Prototypes
 * Written by Dave St. Aubin.  Creative Commons license.
 ********************************************************/
float powr(float x, int y);
void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);

/********************************************************
 * Instantiate class objects
 ********************************************************/
WiFiServer server(SVRPORT);

WiFiClient espClient;             //Create Wifi Client object

void setupWiFi()
{
  // Connect to WiFi network
  if (SERIAL_PRINT) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  
  //clean up any old config that are still present
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(100);
  
  //set a static IP address
  WiFi.config(stationIP, gateway, subnet);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (SERIAL_PRINT) {
      Serial.print(".");
    }
  }
  
  if (SERIAL_PRINT) {
    Serial.println("");
    Serial.println("WiFi connected");
  }
}

/********************************************************
 * connect to local Wifi
 * Function: util_startWIFI(void)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * return       no return value
 ********************************************************/
void util_startWIFI(void) {
  //start the wifi
  setupWiFi();
}

void ArduinoWebServer_Init(void) {
  server.begin(); 
  if (SERIAL_PRINT) {
    Serial.print("Web Server initialized: Type = Arduino library\n");  
  }
}

/********************************************************
 * terminate TCP client connection
 * Function: ArduinoWebServer_KillClient(WiFiClient client, bool *busy)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * client       Wifi client object
 * busy         busy flag
 * return       no return value
 ********************************************************/
void ArduinoWebServer_KillClient(WiFiClient client, bool *busy) {
    lc=0;
    delay(1);
    client.flush();
    client.stop();
    complete=false;
    *busy = false;  
}

/********************************************************
 * Process http GET request using Arduino library
 * Function: ArduinoWebServer_Processor(void)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * return       no return value
 ********************************************************/
void ArduinoWebServer_Processor(void) {
    static bool busy=false;
    static int timeout_busy=0;
    int amux;

    //connect wifi if not connected
    if ( WiFi.status() != WL_CONNECTED) {
        delay(1);
        util_startWIFI();
        return;
    }
    //return if busy
    if(busy) {
        delay(1);
        if(timeout_busy++ >10000) {
            //util_printStatus((char *)" Status: Busy timeout-resetting..",-1);
            ESP.reset(); 
            busy = false;
        }
        return;
    }
    else {
        busy = true;
        timeout_busy=0;
    }
    delay(1);

    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
       busy = false;
       return;
    } 
    // Wait until the client sends some data
    while((!client.available())&&(timeout_busy++<5000)){
        delay(1);
        if(complete==true) {
            ArduinoWebServer_KillClient(client, &busy);
            return;
        }
    }
    //kill client if timeout
    if(timeout_busy>=5000) {
      ArduinoWebServer_KillClient(client, &busy);
      return;
    }
    if (SERIAL_PRINT) {
      Serial.println("Connection to http server made...");
    }
    complete=false; 
    ESP.wdtFeed(); 
    
    // Read the first line of the request
    String payld = client.readStringUntil('\r');
    client.flush();
    if (payld.indexOf("/favicon.ico") != -1) {
        client.stop();
        complete=true;
        busy = false;
        return;
    }
    if (SERIAL_PRINT) {
      Serial.print("Recv http: ");  
      Serial.println(payld);
    }
    delay(100);

    // Identify the request
    int request = Server_GetRequest(payld);
    if(request == INVALID_REQUEST) {
        client.stop();
        complete=true;
        busy = false;
    }
    client.flush();

    // Ignore if invalid
    if(request == INVALID_REQUEST) {
      if (SERIAL_PRINT) {
        Serial.println("Invalid Request");
      }
      return;    
    }
    // Execute request & get reply string
    payld = Server_ExecuteRequest(request);

    // Reply to Request
    client.print(payld);
    yield();

    delay(150);
    complete=true;
    busy = false;
    ESP.wdtFeed(); 
    ArduinoWebServer_KillClient(client, &busy);
}


/********************************************************
 * Utility Function Implementation 
 * Written by Dave St. Aubin.  Creative Commons license.
 ********************************************************/

/*******************************************************
 * Replaces the math.h pwr function since we cannot
 * successfully link to it with the ESP8266 Arduino IDE
 *******************************************************/
float powr(float x, int y)
{
    float temp;
    if( y == 0)
       return 1;
    temp = powr(x, y/2);
    if (y%2 == 0)
        return temp*temp;
    else
    {
        if(y > 0)
            return x*temp*temp;
        else
            return (temp*temp)/x;
    }
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
  if (isnan(n)) {
    res[0] = 'n';
    res[1] = 'a';
    res[2] = 'n';
    res[3] = '\0';
  } else {
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * powr(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
  }
}
// Converts an unsigned long number to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int ulongToStr(unsigned long x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}


/********************************************************
 * Read RPM sensor each time function is called
 * Function: void ReadSensors(int interval)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * interval     milliseconds (minimum) between reads
 * return       no return value
 ********************************************************/
void ReadSensors(int interval) {
    yield();
    long now = millis();                 
    if (now - lastMsg > interval) {  // Read 1 sensor every "interval" milliseconds or longer
        lastMsg = now;
    }
    else {
        return;
    }
    switch(state++) {
        case 0:
            //Read RPM data
            // send to slave device 9
            // request from 9 3 long of data
            if (Wire.requestFrom(9, 3*sizelong) == sizeof myData) {
              // valid response - read it
              for (int i = 0; i < sizeof myData; i++)
                myData.rawData [i] = Wire.read ();
          
              frqHz = myData.longData[0];
              frqRPM = myData.longData[1];
              periodRPM_micros = myData.longData[2];
              
              //computed corrected RPM which is a float:
              correctedRPM = periodRPM_micros / ((float) frqHz );
              correctedRPM = 1e6/correctedRPM *60;
            }
            // convert read longs to strings  
            ulongToStr(frqHz, frqHz_s, 5);
            ulongToStr(frqRPM, frqRPM_s, 5);
            ulongToStr(periodRPM_micros, periodRPM_micros_s, 5);
            ftoa(correctedRPM, correctedRPM_s, 4);
            if (SERIAL_PRINT) {
               Serial.print("Freq: ");
               Serial.print(frqHz);
               Serial.print("=  ");
               Serial.print(frqHz_s);
               Serial.print(", RPM: ");
               Serial.print(frqRPM);
               Serial.print("=  ");
               Serial.println(frqRPM_s);
               Serial.print(", corrected RPM: ");
               Serial.print(correctedRPM);
               Serial.print("=  ");
               Serial.print(correctedRPM_s);
               Serial.print(", period measured: ");
               Serial.print(periodRPM_micros);
               Serial.print("=  ");
               Serial.println(periodRPM_micros_s);
            }
            //last case, no extra sensors, reset state to 0
            state = 0;
            break;
        default:
            break;
    }
    ESP.wdtFeed(); 
    yield();
}

/********************************************************
 * add key/value entry into json string
 * Function: jsonAdd( 
 *                      String * s, 
 *                      String key, 
 *                      String val)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * *s           pointer to current json string
 * key          this json string key
 * val          this json string value
 * return       no return value
 ********************************************************/
void jsonAdd(String *s, String key,String val) {
    *s += '"' + key + '"' + ":" + '"' + val + '"';
}

/********************************************************
 * encode key/value entry into json string
 * Function: jsonEncode(int pos, 
 *                      String * s, 
 *                      String key, 
 *                      String val)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * pos          position of this json entry
 * *s           pointer to current json string
 * key          this json string key
 * val          this json string value
 * return       no return value
 ********************************************************/
void jsonEncode(int pos, String * s, String key, String val) {
    switch (pos) {
      case ONEJSON:      
      case FIRSTJSON:
        *s += "{\r\n";
        jsonAdd(s,key,val);
        *s+= (pos==ONEJSON) ? "\r\n}" : ",\r\n";
        break;
      case NEXTJSON:    
        jsonAdd(s,key,val);
        *s+= ",\r\n";
         break;
      case LASTJSON:    
        jsonAdd(s,key,val);
        *s+= "\r\n}";
        break;
    }
}

/********************************************************
 * Get Request from received data
 * Function: Server_GetRequest(String payload)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * payload      request string
 * return       request enumberation
 ********************************************************/
int Server_GetRequest(String payload) {
    int val;
    if (payload.indexOf("/?request=GetSensors") != -1) {
        val = Get_RPM;
    }  
    else {
        val = INVALID_REQUEST;
    }
    return val;
}

/********************************************************
 * Create request reply string to request
 * Function: Server_ExecuteRequest(int request)
 * 
 * Parameter    Description
 * ---------    ---------------------------------------
 * servertype   server type (SDK or LIB)
 * request      request enumberation
 * return       reply string
 ********************************************************/
String Server_ExecuteRequest(int request) {
    String s = "";  
    String v = "";
    // Prepare Response header
    s = "HTTP/1.1 200 OK\r\n";
    s += "Access-Control-Allow-Origin: *\r\n";
    ESP.wdtFeed();
    
    int value;
    switch (request) {
        case Get_RPM:
            //Create JSON return string
            s += "Content-Type: application/json\r\n\r\n";
            jsonEncode(FIRSTJSON,&s,"frqHz", frqHz_s);
            jsonEncode(NEXTJSON,&s,"frqRPM", frqRPM_s);
            jsonEncode(NEXTJSON,&s,"periodRPM_micros", periodRPM_micros_s);
            jsonEncode(NEXTJSON,&s,"correctedRPM", correctedRPM_s);
            v = system_get_free_heap_size();
            jsonEncode(NEXTJSON,&s,"SYS_Heap", v);
            v = millis()/1000;
            jsonEncode(LASTJSON,&s,"SYS_Time", v);            
            break;
        default:
            // Prepare the response for GPIO state
            s += "Content-Type: text/html\r\n\r\n";
            s += "<!DOCTYPE HTML>\r\n<html>\r\n ";
            s += "<br>NodeMCU working, obtain sensors with /?request=GetSensors <br>\r\n";
            s += "</html>\n";
            break;
   }
   return s;
}

void setup() {
  Wire.begin(D5, D6); // sda, scl on nodeMCU to D5 and D6 pin
  
  if (SERIAL_PRINT) {
    //Serial.begin(115200);
    Serial.begin(SERBAUD);                    // connect to the serial port
    delay(10);
    Serial.println("NodeMCU RPM measuring device");
  }
  
  util_startWIFI();                    // Connect to local Wifi

  ArduinoWebServer_Init();             // Start Arduino library based web server

  // Start the server
  server.begin();
  if (SERIAL_PRINT) {
    Serial.println("Server started");
   
    // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    
    Serial.print("ESP8266 WebServer Port: ");
    Serial.println(SVRPORT);
  }
}

void loop() {

    ArduinoWebServer_Processor();        // Service Arduino Library Web Server
    
    ReadSensors(100);      // Read  sensor every 100 milliseconds or longer (freqcounter is also per second...)

}
