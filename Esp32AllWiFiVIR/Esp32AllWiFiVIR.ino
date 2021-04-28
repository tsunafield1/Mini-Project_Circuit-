/*
 * Mic POT 2.4k
 * LDR POT 4.7k
 * IR POT 12k
 */
#include <WiFi.h>
const char* ssid = "AndroidAPF3F1";     // WiFi name
const char* password = "kckm7267";      // WiFi password
const char* AP_ssid = "ESP32 AP";       // Hotspot name
const char* AP_password = "pw123456";   // Hotspot pw
bool wifi = 1;

WiFiServer server(80);

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 1500;
String header;

#define outIR 32
#define inIR 39
#define inLDR 34
#define outMIC 33
#define inMIC 35
#define outTRAN 27
#define butMode 26
#define butState 25
unsigned long changeModeTime = millis();
unsigned long changeStateTime = millis();
bool state = 0; 
bool segment = 0;
int mode = 0;
String MODE[4] = {"IR","LDR","MIC","BUTTON"};
String STATE[2] = {"Off","On"};

int segmentA1 = 15;
int segmentB1 = 2;
int segmentC1 = 4;
int segmentD1 = 5;
int segmentE1 = 18;
int segmentG1 = 19;

int segmentA2 = 21;
int segmentD2 = 22;
int segmentE2 = 23;
int segmentF2 = 13;

int D1 = 12;
int D2 = 14;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(inLDR,INPUT);
  pinMode(inIR,INPUT);
  pinMode(inMIC,INPUT);
  pinMode(butMode,INPUT_PULLUP);
  pinMode(butState,INPUT_PULLUP);
  pinMode(outIR,OUTPUT);
  pinMode(outMIC,OUTPUT);
  pinMode(outTRAN,OUTPUT);

  pinMode(segmentA1, OUTPUT);
  pinMode(segmentB1, OUTPUT);
  pinMode(segmentC1, OUTPUT);
  pinMode(segmentD1, OUTPUT);
  pinMode(segmentE1, OUTPUT);
  pinMode(segmentG1, OUTPUT);

  pinMode(segmentA2, OUTPUT);
  pinMode(segmentD2, OUTPUT);
  pinMode(segmentE2, OUTPUT);
  pinMode(segmentF2, OUTPUT);
  
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);

  WiFi.mode(WIFI_AP_STA);
  delay(100);
  WiFi.softAP(AP_ssid,AP_password);
  Serial.println();
  Serial.print("AP ID : ");
  Serial.println(AP_ssid);
  Serial.print("AP PW : ");
  Serial.println(AP_password);
  Serial.println("AP IP Address");
  Serial.println(WiFi.softAPIP());
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if(digitalRead(butState) == 0 || digitalRead(butMode) == 0)
    {
      wifi = 0;
      Serial.println("\nCancel to connect WiFi...");
      break;
    }
  }
  if(wifi == 1)
  {
    Serial.println("");
    Serial.println("Wifi Connected");
    Serial.println("IP address:");
    Serial.println(WiFi.localIP());
  }
  server.begin();
  Serial.println("Server has been started...");
  changeModeTime = millis() + 1000;
}

void loop() {
  // put your main code here, to run repeatedly:
  MAIN();
  WiFiClient client = server.available();
  if(client)
  {
    currentTime = millis();
    previousTime = currentTime;
    //Serial.println("New Client.");
    String currentLine = "";
    while(client.connected() && currentTime - previousTime <= timeoutTime)
    {
      MAIN();
      currentTime = millis();
      if(client.available())
      {
        char c = client.read();
        //Serial.write(c);
        header += c;
        if(c == '\n')
        {
          if(currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
          }
          if(header.indexOf("GET /0/mode") >= 0)
          {
            changeMode(0);
          }
          else if(header.indexOf("GET /1/mode") >= 0)
          {
            changeMode(1);
          }
          else if(header.indexOf("GET /2/mode") >= 0)
          {
            changeMode(2);
          }
          else if(header.indexOf("GET /3/mode") >= 0)
          {
            changeMode(3);
          }
          if(header.indexOf("GET /1/on") >= 0)
          {
            if(state == 0)changeState();
          }
          else if(header.indexOf("GET /0/off") >= 0)
          {
            if(state == 1)changeState();
          }
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
            
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          client.println(".button { background-color: #5AFF52; border: none; color: black; padding: 16px 40px;");
          client.println("text-decoration: none; font-size: 40px; margin: 2px; cursor: pointer;}</style></head>");
           
          
          client.println("<body><BODY BGCOLOR=#FFFFE0><h1>Circuit Sec17 G1</h1>");

          client.println("<FONT SIZE=5><B>Mode : " + MODE[mode] + "</B></FONT>");
          client.println("<p><a href=\"/1/mode\"><button class=\"button\">LDR</button></a>");
          client.println("<p><a href=\"/0/mode\"><button class=\"button\">IR</button></a>");
          client.println("<a href=\"/2/mode\"><button class=\"button\">MIC</button></a></p>");
          client.println("<p><a href=\"/3/mode\"><button class=\"button\">BUTTON</button></a></p>");
          client.println("<FONT SIZE=5><B>State : " + STATE[state] + "</B></FONT>");
          if(mode == 3 || mode == 2 || mode == 0)
          {
            client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a>");
            client.println("<a href=\"/0/off\"><button class=\"button\">OFF</button></a></p>");
          }
          client.println("<p><a href=\"/99/refresh\"><button class=\"button\">REFRESH</button></a></p>");
          client.println("</body></html>");
          client.println();
          break;
        }
        else
        {
          currentLine = "";
        }
        if(c != '\r')
        {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    /*Serial.println("Client disconnected.");
    Serial.println("");*/    
  }
}

void MAIN()
{
  if(digitalRead(butMode) == 0)
  {
    changeMode();
  }
  switch(mode){
    case 0: //IR
    {
      digitalWrite(outIR,HIGH);
      digitalWrite(outMIC,LOW);
      if(digitalRead(inIR) == 1) 
      {
        changeState();
      }
      break;
    }
    case 1: //LDR
    {
      digitalWrite(outIR,LOW);
      digitalWrite(outMIC,LOW);
      if(digitalRead(inLDR) == 1) state = 1;
      else state = 0;
      break;
    }
    case 2: //MIC
    {
      digitalWrite(outIR,LOW);
      digitalWrite(outMIC,HIGH);
      if(digitalRead(inMIC) == 1) 
      {
        changeState();
      }
      break;
    }
    case 3: //BUTTON
    {
      digitalWrite(outIR,LOW);
      digitalWrite(outMIC,LOW);
      if(digitalRead(butState) == 0)
      {
        changeState();
      }
      break;
    }
  }
  if(state == 0) digitalWrite(outTRAN,LOW);
  else digitalWrite(outTRAN,HIGH);
  showSegment();
}

void changeState()
{
  if(diff(millis(), changeStateTime) > 500)
  {
    changeStateTime = millis();
    if(state == 0) state = 1;
    else state = 0;
    Serial.println("State " + STATE[state]); 
  }
}

void changeMode()
{
  if(diff(millis(),changeModeTime) > 500)
  {
    Serial.print("ChangeMode to ");
    state = 0;
    changeModeTime = millis();
    mode++;
    if(mode > 3) mode = 0;
    Serial.print(mode);
    Serial.println(" " + MODE[mode]);
    changeStateTime = millis() + 500;
  }
}

void changeMode(int newMode)
{
  if(diff(millis(),changeModeTime) > 500 && newMode != mode)
  {
    Serial.print("ChangeMode to ");
    state = 0;
    changeModeTime = millis();
    mode = newMode;
    Serial.print(mode);
    Serial.println(" " + MODE[mode]);
    changeStateTime = millis() + 500;
  }
}

void displayNumber(int d,int digit)
{
  displaySegment(digit,d); // 1 digit on 7 segments
    switch(d)
    {
      case 1:
      digitalWrite(D1, HIGH);
      break;
      
      case 2:
      digitalWrite(D2, HIGH);
      break;
    }
    delay(1);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
}

void displaySegment(int numberToDisplay,int d)
{
  if( d == 1){
    switch (numberToDisplay)
    {
      case 0:
      digitalWrite(segmentA1, LOW);
      digitalWrite(segmentB1, LOW);
      digitalWrite(segmentC1, LOW);
      digitalWrite(segmentD1, LOW);
      digitalWrite(segmentE1, LOW);
      digitalWrite(segmentG1, HIGH);
      
      digitalWrite(segmentA2, LOW);
      digitalWrite(segmentD2, LOW);
      digitalWrite(segmentE2, LOW);
      digitalWrite(segmentF2, LOW);
      break;
      
      case 1:  // show 1
      digitalWrite(segmentA1, LOW);
      digitalWrite(segmentB1, HIGH);
      digitalWrite(segmentC1, HIGH);
      digitalWrite(segmentD1, LOW);
      digitalWrite(segmentE1, LOW);
      digitalWrite(segmentG1, LOW);
      
      digitalWrite(segmentA2, LOW);
      digitalWrite(segmentD2, LOW);
      digitalWrite(segmentE2, LOW);
      digitalWrite(segmentF2, LOW);
      break;
    
      case 2:  // show 2
      digitalWrite(segmentA1, HIGH);
      digitalWrite(segmentB1, HIGH);
      digitalWrite(segmentC1, LOW);
      digitalWrite(segmentD1, HIGH);
      digitalWrite(segmentE1, HIGH);
      digitalWrite(segmentG1, HIGH);
      
      digitalWrite(segmentA2, LOW);
      digitalWrite(segmentD2, LOW);
      digitalWrite(segmentE2, LOW);
      digitalWrite(segmentF2, LOW);
      break;
    
      case 3:  // show 3
      digitalWrite(segmentA1, HIGH);
      digitalWrite(segmentB1, HIGH);
      digitalWrite(segmentC1, HIGH);
      digitalWrite(segmentD1, HIGH);
      digitalWrite(segmentE1, LOW);
      digitalWrite(segmentG1, HIGH);

      digitalWrite(segmentA2, LOW);
      digitalWrite(segmentD2, LOW);
      digitalWrite(segmentE2, LOW);
      digitalWrite(segmentF2, LOW);
      break;
    }
  }
  else switch(numberToDisplay)
  {
    case 0:
    digitalWrite(segmentA1, LOW);
    digitalWrite(segmentB1, HIGH);
    digitalWrite(segmentC1, HIGH);
    digitalWrite(segmentD1, LOW);
    digitalWrite(segmentE1, LOW);
    digitalWrite(segmentG1, LOW);

    digitalWrite(segmentA2, HIGH);
    digitalWrite(segmentD2, HIGH);
    digitalWrite(segmentE2, HIGH);
    digitalWrite(segmentF2, HIGH);
    break;

    case 1:
    digitalWrite(segmentA1, LOW);
    digitalWrite(segmentB1, HIGH);
    digitalWrite(segmentC1, HIGH);
    digitalWrite(segmentD1, LOW);
    digitalWrite(segmentE1, LOW);
    digitalWrite(segmentG1, LOW);

    digitalWrite(segmentA2, LOW);
    digitalWrite(segmentD2, LOW);
    digitalWrite(segmentE2, LOW);
    digitalWrite(segmentF2, LOW);
    break;
  }
}

void showSegment()
{
  if(segment == 0) 
  {
    displayNumber(1,mode);
    segment = 1;
  }
  else 
  {
    displayNumber(2,state);
    segment = 0;
  }
}

unsigned long diff(unsigned long a,unsigned long b)
{
  if(a>=b) return a-b;
  else return 0;
}
