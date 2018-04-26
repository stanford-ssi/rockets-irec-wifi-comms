
/*
  ESP WiFi Arming Code Rev 01
  Created by Tylor Jilk, February 2018
  A part of the IREC Avionics Team of Stanford SSI
  Once the ESP is running, connect to its Wi-Fi network
  and go to one of 3 websites to do exactly what they
  sound like: ipaddress/arm, /disarm, or /status
  The GPIO pin which is controlled by these commands
  is called 'onpin'.
  If the serial into the ESP contains "Arm"/"Disarm", it will tell motherboard ESP to arm/disarm. If it contains
  "Stage", it will tell the Staging ESP to trigger staging
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "min.h"
#include "min.c"

//port 0 is skybass serial
//port 1 is payload client
struct min_context min_ctx;

const char WiFiAPPSK[] = "redshift";
const char APName[] = "Skybass";
const int onpin = 4; // IO5 on the Esp8266 WROOM 02
const int ledpin = 5;
String payload_ip = "192.168.4.2";
String a = "Armed";
String d = "Disarmed";
WiFiServer server(80);
WiFiClient client;
uint8_t skyb_data;
uint16_t min_tx_space(uint8_t port)
{
  // Ignore 'port' because we have just one context. But in a bigger application
  // with multiple ports we could make an array indexed by port to select the serial
  // port we need to use.
  uint16_t n =-1;
  switch(port)
  {

    case 0:
    n=Serial.available();
    break;
    case 1:
    n=client.available();
    break;

  }

  return n;
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  switch(port)
  {
    case 0:
    Serial.write(&byte, 1U);
    break;
    case 1:
    client.write(&byte, 1U);
    break;

  }

}
uint32_t min_time_ms(void)
{
  return millis();
}


void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port)
{
  uint32_t now = millis();
  switch(port)
  {
    case 0:
    //RECVD FROM SKYBASS
    //IF ID = 0, PASS THROUGH TO PAYLOAD
    //IF ID = 1, ARM - TO DO
    //IF ID =  2 DISRAM - TO DO

      switch(min_id)
      {
        case 0:
          min_queue_frame(&min_ctx, 0, min_payload, len_payload);
          break;
      }
    case 1:
    //RECVD FROM PAYLOAD
    //PRINT OUT TO SKYBASS
    min_queue_frame(&min_ctx, 0, min_payload, len_payload);
    break;
}
}

void setup() {
  Serial.begin(9600);
  pinMode(onpin, OUTPUT);
  digitalWrite(onpin, LOW);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, HIGH);
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  IPAddress ip(192, 168, 4, 1);
  IPAddress dns(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(APName, WiFiAPPSK);
  server.begin();
  min_init_context(&min_ctx, 0);
}

void loop()
{

  client = server.available();
  if (!client) {
    return;
  }
    char buf1[32];
    size_t buf_len1;
    if(Serial.available() > 0) {
      buf_len1 = Serial.readBytes(buf1, 32U);
    }
    else {
      buf_len1 = 0;
    }
    min_poll(&min_ctx, (uint8_t *)buf1, (uint8_t)buf_len1);

    char buf2[32];
    size_t buf_len2;
    if(client.available() > 0) {
      buf_len2 = client.readBytes(buf2, 32U);
    }
    else {
      buf_len2 = 0;
    }
    min_poll(&min_ctx, (uint8_t *)buf2, (uint8_t)buf_len2);

//BACKUP ARMING BY PHONE
    String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: " + req); //DBG
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
}
