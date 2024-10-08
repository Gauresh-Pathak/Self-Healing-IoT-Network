#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// network config
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// each node has a unique id
int nodeID = 1; // change this to 2, 3 for other nodes

// all node IPs on the network
String nodeIPs[] = {
  "192.168.1.101", // node 1
  "192.168.1.102", // node 2
  "192.168.1.103"  // node 3
};
int totalNodes = 3;

// heartbeat timing
unsigned long lastHeartbeat = 0;
int heartbeatInterval = 5000; // every 5 seconds

// node status
bool nodeStatus[3] = {true, true, true};

WiFiServer server(80);

void sendHeartbeat(String targetIP) {
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + targetIP + "/heartbeat?from=" + String(nodeID);
  http.begin(client, url);
  http.GET();
  http.end();
}

void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/heartbeat") != -1) {
    // another node is alive
    int fromNode = request.substring(request.indexOf("from=") + 5).toInt();
    nodeStatus[fromNode - 1] = true;
    Serial.println("Heartbeat received from Node " + String(fromNode));
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("OK");
  }

  if (request.indexOf("/status") != -1) {
    // return status of all nodes
    String response = "";
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

  server.begin();
  Serial.println("Node " + String(nodeID) + " started and listening...");
}

void loop() {
  // handle incoming requests from other nodes
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  // send heartbeat to all other nodes every 5 seconds
  if (millis() - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = millis();

    for (int i = 0; i < totalNodes; i++) {
      if (i != nodeID - 1) {
        Serial.println("Sending heartbeat to Node " + String(i + 1));
        sendHeartbeat(nodeIPs[i]);
      }
    }
  }
}
