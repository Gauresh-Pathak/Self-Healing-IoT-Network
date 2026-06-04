#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* telegramUsername = "@YOUR_TELEGRAM_USERNAME";
const char* apiKey = "YOUR_CALLMEBOT_API_KEY";

int nodeID = 1;

String nodeIPs[] = {
  "192.168.1.101",
  "192.168.1.102",
  "192.168.1.103"
};
int totalNodes = 3;

unsigned long lastHeartbeat = 0;
int heartbeatInterval = 5000;
unsigned long lastSeen[3] = {0, 0, 0};
int deadThreshold = 15000;

bool nodeStatus[3] = {true, true, true};
bool alertSent[3] = {false, false, false};

WiFiServer server(80);

void reconnectWifi() {
  // retry wifi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected!");
    } else {
      Serial.println("Reconnection failed.");
    }
  }
}

void sendAlert(String message) {
  // sending telegram alert
  WiFiClient client;
  HTTPClient http;
  String url = "https://api.callmebot.com/text.php?user=" + String(telegramUsername) + "&apikey=" + String(apiKey) + "&text=" + message;
  http.begin(client, url);
  http.GET();
  http.end();
}

void sendHeartbeat(String targetIP) {
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + targetIP + "/heartbeat?from=" + String(nodeID);
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode != 200) {
    int nodeIndex = 0;
    for (int i = 0; i < totalNodes; i++) {
      if (nodeIPs[i] == targetIP) nodeIndex = i;
    }
    nodeStatus[nodeIndex] = false;
  }
  http.end();
}

void checkDeadNodes() {
  for (int i = 0; i < totalNodes; i++) {
    if (i != nodeID - 1) {
      if (millis() - lastSeen[i] > deadThreshold) {
        if (nodeStatus[i]) {
          nodeStatus[i] = false;
          Serial.println("NODE " + String(i + 1) + " IS DOWN!");

          if (!alertSent[i]) {
            alertSent[i] = true;
            sendAlert("NETWORK ALERT: Node " + String(i + 1) + " is DOWN! Node " + String(nodeID) + " taking over.");
            Serial.println("Alert sent! Taking over Node " + String(i + 1) + " responsibilities");
          }
        }
      } else {
        if (!nodeStatus[i]) {
          nodeStatus[i] = true;
          alertSent[i] = false;
          Serial.println("Node " + String(i + 1) + " is back online!");
          sendAlert("Node " + String(i + 1) + " is back ONLINE!");
        }
      }
    }
  }
}

void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/heartbeat") != -1) {
    int fromNode = request.substring(request.indexOf("from=") + 5).toInt();
    nodeStatus[fromNode - 1] = true;
    lastSeen[fromNode - 1] = millis();
    Serial.println("Heartbeat from Node " + String(fromNode));
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("OK");
  }

  if (request.indexOf("/status") != -1) {
    String response = "Network Status:\n";
    for (int i = 0; i < totalNodes; i++) {
      response += "Node " + String(i + 1) + ": " + (nodeStatus[i] ? "ALIVE" : "DEAD") + "\n";
    }
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println();
    client.println(response);
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.println("Node " + String(nodeID) + " IP: " + WiFi.localIP().toString());

  lastSeen[nodeID - 1] = millis();
  server.begin();
  Serial.println("Node " + String(nodeID) + " online!");
  sendAlert("Node " + String(nodeID) + " is online and monitoring network.");
}

void loop() {
  reconnectWifi();

  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  if (millis() - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = millis();

    for (int i = 0; i < totalNodes; i++) {
      if (i != nodeID - 1) {
        sendHeartbeat(nodeIPs[i]);
      }
    }

    checkDeadNodes();
  }
}
