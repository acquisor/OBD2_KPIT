#ifndef acQuisor_WiFi
#define acQuisor_WiFi

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"

#endif

class acQuisorWiFi
{
  public:
  acQuisorWiFi(String WiFiSsid, String WiFiPass, String AP_ssid, String AP_pass, String host);
  
  String deviceIP, customerWifiSsid, customerWifiPass, device_SSID, device_PASS, url, server;
  void Wconnect();

  void generateURL(String customerName, String speedData, String rpmData, String loadData, String throttleData, String dtcData);
};

#endif