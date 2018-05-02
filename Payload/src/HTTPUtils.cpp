#include "HTTPUtils.hpp"

namespace httputils
{

void HTTPRespond(WiFiClient client, String message)
{
  //Prepare the response. Start with the common header:
  String resp = "HTTP/1.1 200 OK\r\n";
  resp += "Content-Type: text/html\r\n\r\n";
  resp += "<!DOCTYPE HTML>\r\n<html>\r\n";
  resp += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";
  resp += message;
  resp += "</body></html>\n";

  // Send the response to the client
  client.print(resp);
  delay(1);
  Serial.println("Client disconnected"); //DBG

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}
}