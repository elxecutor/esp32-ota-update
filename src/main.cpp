
#include <WiFi.h>
#include <WebServer.h>

#include <ElegantOTA.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password, 6);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)  {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []()
    { server.send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
  });

  ElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  ElegantOTA.loop();
}