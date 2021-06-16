#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BusIO_Register.h>
#include <Update.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <WiFiMulti.h>

WiFiClient client;
const char* SSID = "Kirschenman_2.4";
const char* PSWD = "Buddydog17";
long contentLength = 0;
bool isValidContentType = false;
String line;
uint16_t currentVersion;

WiFiMulti wifiMulti;
Adafruit_MCP23017 mcp[4];
//*****************************************************//
//                  CHESS STUFF                        //
//*****************************************************//
struct chessPiece
{
  String Name;
  byte pieceNumber;
  byte pieceType;
  byte x;
  byte y;
  bool Active;
  bool onBoard;
};

//*****************************************************//
//              AUTO UPDATE STUFF                      //
//*****************************************************//
String host = "chessprogram.s3-us-west-2.amazonaws.com"; // Host => bucket-name.s3.region.amazonaws.com
int port = 80; // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
String bin = "/firmware.bin"; // bin file name with a slash in front.
String version = "/version.txt"; // bin file name with a slash in front.


// Utility to extract header value from headers
String getHeaderValue(String header, String headerName) {
  return header.substring(strlen(headerName.c_str()));
}

void execOTA();

struct chessPiece piece[32];
void PieceToActive(chessPiece *piecePtr);
void LedToActive(chessPiece *piecePtr);


//*****************************************************//
//                NEOPIXEL STUFF                       //
//*****************************************************//
#define PIN 23
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 4, 2, 2, PIN,
  NEO_TILE_BOTTOM   + NEO_TILE_LEFT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRBW + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };
  void resetBoard(uint16_t whiteSpaceColor, uint16_t blackSpaceColor);
  uint16_t color1=0xd6ba, color2=0x0000;
//void LedSet();

//*****************************************************//
//                  TASK STUFF                         //
//*****************************************************//
void PollForEvent( void *pvParameters );
void EventTask( void *pvParameters );
static TaskHandle_t PollForEventHandle = 0;
static TaskHandle_t EventTaskHandle = 0;
static QueueHandle_t detection_queue;
static const uint8_t msg_queue_len = 5;
/////////////////////*****************************************************//
/////////////////////                            SETUP                    //
/////////////////////*****************************************************//
void setup() 
{
     Serial.begin(115200);
     delay(1000);  
/////////////////////*****************************************************//
/////////////////////                            Check for Updates        //
/////////////////////*****************************************************//
if (!EEPROM.begin(1000)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  currentVersion = EEPROM.readShort(0);
    wifiMulti.addAP("Kirschenman_2.4", "Buddydog17");
    wifiMulti.addAP("CenturyLink4841", "36e3b7u8eawa3y");



    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
 //Serial.println("Connecting to " + String(SSID));
  // Connect to provided SSID and PSWD
  //WiFi.begin(SSID, PSWD);
  // Wait for connection to establish
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); // Keep the serial monitor lit!
    delay(500);
  }

  // Connection Succeed
  Serial.println("");
  Serial.println("Connected to " + String(SSID));
 Serial.println("Connecting to: " + String(host));
  // Connect to S3
  if (client.connect(host.c_str(), port)) {
    // Connection Succeed.
    // Fecthing the bin
    Serial.println("Fetching Bin: " + String(version));

    // Get the contents of the bin file
    client.print(String("GET ") + version + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Cache-Control: no-cache\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client Timeout !");
        client.stop();
        return;
      }
    }
    
     while (client.available()) {
      // read line till /n
      line = client.readStringUntil('\n');
      
     }
     int uVersion=line.toInt();
     Serial.print("Cloud Version: "); Serial.println(uVersion);
     Serial.print("current Version: "); Serial.println(currentVersion);
    if (uVersion!=currentVersion)
    {

      Serial.print("Will Update");
      currentVersion=uVersion;
      EEPROM.writeShort(0, currentVersion);
      EEPROM.commit();
      execOTA();
    }
  }
      
    // Once the response is









 for (int i=0;i<32;i++) {piece[i].pieceNumber=i;}
for (int i=0;i<=7;i++) {piece[i].pieceType=0;}//white pawn
for (int i=16;i<=23;i++) {piece[i].pieceType=6;}//black pawn
for (int i=8;i<=9;i++) {piece[i].pieceType=1;}//white rook
for (int i=24;i<=25;i++) {piece[i].pieceType=1;}//black rook
for (int i=10;i<=11;i++) {piece[i].pieceType=2;}//white knight
for (int i=26;i<=27;i++) {piece[i].pieceType=2;}//black knight
for (int i=12;i<=13;i++) {piece[i].pieceType=3;}//white bishop
for (int i=28;i<=29;i++) {piece[i].pieceType=3;}//black bishop
piece[15].pieceType=5;//white king
piece[31].pieceType=5;//black king
piece[14].pieceType=4;//white queen
piece[30].pieceType=4;//black queen
Serial.print(piece[30].pieceType);Serial.print("\t"); Serial.print(piece[31].pieceType);Serial.print("\t");Serial.print(piece[10].pieceType);Serial.print("\t"); Serial.println(piece[9].pieceType);

//*****************************************************//
//                  CHESS SETUP                        //
//*****************************************************//
  mcp[0].begin();       // use default address 0
  mcp[1].begin(1);      // use default address 1
  mcp[2].begin(2);      // use default address 2
  mcp[3].begin(3);      // use default address 3
  
 for(int j=0; j<=3; j++){
 for (int i=0; i<15;i++){ //
  mcp[j].pinMode(i, INPUT);
  mcp[j].pullUp(i, HIGH);  // turn on a 100K pullup internally
 }
 }
//*****************************************************//
//                  MATRIX SETUP                       //
//*****************************************************//
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.drawPixel(0,0,0x4EE2);
  matrix.show();

//*****************************************************//
//                  TASK SETUP                         //
//*****************************************************//

xTaskCreatePinnedToCore(PollForEvent, "PollForEvent", 2048, NULL,2, &PollForEventHandle, 1);
xTaskCreatePinnedToCore(EventTask, "EventTask", 2048, NULL,2, &EventTaskHandle, 1);
detection_queue = xQueueCreate(msg_queue_len,sizeof(byte));
delay(50);
resetBoard(color1, color2);
}



void loop() 
{
vTaskDelete(NULL);
}

//hall effect is detected
//coordinates found
//figure out what piece is currently on that square
//set that piece active
//set leds based on what piece is active
//

void PollForEvent(void *pvParameters)
{

    //const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 500 );
    // -- Run forever...
    uint16_t quadrant[4];
    byte errorStat=0;
    byte pieceFlag=0;
    byte pieceFlagged=0;
    uint64_t pieceDetector, detectorCheck;

          quadrant[0]=mcp[0].readGPIOAB();
          quadrant[1]=mcp[1].readGPIOAB();
          quadrant[2]=mcp[2].readGPIOAB();
          quadrant[3]=mcp[3].readGPIOAB();
          pieceDetector= (uint64_t)(((uint64_t)quadrant[0])<< 48 | ((uint64_t)quadrant[1] << 32) | ((uint64_t)quadrant[2] << 16) | ((uint64_t)quadrant[3])); 
          detectorCheck=pieceDetector;
    for(;;) 
    {    
         
      quadrant[0]=mcp[0].readGPIOAB();
      quadrant[1]=mcp[1].readGPIOAB();
      quadrant[2]=mcp[2].readGPIOAB();
      quadrant[3]=mcp[3].readGPIOAB();

        //Serial.print(quadrant[0],BIN);Serial.print("\t");Serial.print(quadrant[1],BIN);Serial.print("\t");Serial.print(quadrant[2],BIN);Serial.print("\t");Serial.println(quadrant[3],BIN);
      pieceDetector=0;
      pieceDetector = (uint64_t)(((uint64_t)quadrant[0])<< 48 | ((uint64_t)quadrant[1] << 32) | ((uint64_t)quadrant[2] << 16) | ((uint64_t)quadrant[3])); 
          //piecenum,x,y,
          //2,4,5
          //'c' for clearboard
      if (Serial.available() > 0) 
      {
          // read the incoming byte:
          uint8_t vsize=Serial.available();
          Serial.println(vsize);
          char inArray[vsize];
          int value=0, value2=0, piecenumber=0;
          for (int i=0;i<vsize; i++)
          {  
            
            inArray[i] = Serial.read();
              
          }
            for(int i=0; i<vsize; i++)
              {Serial.print(i); Serial.print(": "); Serial.println(inArray[i]);}
            if (inArray[0]=='c') 
              resetBoard(color1,color2);
            else
            {
                for (int i=0;i<vsize-2; i++)
                { 
                  if (inArray[i]==','){value++; value2=0; }
                  else 
                  {
                    switch(value)
                    {
                      case 0://piecenumber
                        if (value2==1 || inArray[1]==',') {piecenumber+= inArray[i]-48; Serial.print("i: ");Serial.print(i);Serial.print(" ");Serial.print("piecenumber: ");Serial.println(piecenumber);}
                        else {piecenumber=(inArray[i]-48)*10;value2++;Serial.print("piecenumber: ");Serial.println(piecenumber);}
                        break;
                      case 1://xcoor
                        if (value2==0) {piece[piecenumber].x=inArray[i]-48;Serial.print("i: ");Serial.print(i);Serial.print(" ");Serial.print("x: ");Serial.println(inArray[i]);}
                        break;
                      case 2://ycoor
                        if (value2==0) {piece[piecenumber].y=inArray[i]-48;Serial.print("i: ");Serial.print(i);Serial.print(" ");Serial.print("y: ");Serial.println(inArray[i]);}
                        break;
                    }
                  }
                }
                  Serial.print("piecenumber: ");  Serial.println(piecenumber);
                  PieceToActive(&piece[piecenumber]);
                  LedToActive(&piece[piecenumber]);
            }
        }
          if (errorStat==1){
            if (pieceDetector==detectorCheck) errorStat=0;
          }
          else {
            pieceFlag=0;

            if (detectorCheck!=pieceDetector){//////Piece event detected

            for (int i=0; i<63; i++){
              if( bitRead(detectorCheck,i) != bitRead(pieceDetector,i)) {pieceFlag++; pieceFlagged=i;}
            }
            if (pieceFlag>1) {errorStat=1;}
            else{
              
              detectorCheck=pieceDetector;
            }

              //vTaskResume(EventTaskHandle);
              
            
            }/////////////////////////////////////////////////////////////

          }
     //Serial.print(pieceFlag); Serial.print("\t");Serial.print(errorStat); Serial.print("\t"); Serial.println(pieceFlagged);
           vTaskDelay(200 / portTICK_PERIOD_MS);               
    }

}



void EventTask (void *pvParameters)
{
      byte item;
   //   const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 500 );
    // -- Run forever...
    for(;;) { 

       // if (xQueueReceive(  detection_queue, (void *)&item,10)==pdTRUE){
       //   Serial.println(item);

       // }
            vTaskDelay(100 / portTICK_PERIOD_MS);    
    //        f[(packet.universe>> 8)-1]++;
    }//else {e++;Serial.print(f[0]);Serial.print("/");Serial.print(f[1]);Serial.print("/");Serial.print(f[2]);Serial.print("/");Serial.print(f[3]);Serial.print("/");Serial.print(f[4]);Serial.print("/");Serial.print(f[5]);Serial.print("/"); Serial.println(e);}
            
           
 }



void resetBoard(uint16_t whiteSpaceColor, uint16_t blackSpaceColor){

bool xboardColor=0;
  for(int i =0; i<=7; i++){
    for (int j = 0; j<=7; j++ ){
      if (xboardColor) matrix.drawPixel(i,j,whiteSpaceColor);
      else matrix.drawPixel(i,j,blackSpaceColor);
      xboardColor=!xboardColor;
    
    }
    xboardColor=!xboardColor;
  }
  matrix.show();
}


void PieceToActive(struct chessPiece *piecePtr ){
  piecePtr->Active=1;


  

}


void PieceToInactive(struct chessPiece *piecePtr){
  piecePtr->Active=0;
  resetBoard(color1,color2);


}

void LedToActive(struct chessPiece *piecePtr){
    int x=0, y=0;
  Serial.print(piecePtr->y);Serial.print("\t"); Serial.print(piecePtr->x);Serial.print("\t");Serial.print(piecePtr->pieceNumber);Serial.print("\t"); Serial.println(piecePtr->pieceType);
  switch (piecePtr->pieceType){
    case 0://white pawn
          matrix.drawPixel(piecePtr->x,piecePtr->y, 0xF800);  
          //potentialx=piecePtr->x;
          //potentialy=piecePtr->y+1;
          if (piecePtr->y<7) matrix.drawPixel(piecePtr->x,piecePtr->y+1, 0x07E0);
          if (piecePtr->y==1) matrix.drawPixel(piecePtr->x,piecePtr->y+2, 0x07E0);
          matrix.show();
      break;
    case 1://rook
          x=piecePtr->x; y=7-piecePtr->y;
          matrix.drawPixel(x,y, 0xF967);
          x=piecePtr->x; y=7-piecePtr->y;
          while (y <= 7) {y++; matrix.drawPixel(x,y, 0x07E0);}      //Up
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7) { x++; matrix.drawPixel(x,y, 0x07E0);}     //Right
          x=piecePtr->x; y=7-piecePtr->y;
          while (y >= 0) {y--; matrix.drawPixel(x,y, 0x07E0);}      //Down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0) { x--; matrix.drawPixel(x,y, 0x07E0);}     //Left
          matrix.show();
      break;
    case 2://knight
          x=piecePtr->x; y=7-piecePtr->y;
          matrix.drawPixel(x,y, 0xF967);

          if (x <= 7 && y <= 7) matrix.drawPixel(x+1,y+2, 0x07E0);    //Right up1
          if (x <= 7 && y <= 7) matrix.drawPixel(x+2,y+1, 0x07E0);    //Right up2
          if (x <= 7 && y >= 0) matrix.drawPixel(x+2,y-1, 0x07E0);    //Right down1
          if (x <= 7 && y >= 0) matrix.drawPixel(x+1,y-2, 0x07E0);    //Right down2
          if (x >= 0 && y >= 0) matrix.drawPixel(x-1,y-2, 0x07E0);    //Left down1
          if (x >= 0 && y >= 0) matrix.drawPixel(x-2,y-1, 0x07E0);    //Left down2
          if (x >= 0 && y <= 7) matrix.drawPixel(x-2,y+1, 0x07E0);    //Left up1
          if (x >= 0 && y <= 7) matrix.drawPixel(x-1,y+2, 0x07E0);    //Left up2

          matrix.show();
      break;
    case 3://bishop
           x=piecePtr->x; y=7-piecePtr->y;
          matrix.drawPixel(x,y, 0xF967);
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7 && y <= 7) {x++; y++; matrix.drawPixel(x,y, 0x07E0);}       //Right up
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7 && y >= 0) {x++; y--; matrix.drawPixel(x,y, 0x07E0);}        //Right down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0 && y >= 0) {x--; y--; matrix.drawPixel(x,y, 0x07E0);}       //Left down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0 && y <= 7) {x--; y++; matrix.drawPixel(x,y, 0x07E0);}       //Left up
          matrix.show();
      break;
 
    case 4://queen
          //Rook Qualities
          x=piecePtr->x; y=7-piecePtr->y;
          matrix.drawPixel(x,y, 0xF967);
          x=piecePtr->x; y=7-piecePtr->y;
          while (y <= 7) {y++; matrix.drawPixel(x,y, 0x07E0);}      //Up
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7) { x++; matrix.drawPixel(x,y, 0x07E0);}     //Right
          x=piecePtr->x; y=7-piecePtr->y;
          while (y >= 0) {y--; matrix.drawPixel(x,y, 0x07E0);}      //Down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0) { x--; matrix.drawPixel(x,y, 0x07E0);}     //Left

          //Biship Qualities
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7 && y <= 7) {x++; y++; matrix.drawPixel(x,y, 0x07E0);}       //Right up
          x=piecePtr->x; y=7-piecePtr->y;
          while (x <= 7 && y >= 0) {x++; y--; matrix.drawPixel(x,y, 0x07E0);}       //Right down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0 && y >= 0) {x--; y--; matrix.drawPixel(x,y, 0x07E0);}       //Left down
          x=piecePtr->x; y=7-piecePtr->y;
          while (x >= 0 && y <= 7) {x--; y++; matrix.drawPixel(x,y, 0x07E0);}       //Left up
          matrix.show();
      break;
    case 5://king
            x=piecePtr->x; y=7-piecePtr->y;
          matrix.drawPixel(x,y, 0xF967);
          x=piecePtr->x; y=7-piecePtr->y;
          if (y <= 7) {y++; matrix.drawPixel(x,y, 0x07E0);}      //Up
          x=piecePtr->x; y=7-piecePtr->y;
          if (x <= 7) { x++; matrix.drawPixel(x,y, 0x07E0);}     //Right
          x=piecePtr->x; y=7-piecePtr->y;
          if (y >= 0) {y--; matrix.drawPixel(x,y, 0x07E0);}      //Down
          x=piecePtr->x; y=7-piecePtr->y;
          if (x >= 0) { x--; matrix.drawPixel(x,y, 0x07E0);}     //Left

          //Biship Qualities
          x=piecePtr->x; y=7-piecePtr->y;
          if (x <= 7 && y <= 7) {x++; y++; matrix.drawPixel(x,y, 0x07E0);}       //Right up
          x=piecePtr->x; y=7-piecePtr->y;
          if (x <= 7 && y >= 0) {x++; y--; matrix.drawPixel(x,y, 0x07E0);}       //Right down
          x=piecePtr->x; y=7-piecePtr->y;
          if (x >= 0 && y >= 0) {x--; y--; matrix.drawPixel(x,y, 0x07E0);}       //Left down
          x=piecePtr->x; y=7-piecePtr->y;
          if (x >= 0 && y <= 7) {x--; y++; matrix.drawPixel(x,y, 0x07E0);}       //Left up
          matrix.show();
      break;
    case 6://black pawn
          matrix.drawPixel(piecePtr->x,piecePtr->y, 0xF800);
          if (piecePtr->y>0) matrix.drawPixel(piecePtr->x,piecePtr->y-1, 0x07E0);
          if (piecePtr->y==6) matrix.drawPixel(piecePtr->x,piecePtr->y-2, 0x07E0);
          matrix.show();
      break;
  }
}


// OTA Logic 
void execOTA() {
  Serial.println("Connecting to: " + String(host));
  // Connect to S3
  if (client.connect(host.c_str(), port)) {
    // Connection Succeed.
    // Fecthing the bin
    Serial.println("Fetching Bin: " + String(bin));

    // Get the contents of the bin file
    client.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Cache-Control: no-cache\r\n" +
                 "Connection: close\r\n\r\n");

    // Check what is being sent
    //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
    //                 "Host: " + host + "\r\n" +
    //                 "Cache-Control: no-cache\r\n" +
    //                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client Timeout !");
        client.stop();
        return;
      }
    }
    // Once the response is available,
    // check stuff

    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3
                                   
        {{BIN FILE CONTENTS}}

    */
    while (client.available()) {
      // read line till /n
      String line = client.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty,
      // this is end of headers
      // break the while and feed the
      // remaining `client` to the
      // Update.writeStream();
      if (!line.length()) {
        //headers ended
        break; // and get the OTA started
      }

      // Check if the HTTP Response is 200
      // else break and Exit Update
      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }

      // extract headers here
      // Start with content length
      if (line.startsWith("Content-Length: ")) {
        contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }

      // Next, the content type
      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {
    // Connect to S3 failed
    // May be try?
    // Probably a choppy network?
    Serial.println("Connection to " + String(host) + " failed. Please check your setup");
    // retry??
    // execOTA();
  }

  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType) {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);

    // If yes, begin
    if (canBegin) {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      size_t written = Update.writeStream(client);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
        // retry??
        // execOTA();
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      client.flush();
    }
  } else {
    Serial.println("There was no content in the response");
    client.flush();
  }
}



