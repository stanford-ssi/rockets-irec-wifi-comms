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
#include "HTTPUtils.hpp"

const int shim = 13; // controls the power to the pi
const int pi = 12; // signals to the pi to do an orderly shutdown
const char AP_SSID[] = "Payload";
const char STA_SSID[] = "Skybass";
const char PSK[] = "redshift";

void setupAP();
void setupSTA();

const int checkTime = 2000; //Interval to check for skybass in ms
const int onpin = 5; // IO5 on the Esp8266 WROOM 02

String armed = "Armed";
String disarmed = "Disarmed";
String invalid = "Invalid Request";

WiFiServer server(80);

uint32_t scanTimer = millis();
boolean inSTAMode = false;
boolean skybassAvailable = false;

void setup()
{
  Serial.begin(115200);
  pinMode(shim, OUTPUT);
    pinMode(pi, OUTPUT);
    digitalWrite(shim, LOW); // start out disarmed
    digitalWrite(pi, HIGH);

  setupSTA();
  server.begin();
}

void checkForSkybass()
{
  skybassAvailable = false;
  int n = WiFi.scanNetworks();
  Serial.println("scan done"); //DBG
  if (n == 0)
  {
    Serial.println("no networks found"); //DBG
  }
  else
  {
    Serial.print(n);                   //DBG
    Serial.println(" networks found"); //DBG
    for (int i = 0; i < n; ++i)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
      if (WiFi.SSID(i).equals(STA_SSID))
      {
        skybassAvailable = true;
      }
    }
  }
  Serial.println();
}

void setupAP()
{
  WiFi.disconnect();

  IPAddress ip(192, 168, 4, 2);
  IPAddress dns(192, 168, 4, 2);
  IPAddress gateway(192, 168, 4, 2);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(AP_SSID, PSK);
  inSTAMode = false;
}

void setupSTA()
{
  WiFi.softAPdisconnect();

  IPAddress ip(192, 168, 4, 2);
  IPAddress dns(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(STA_SSID, PSK);
  inSTAMode = true;
}

void loop()
{

  if (millis() - scanTimer > checkTime)
  {
    scanTimer = millis();
    checkForSkybass();
  }
  if (!inSTAMode && skybassAvailable)
  {
    setupSTA();
  }
  if (!skybassAvailable && inSTAMode)
  {
    setupAP();
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: " + req); //DBG
  client.flush();
  String resp = "";

  if (req.indexOf("/disarm") != -1)
  {
    digitalWrite(pi, LOW);
    delay(1000 * 5); // wait 5 seconds for shutdown
    digitalWrite(shim, LOW);
    httputils::HTTPRespond(client, disarmed);
  }
  else if (req.indexOf("/arm") != -1)
  {
    digitalWrite(pi, HIGH);
    digitalWrite(shim, HIGH);
    httputils::HTTPRespond(client, armed);
  }
  else if (req.indexOf("/status") != -1)
  {
    if (digitalRead(shim))
    {
      httputils::HTTPRespond(client, armed);
    }
    else
    {
      httputils::HTTPRespond(client, disarmed);
    }
  }
  else
  {
    httputils::HTTPRespond(client, invalid);
  }
}
