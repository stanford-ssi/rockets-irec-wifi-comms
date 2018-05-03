
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
#include "HTTPUtils.hpp"
#include "min_support.h"
#include "min-irec.h"
#include "min.h"
#include "min.c"

void poll_min();
void min_tx_byte(uint8_t port, uint8_t byte);
void sendPayloadArming();
void checkStatus();
void handleManualArming();
void handleArming();
void setSkybassArming();

//port 0 is skybass serial
struct min_context skyb_ctx;
const int checkTime = 2000; //refresh time to check status in ms
const char PSK[] = "redshift";
const char AP_SSID[] = "Skybass";

const int onpin = 4; // IO5 on the Esp8266 WROOM 02
const int ledpin = 5;

IPAddress payloadIP(192, 168, 4, 2);

String armed = "Armed";
String disarmed = "Disarmed";
String invalid = "Invalid Request";

WiFiServer server(80);
WiFiClient payload_socket;

esp_arm_t esp_arm;
esp_status_t esp_status;

uint32_t timer = millis();

void setup()
{
  Serial.begin(115200);

  pinMode(onpin, OUTPUT);
  pinMode(ledpin, OUTPUT);
  digitalWrite(onpin, LOW);
  digitalWrite(ledpin, HIGH);

  WiFi.mode(WIFI_AP);

  IPAddress ip(192, 168, 4, 1);
  IPAddress dns(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.config(ip, dns, gateway, subnet);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(AP_SSID, PSK);
  server.begin();

  min_init_context(&skyb_ctx, 0);
}

void loop()
{
  //poll serial ports into MIN
  poll_min();
  //BACKUP ARMING BY PHONE
  handleManualArming();

  if (millis() > timer - checkTime)
  {
    timer = millis();
    checkStatus();
    min_queue_frame(&skyb_ctx, ESP_STATUS, (uint8_t *)&esp_status, sizeof(esp_status));
  }
}

void handleManualArming()
{
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
    digitalWrite(onpin, 0);
    httputils::HTTPRespond(client, disarmed);
  }
  else if (req.indexOf("/arm") != -1)
  {
    digitalWrite(onpin, 1);
    httputils::HTTPRespond(client, armed);
  }
  else if (req.indexOf("/status") != -1)
  {
    if (digitalRead(onpin))
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

void poll_min()
{
  char buf[32];
  size_t buf_len;
  if (Serial.available() > 0)
  {
    buf_len = Serial.readBytes(buf, 32U);
  }
  else
  {
    buf_len = 0;
  }
  min_poll(&skyb_ctx, (uint8_t *)buf, (uint8_t)buf_len);
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  switch (port)
  {
  case 0:
    Serial.write(&byte, 1U);
    break;
    /*case 1:
    payload_socket.write(&byte, 1U);
    break;*/
  }
}

uint16_t min_tx_space(uint8_t port)
{
  uint16_t n = -1;
  switch (port)
  {
  case 0:
    n = Serial.availableForWrite();
    break;
    /*case 1:
    n = payload_socket.availableForWrite();
    break;*/
  }
  return n;
}

void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port)
{
  switch (min_id)
  {
  case ESP_ARM:
    memcpy(&esp_arm, min_payload, len_payload);
    handleArming();
    break;
  case ESP_STATUS:
    break;
  }
}

void handleArming()
{
  sendPayloadArming();
  setSkybassArming();
}


void setSkybassArming()
{
  if (esp_arm.arm_skybass)
  {
    digitalWrite(onpin,1);
    esp_status.skybass_armed = true;
  }
  else
  {
    digitalWrite(onpin,0);
    esp_status.skybass_armed = false;
  }

}

void sendPayloadArming()
{
  if (payload_socket.connect(payloadIP, 80))
  {
    Serial.println("Connected"); //DBG
    esp_status.payload_alive = true;
    if (esp_arm.arm_payload)
    {
      payload_socket.println("/arm");
      esp_status.payload_armed = true;
    }
    else
    {
      payload_socket.println("/disarm");
      esp_status.payload_armed = false;
    }
  }
  else
  {
    Serial.println("Connect Failed");
    esp_status.payload_alive = false;
  }
}

void checkStatus()
{
  esp_status.skybass_alive = true;
  esp_status.skybass_armed = digitalRead(onpin);

  if (payload_socket.connect(payloadIP, 80))
  {
    Serial.println("Connected");
    esp_status.payload_alive = true;
    payload_socket.println("/status");
    esp_status.payload_armed = false;
    delay(100);

    while (payload_socket.available())
    {
      String resp = payload_socket.readStringUntil('\n');
      if (resp.indexOf("Armed") != -1)
      {
        esp_status.payload_armed = true;
      }
    }
  }
  else
  {
    Serial.println("Connect Failed");
    esp_status.payload_alive = false;
  }
}
