/*
  More Tutorials:
  Website http://trialcommand.com
  In English: http://en.trialcommand.com
  En Español: http://en.trialcommand.com
*/
#include <WiFi.h>

const char* ssid = "****************";
const char* password = "********************";
int ModbusTCP_port = 502;

//////// Required for Modbus TCP / IP /// Requerido para Modbus TCP/IP /////////
#define maxInputRegister 20
#define maxHoldingRegister 20

#define MB_FC_NONE 0
#define MB_FC_READ_REGISTERS 3 //implemented
#define MB_FC_WRITE_REGISTER 6 //implemented
#define MB_FC_WRITE_MULTIPLE_REGISTERS 16 //implemented
//
// MODBUS Error Codes
//
#define MB_EC_NONE 0
#define MB_EC_ILLEGAL_FUNCTION 1
#define MB_EC_ILLEGAL_DATA_ADDRESS 2
#define MB_EC_ILLEGAL_DATA_VALUE 3
#define MB_EC_SLAVE_DEVICE_FAILURE 4
//
// MODBUS MBAP offsets
//
#define MB_TCP_TID 0
#define MB_TCP_PID 2
#define MB_TCP_LEN 4
#define MB_TCP_UID 6
#define MB_TCP_FUNC 7
#define MB_TCP_REGISTER_START 8
#define MB_TCP_REGISTER_NUMBER 10

byte ByteArray[260];
unsigned int MBHoldingRegister[maxHoldingRegister];

WiFiServer MBServer(ModbusTCP_port);

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());
}
void setup() {
  pinMode(14, OUTPUT);
  Serial.begin(9600);
  delay(100) ;

  setup_wifi();

  MBServer.begin();
  Serial.println("Connected ");
  Serial.print("ESP8266 Slave Modbus TCP/IP ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(String(ModbusTCP_port));
  Serial.println("Modbus TCP/IP Online");
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    setup_wifi();
  }

  // Check if a client has connected // Modbus TCP/IP
  WiFiClient client = MBServer.available();
  if (!client) {
    return;
  }

  boolean flagClientConnected = 0;
  byte byteFN = MB_FC_NONE;
  int Start;
  int WordDataLength;
  int ByteDataLength;
  int MessageLength;

  // Modbus TCP/IP
  while (client.connected()) {
    if (client.available())
    {
      flagClientConnected = 1;
      int i = 0;
      while (client.available())
      {
        ByteArray[i] = client.read();
        i++;
      }
      client.flush();

      MBHoldingRegister[0] = random(25, 30);
      MBHoldingRegister[1] = random(70, 80);

      //// end code - fin
      //// rutine Modbus TCP
      byteFN = ByteArray[MB_TCP_FUNC];
      Start = word(ByteArray[MB_TCP_REGISTER_START], ByteArray[MB_TCP_REGISTER_START + 1]);
      WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
    }

    // Handle request
    switch (byteFN) {
      case MB_FC_NONE:
        break;
      case MB_FC_READ_REGISTERS: // 03 Read Holding Registers
        ByteDataLength = WordDataLength * 2;
        ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
        ByteArray[8] = ByteDataLength; //Number of bytes after this one (or number of bytes of data).
        for (int i = 0; i < WordDataLength; i++)
        {
          ByteArray[ 9 + i * 2] = highByte(MBHoldingRegister[Start + i]);
          ByteArray[10 + i * 2] = lowByte(MBHoldingRegister[Start + i]);
        }
        MessageLength = ByteDataLength + 9;
        client.write((const uint8_t *)ByteArray, MessageLength);
        byteFN = MB_FC_NONE;
        break;
      case MB_FC_WRITE_REGISTER: // 06 Write Holding Register
        MBHoldingRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
        ByteArray[5] = 6; //Number of bytes after this one.
        MessageLength = 12;
        client.write((const uint8_t *)ByteArray, MessageLength);
        byteFN = MB_FC_NONE;
        break;
      case MB_FC_WRITE_MULTIPLE_REGISTERS: //16 Write Holding Registers
        ByteDataLength = WordDataLength * 2;
        ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
        for (int i = 0; i < WordDataLength; i++)
        {
          MBHoldingRegister[Start + i] = word(ByteArray[ 13 + i * 2], ByteArray[14 + i * 2]);
        }
        MessageLength = 12;
        client.write((const uint8_t *)ByteArray, MessageLength);
        byteFN = MB_FC_NONE;
        break;
    }
  }
}
