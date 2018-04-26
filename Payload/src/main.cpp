/*
  ESP WiFi Arming Code Rev 01
  Created by Tylor Jilk, February 2018
  A part of the IREC Avionics Team of Stanford SSI

  Once the ESP is running, connect to its Wi-Fi network
  and go to one of 3 websites to do exactly what they
  sound like: ipaddress/arm, /disarm, or /status

  The GPIO pin which is controlled by these commands
  is called 'onpin'.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <min.h>
const char APName[] = "Payload";
const char SkybassAP[] = "Skybass";
const char WiFiAPPSK[] = "redshift";
const int onpin = 5; // IO5 on the Esp8266 WROOM 02
String a = "Armed";
String d = "Disarmed";
IPAddress ip(192,168,4,2);
int c=0;
WiFiServer server(80);
boolean Skybass = false;
void setup() {
    Serial.begin(115200);
    pinMode(onpin, OUTPUT);
    digitalWrite(onpin, LOW);
    WiFi.mode(WIFI_STA);
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);

    IPAddress dns(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,dns,gateway,subnet);
    server.begin();
    WiFi.begin(SkybassAP,WiFiAPPSK);
    Skybass = true;
}
void checkForSkybass()
{
  // WiFi.scanNetworks will return the number of networks found
  if((WiFi.SSID().equals("Skybass"))&&WiFi.localIP()==ip)
  {
    Skybass = true;
  }
  else
  {
    Skybass = false;
  }
/*

  int n = WiFi.scanNetworks();
  Serial.println("scan done"); //DBG
  if (n == 0) {
    Serial.println("no networks found"); //DBG
    Skybass = false;
  } else {
    Serial.print(n); //DBG
    Serial.println(" networks found"); //DBG
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      if(WiFi.SSID(i).equals("Skybass"))
      {
        Skybass = true;
      }

    }
  }
  Serial.println("");
  */
}

void setupAP()
{
  WiFi.mode(WIFI_AP);
  IPAddress ip(192, 168, 4, 2);
  IPAddress dns(192, 168, 4, 2);
  IPAddress gateway(192, 168, 4, 2);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(APName, WiFiAPPSK);

}

void loop() {
  if(!Skybass)
  {
    setupAP();
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: "+req); //DBG
  client.flush();
  String resp = "";

  if (req.indexOf("/disarm") != -1)
  {
    digitalWrite(onpin, 0);
    resp = d;
  }
  else if (req.indexOf("/arm") != -1)
  {
      digitalWrite(onpin, 1);
      resp = a;
  }
  else if (req.indexOf("/status") != -1)
  {
    if(digitalRead(onpin))
    {
      resp=a;
    }
    else
    {
      resp= d;
    }
  }
  client.flush();
  client.print(resp);
  delay(1);
  Serial.println("Client disconnected"); //DBG

  c++;
  if(c>500)
  {
    checkForSkybass();
    c=0;
  }


}
