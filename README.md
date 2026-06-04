# Self Healing IoT Network

## What does it do?
A mesh-like IoT network of ESP8266 nodes that monitor each other using heartbeat signals.
If any node goes down, the remaining nodes detect the failure, take over its responsibilities,
and send an instant Telegram alert. When the node comes back online, the network heals itself automatically.

## Why I built this
Single point of failure is a major problem in IoT deployments.
If one device fails, the entire system can go down.
I built this to demonstrate a self healing network where nodes monitor each other
and automatically recover from failures without any human intervention.

## Hardware used
- 3x ESP8266 NodeMCU modules
- WiFi router

## How it works
- Each node runs the same code with a unique nodeID
- Every 5 seconds each node sends a heartbeat ping to all other nodes
- If a node misses heartbeats for 15 seconds it is marked as dead
- The detecting node takes over and sends a Telegram alert
- When the dead node comes back online the network heals automatically

## Features
- Heartbeat based node health monitoring
- Automatic dead node detection
- Self healing — network recovers without manual intervention
- Telegram alert on node failure and recovery
- Easily scalable — just add more nodes and IPs

## How to run
1. Open node.cpp in Arduino IDE
2. Install ESP8266 board package
3. Change nodeID to 1, 2, 3 for each ESP8266
4. Enter your WiFi name and password
5. Enter your Telegram username and CallMeBot API key
6. Upload to each ESP8266
7. Open Serial Monitor at 115200 baud on any node to monitor network

## Future Improvements
- Increase network to 5+ nodes
- Add data collection on each node with local storage
- MQTT protocol instead of HTTP for faster communication
- Web dashboard showing live network topology
- Battery backup on each node for power failure scenarios

## Built by
Gauresh Pathak — Electronics and Telecommunication Engineering Student, Mumbai
