#include "acQuisor_WiFi_motor.h"
#include<ESP8266WiFi.h>
acQuisorWiFi::acQuisorWiFi(String WiFiSsid, String WiFiPass, String AP_ssid, String AP_pass, String host)
{
  customerWifiSsid = WiFiSsid;
  customerWifiPass = WiFiPass;
  device_SSID = AP_ssid;
  device_PASS = AP_pass;
  server = host;
}

void acQuisorWiFi::Wconnect()
{
  deviceIP="";
  //server.handleClient();
  WiFi.begin(customerWifiSsid.c_str(), customerWifiPass.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  
  Serial.print("\nIP address: ");
  deviceIP = String(WiFi.localIP()[0]) + String('.');
  deviceIP += String(WiFi.localIP()[1]) + String('.');
  deviceIP += String(WiFi.localIP()[2]) + String('.');
  deviceIP += String(WiFi.localIP()[3]);
}


void acQuisorWiFi::generateURL(String customerName, String Rpm, String Speed, String engineLoad, String throttle, String dtc, String Cdate)
{
  String u = "http://";
  u += server;
  u += "/obd/embeddedGateway/dataFromObdScanner.php";
  u += "?customerName=";
  u += customerName;
  u += "&rpm=";
  u += Rpm;
  u += "&speed=";
  u += Speed;
  u += "&load=";
  u += engineLoad;
  u += "&throttle=";
  u += throttle;
  u += "&cdate=";
  u += Cdate;
  u += "&IP=";
  u += deviceIP;
  u += "&customerWifiSsid=";
  u += customerWifiSsid;
  url = u;
}

