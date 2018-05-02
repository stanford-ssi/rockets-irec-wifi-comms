#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

namespace httputils
{
   void HTTPRespond(WiFiClient client, String message);
}