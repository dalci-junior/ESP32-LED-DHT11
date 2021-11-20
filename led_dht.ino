#include <WiFi.h>
#include "DHT.h"

const char* ssid = "Casa_2.4";
const char* password = "takenking";

// Setting Static IP.
IPAddress local_IP(192, 168, 0, 150);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(80); // Port 80

#define LED32  32    // LED port32
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


String estado = "";
int wait30 = 30000; // time to reconnect when connection is lost.

void setup() {
  Serial.begin(9600);
  pinMode(LED32, OUTPUT);

  dht.begin();

  // Setting Static IP.
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Error in configuration.");
  }

  // Connect WiFi net.
  Serial.println();
  Serial.print("Connecting with ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected with WiFi.");

  // Start Web Server.
  server.begin();
  Serial.println("Web Server started.");

  // This is IP
  Serial.print("This is IP to connect to the WebServer: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // If disconnected, try to reconnect every 30 seconds.
  if ((WiFi.status() != WL_CONNECTED) && (millis() > wait30)) {
    Serial.println("Trying to reconnect WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    wait30 = millis() + 30000;
  }
  // Check if a client has connected..
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.print("New client: ");
  Serial.println(client.remoteIP());

  // Espera hasta que el cliente envíe datos.
  while (!client.available()) {
    delay(1);
  }

  // Read the information sent by the client.
  String req = client.readStringUntil('\r');
  Serial.println(req);

  //Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  String humi = String(h, 2);
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  String temp = String(t, 2);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  

  // Make the client's request.
  if (req.indexOf("ledon") != -1) {
    digitalWrite(LED32, HIGH);
    estado = "LED Ligado";
  }
  if (req.indexOf("ledoff") != -1) {
    digitalWrite(LED32, LOW);
    estado = "LED desligado";
  }


  //////////////////////////////////////////////
  // Página WEB. ////////////////////////////
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  
  client.print(estado);
  client.print(",");
  if (isnan(t)) {
    client.print("NaN");
  } else {
    client.print(temp);
  }
  client.print(",");
  if (isnan(h)) {
    client.print("NaN");
  } else {
    client.print(humi);
  }
  
  client.stop();
  delay(1);
}
