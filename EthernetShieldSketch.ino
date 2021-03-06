/*
Things to do:
Change means code needs to be added or modified there.
Replace char arrays with strings if more space is available.
Remove means remove
*/
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10,9,0,138);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);
boolean clientConnected=false;

//Light
  //Sensor
  const int ldrPin1=A0;  
  const int ldrPin2=A1;
  const int ldrPin3=A2;
  const int ldrPin4=A3;
  //Output
  const int lightPin=2;
  const int brightnessRelayPin1=3;
  const int brightnessRelayPin2=4;
  //State
  int lightPinState=LOW;
  int brightnessRelayState1=LOW;
  int brightnessRelayState2=LOW;

//LDR readings 
int ldrValue1=0;
int ldrValue2=0;
int ldrValue3=0;
int ldrValue4=0;

//LDR levels
int ldrB0=700;
int ldrB1=600;  //Change with actual values
int ldrB2=500;
int ldrB3=400;
int ldrB4=0;

//Modes
const int AUTO=0;
const int MANUAL=1;
int currentMode=AUTO;

//HTTP requests
char page[110];
char value[5];
int index=0;

void setup() {
 // Open serial communications:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
  //Output initialization
  pinMode(lightPin,OUTPUT);
  digitalWrite(lightPin,LOW);
  pinMode(brightnessRelayPin1,OUTPUT);
  digitalWrite(brightnessRelayPin1,LOW);
  pinMode(brightnessRelayPin2,OUTPUT);
  digitalWrite(brightnessRelayPin2,LOW);
}


void loop() {
  //LDR sensors
  ldrValue1=analogRead(ldrPin1);
  ldrValue2=analogRead(ldrPin2);
  ldrValue3=analogRead(ldrPin3);
  ldrValue4=analogRead(ldrPin4);
  //Call auto mode function if the current mode is auto.
  if(currentMode==AUTO)
  {
    autoRecommend();
    setRecommended();
  }
  
  // listen for incoming clients
  EthernetClient client = server.available();
  for(int i=0;i<110;i++)page[i]=' ';  //Char array/string
  if (client) {
    int count=0;
    int storeRequest=0;
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(storeRequest==1)
          page[count++]=c;  //Char array/string
        if(c=='/' && storeRequest!=2)
          storeRequest=1;
        if(c==' ' && storeRequest)
          storeRequest=2;
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {      
          Serial.println(page);  //Remove
          //Status
          if(page[0]==' ')  //Char array/String
          {
            currentMode=AUTO;
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();
            //Send page to client
            sendStatusPage(client);
          }
          
          //Manual   
          else if(page[0]=='m')  //Char array/String
          {
            //New state set  //The random indexes are the places where values will be present when data is sent through the correct format.
            if(page[11]=='?')  //Char array/String
            {
              for(int i=0;i<5;i++)value[i]=' ';  //Char array/String
              //Set light on or off
              value[0]=page[24];  //Char array/String 
              Serial.println(value);
              if(value[0]=='1')  //Char array/String
                digitalWrite(lightPin,HIGH);
              else
                digitalWrite(lightPin,LOW);
                
              for(int i=0;i<5;i++)value[i]=' ';  //Char array/String
              //Set brightness
              value[0]=page[42];
              Serial.println(value);
              if(value[0]=='0')  
                digitalWrite(lightPin,LOW);
              else if(value[0]=='1')
              {
                digitalWrite(brightnessRelayPin1,HIGH);
                digitalWrite(brightnessRelayPin2,HIGH);
              }
              else if(value[0]=='2')
              {
                digitalWrite(brightnessRelayPin1,LOW);
                digitalWrite(brightnessRelayPin2,HIGH);
              }
              else if(value[0]=='3')
              {
                digitalWrite(brightnessRelayPin1,HIGH);
                digitalWrite(brightnessRelayPin2,LOW);
              }
              else
              {
                digitalWrite(brightnessRelayPin1,LOW);
                digitalWrite(brightnessRelayPin2,LOW);
              }
              
              //Set others
            }
            currentMode=MANUAL;  
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();
            //Send page to client
            sendManualPage(client);
          }          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void autoRecommend()  //Sets the state variables according to sensor values
{ 
  //Light
  if(ldrValue1>ldrB0)  
    lightPinState=LOW;
  else if(ldrValue1>ldrB1)
  {
    lightPinState=HIGH;
    brightnessRelayState1=HIGH;
    brightnessRelayState2=HIGH;
  }
  else if(ldrValue1>ldrB2)
  {
    lightPinState=HIGH;
    brightnessRelayState1=LOW;
    brightnessRelayState2=HIGH;
  }
  else if(ldrValue1>ldrB3)
  {
    lightPinState=HIGH;
    brightnessRelayState1=HIGH;
    brightnessRelayState2=LOW;
  }
  else 
  {
    lightPinState=HIGH;
    brightnessRelayState1=LOW;
    brightnessRelayState2=LOW;
  }
}

void setRecommended()  //Writes the state to the output pins
{
  //Light
  digitalWrite(lightPin,lightPinState);  
  digitalWrite(brightnessRelayPin1,brightnessRelayState1);
  digitalWrite(brightnessRelayPin2,brightnessRelayState2);
}

void sendManualPage(EthernetClient cl)
{
  cl.println("<!DOCTYPE html");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
  cl.println("<title>Manual Mode</title>");
  cl.println("</head>");
  cl.println("<body>");
  //cl.println("<h1>Currently in manual mode. Go to the auto mode page to set it to auto and/or change the auto mode settings.<h1>");  //Uncomment
  cl.println("<form method=\"get\">");
  cl.println("<h2>Lights</h2>");
  
  if(digitalRead(lightPin)==HIGH)  //Light on/off
  {
    cl.println("<input type=\"radio\" id=\"lightOnButton\" value=\"1\" name=\"lightButton\" checked=\"checked\"/>On");
    cl.println("<input type=\"radio\" id=\"lightOffButton\" value=\"0\" name=\"lightButton\"/>Off");
  }
  else
  {
    cl.println("<input type=\"radio\" id=\"lightOnButton\" value=\"1\" name=\"lightButton\" />On");
    cl.println("<input type=\"radio\" id=\"lightOffButton\" value=\"0\" name=\"lightButton\" checked=\"checked\"/>Off");
  }
  
  cl.println("<br/>");
  
  cl.println("Brightness:<input type=\"number\" id=\"lightBrightness\" name=\"lightBrightness\" min=\"0\" max=\"4\"");  //Brightness
  if(digitalRead(lightPin)==LOW)
    cl.println("value=\"0\" />");
  else if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==HIGH)
    cl.println("value=\"1\" />");
  else if(digitalRead(brightnessRelayPin1)==LOW && digitalRead(brightnessRelayPin2)==HIGH)
    cl.println("value=\"2\" />");
  else if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==LOW)
    cl.println("value=\"3\" />");
  else
    cl.println("value=\"4\" />");
  
  cl.println("<p id=\"lightStatus\">Current status is");  //Brightness
  if(digitalRead(lightPin)==HIGH)
  {
    cl.println("on at brightness ");
    if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==HIGH)
      cl.println("1</p>");
    else if(digitalRead(brightnessRelayPin1)==LOW && digitalRead(brightnessRelayPin2)==HIGH)
      cl.println("2</p>");
    else if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==LOW)
      cl.println("3</p>");
    else
      cl.println("4</p>");
  }
  else
    cl.println("off</p>");
    
  cl.println("<p id=\"ldrReading\">LDR reading is");
  cl.println(ldrValue1);  //Change with average or something else
  cl.println("</p>");
  
  cl.println("<p id=\"lightRecommended\">Recommended:");  //Light Recommended
  autoRecommend();
  if(lightPinState==HIGH)
  {
    cl.println("on at brightness ");
    if(brightnessRelayState1==HIGH && brightnessRelayState2==HIGH)
      cl.println("1</p>");
    else if(brightnessRelayState1==LOW && brightnessRelayState2==HIGH)
      cl.println("2</p>");
    else if(brightnessRelayState1==HIGH && brightnessRelayState2==LOW)
      cl.println("3</p>");
    else
      cl.println("4</p>");
  }
  else
    cl.println("off</p>");
    
  cl.println("<input type=\"submit\" />");
  //Fan
  //AC
  
  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" />");
  cl.println("<input type=\"button\" id=\"backButton\" value=\"Back\" onclick=\"window.location='/'\"/>");
  cl.println("</form>");
  cl.println("</body>");
  cl.println("</html>");
}

void sendStatusPage(EthernetClient cl)
{
  cl.println("<!DOCTYPE html>");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
  cl.println("<title>Status</title>");
  cl.println("</head>");
  
  cl.println("<body>");
  //cl.println("<h1>Currently in auto mode. Go to manual mode to set to manual mode. Go to auto mode to set to auto mode and/or change auto mode settings.");  //Change Uncomment
  cl.println("<h2>Lights</h2>");
  cl.println("<br />");
  
  cl.println("<p id=\"lightStatus\">Current status is");  //Brightness
  if(digitalRead(lightPin)==HIGH)
  {
    cl.println("on at brightness ");
    if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==HIGH)
      cl.println("1</p>");
    else if(digitalRead(brightnessRelayPin1)==LOW && digitalRead(brightnessRelayPin2)==HIGH)
      cl.println("2</p>");
    else if(digitalRead(brightnessRelayPin1)==HIGH && digitalRead(brightnessRelayPin2)==LOW)
      cl.println("3</p>");
    else
      cl.println("4</p>");
  }
  else
    cl.println("off</p>");
    
  cl.println("<p id=\"ldrReading\">LDR reading is");
  cl.println(ldrValue1);  //Change with average or something
  cl.println("</p>");
  
  cl.println("<p id=\"lightRecommended\">Recommended:");  //Light Recommended
  autoRecommend();
  if(lightPinState==HIGH)
  {
    cl.println("on at brightness ");
    if(brightnessRelayState1==HIGH && brightnessRelayState2==HIGH)
      cl.println("1</p>");
    else if(brightnessRelayState1==LOW && brightnessRelayState2==HIGH)
      cl.println("2</p>");
    else if(brightnessRelayState1==HIGH && brightnessRelayState2==LOW)
      cl.println("3</p>");
    else
      cl.println("4</p>");
  }
  else
    cl.println("off</p>");
  
  cl.println("<input type=\"button\" id=\"refreshButton\" value=\"Refresh\" onclick=\"window.location.reload()\" />");
  cl.println("<input type=\"button\" id=\"manualButton\" value=\"Manual mode\" onclick=\"window.location='/manual.html'\"/>");
  cl.println("<input type=\"button\" id=\"autoButton\" value=\"Auto mode\" />");
  cl.println("</body>");
  cl.println("</html>");
}
