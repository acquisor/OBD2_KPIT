/*CAN Receiver*/
#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<ESP8266HTTPClient.h>
#include <SPI.h>

#include "acQuisor_WiFi.h"
#include "mcp_can.h"

char *host="172.22.25.3";
const int httpPort=80;

String serverResponse, Cdate;
ESP8266WebServer server(80);

String customerName="";

String wifiSsid = "BOYS_HOSTEL", wifiPass = "computer12345", apSsid = "obd_acQuisor", apPass = "acquisor123";
acQuisorWiFi acqWifi(wifiSsid, wifiPass, apSsid, apPass, host);

void clearDTC();
void handleRoot();
void handleEdit();
void handleSuccess();

bool canReceive();
const int spiCSPin=D8;
int flag = 0;
boolean cnt=0;
MCP_CAN CAN(spiCSPin);

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN,OUTPUT);
    
    Serial.print("Configuring access point...");
    WiFi.softAP(acqWifi.device_SSID.c_str(), acqWifi.device_PASS.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    
    server.on("/dtcClear",clearDTC);
    server.on("/", handleRoot);
    server.on("/edit", handleEdit); //Associate the handler function to the path
    server.on("/success", handleSuccess);
    server.begin();

    if(acqWifi.customerWifiSsid!="default"&&acqWifi.customerWifiSsid!="")
    acqWifi.Wconnect();
    
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS Init Failed");
        delay(100);
    }
    Serial.println("CAN BUS  Init OK!");
}

unsigned char stmp[8] =  {0X01,0X0D,0X55,0X55,0X55,0X55,0X55,0X55};   //Vehicle Speed
unsigned char stmp1[8] = {0X01,0X0C,0X55,0X55,0X55,0X55,0X55,0X55};   //RPM
unsigned char stmp2[8] = {0X01,0X04,0X55,0X55,0X55,0X55,0X55,0X55};   //Engine load
unsigned char stmp3[8] = {0X01,0X11,0X55,0X55,0X55,0X55,0X55,0X55};   //Throttle Positionb
unsigned char stmp4[8] = {0x03,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   //DTC
unsigned char stmp5[8] = {0x04,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   //Clear DTC

void wait()
{
  digitalWrite(D4,1);
  delay(200);
  digitalWrite(D4,0);
  delay(200);
}

void loop()
{
  unsigned char len = 0;
  unsigned char buf[8];
  String obdData="";
  server.handleClient();
  while(acqWifi.customerWifiSsid=="default"||acqWifi.customerWifiSsid=="")
  {
    wait();
    server.handleClient();
  }
  
  if(flag==0 && cnt==0)
  {
    Serial.println("Requesting Speed Data");
    CAN.sendMsgBuf(0x71, 0, 8, stmp);
    cnt=1;
  }
     
  if(CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buf);
    unsigned long canId = CAN.getCanId();
    if(canId=0x70)
    {
      Serial.println("-----------------------------");
      Serial.print("Data from ID: 0x");
      Serial.println(canId, HEX);
        
      for(int i = 0; i<len; i++)
      {
            String s2=String(buf[i],HEX);
            if(s2.length()==1)
            {
              obdData+="0";
            }
            obdData+=String(buf[i],HEX);
        Serial.print(buf[i],HEX);
        Serial.print("\t");
        
        if(i==0 && buf[i]==0X43)
        {
          flag=0;
        }
         
          if(i==1)
          {
              if(buf[i]==0x0D)
              {
                flag=1;
            
              }
              else if(buf[i]==0x0C)
              {
                flag=2;
              }
              else if(buf[i]==0x04)
              {
                flag=3;
              }
              
              else if(buf[i]==0x11)
              {
                flag=4;
              }
           } 
        }
      }
    }

    if (WiFi.status() == WL_CONNECTED)
    {   
      Serial.print("\nThis device's IP: ");
      Serial.println(acqWifi.deviceIP);
      Serial.print("\nConnecting to host @ ");
      Serial.print(host);
      Serial.println("\n");
      
      HTTPClient http;
      
      acqWifi.generateURL(customerName,obdData,Cdate);
      Serial.println(acqWifi.url);
      
      http.begin(acqWifi.url);
      
      int httpCode = http.GET();
      
      Serial.print("HTML Code: ");
      Serial.println(httpCode);

      delay(100);    
    
      serverResponse="x";
      
      serverResponse = http.getString();
      http.end();
      Serial.print("Response from server: ");
      Serial.println(serverResponse);
      
      int index = serverResponse.indexOf('_');
      String cmd = serverResponse.substring(0,index);
      Cdate = serverResponse.substring(index+1);
      Serial.print("\nCommand from server: ");
      Serial.println(cmd);
      Serial.print("\nToday is: ");
      Serial.println(Cdate);
    }
    
    if(flag==0)
    {
      Serial.println("\nRequesting Speed Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp);
    }
    else if(flag==1)
    {
      Serial.println("\nRequesting RPM Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp1);
    }
    
    else if(flag==2)
    {
      Serial.println("\nRequesting Engine Load Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp2);
    }
    
    else if(flag==3)
    {
      Serial.println("\nRequesting Throttle Position Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp3);
    }
    
    else if(flag==4)
    {
      Serial.println("\nRequesting DTC code");
      CAN.sendMsgBuf(0x71, 0, 8, stmp4);
    }
    delay(1000);
}

void clearDTC()
{
 String htmlCode;
 
 Serial.println("\nRequesting DTC code");
 CAN.sendMsgBuf(0x71, 0, 8, stmp5);

  delay(100);
 if(canReceive())
  htmlCode="ok";
 else
  htmlCode="failed";
  
  server.send(200, "text/html", htmlCode); 
}

bool canReceive()
{
  unsigned char len = 0;
  unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();
        if(canId==0x70)
        {
          for(int i = 0; i<len; i++)
          {
              Serial.print(buf[i]);
              Serial.print("\t");
              if(i==0&&buf[i]==0x44)
              {
                return true;  
              }
              else
              {
                return false;
              }
          }
        }
    }
}

void handleSuccess()
{
  
  acqWifi.customerWifiSsid = String(server.arg("customerWifiSsid"));
  acqWifi.customerWifiPass = String(server.arg("customerWifiPass"));
  customerName = String(server.arg("customerName"));
  
  acqWifi.Wconnect();

  String message = "<br><br><center><h1>The OBD details have been successfully updated.</h1><br><br> The OBD is connected to: ";
  message += acqWifi.customerWifiSsid;
  message += "<br><br>The IP address of OBD is: ";
  message += acqWifi.deviceIP;
  message += "<br><br><li><a href='http://172.22.25.3/obd/'>Visit website for management and control</a></li><br><li><b>Contact us at: 1505051@ritindia.edu</b></li>";
  message += "</center>";
  delay(500);
  server.send(200, "text/html", message);
}

void handleRoot()
{
  String message2 = "<center><h1> Welcome to acQuisor OBD </h1><br><br><li><a href = '/edit'> Setup the acQuisor OBD </a></li><br><li><a href = '/deviceStatus'> View device status and parameters</a></li><br><br> Thank you for buying acQuisor OBD.<br><br><b>Contact us at 1505051@ritindia.edu </b></center>";

  server.send(200, "text/html", message2);
}
