#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#define VSD_RESET_PIN D5

SoftwareSerial mySerial(D1,D2);   // RX , TX

ESP8266WebServer server;

char* ssid = "vpb";
char* password = "password";

// hold uploaded file
File fsUploadFile;


byte data[] = {
  0x57,   // Uart_Sync_Head1
  0xab,   // Uart_Sync_Head2
  0x83,   // IAP end command
  0x00,   // length
  0x00,   // Rev 0
  0x00,   // Rev 1
  0x83   // Checksum
};

byte data2[] = {
  0x57,   // Uart_Sync_Head1
  0xab,   // Uart_Sync_Head2
  0x81,   // IAP chip erase command
  0x00,   // length
  0x00,   // Rev 0
  0x00,   // Rev 1
  0x81   // Checksum
};

byte data3[64];

byte checksum = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  mySerial.begin(38400);
  if(!SPIFFS.begin()){
    Serial.println("An error has occurred while mounting the SPIFFS");  
    return;
  }
  pinMode(VSD_RESET_PIN, OUTPUT);
  
  Serial.println("\nClearing RX buffer");
  delay(1000);
  while(mySerial.available() > 0)
      Serial.println(mySerial.read());
  reset_vsdsm();
  delay(250);
  iap_chip_erase();
  delay(250);

  File file = SPIFFS.open("/blink_fast.bin", "r");
  if(!file){
      Serial.println("Failed to open file for reading");
      return;
  }

  Serial.println();
  uint32_t count = 0;
  while(file.available()){
    for(int i = 0; i < 64; i++){
      data3[i] = (file.available()) ? file.read() : 0xFF;
    }
    iap_program();
  }
  file.close();
  
  delay(250);
  iap_end();
}

void loop() {
  // put your main code here, to run repeatedly:

}

// Chip erase command
void iap_chip_erase() {
  Serial.println("Starting Chip erase command");
  Serial.println("Sending commands");
  for(int i = 0; i < 7; i++){
    mySerial.write(data2[i]);
    Serial.print(data2[i],HEX);
    Serial.print(" ");
    delay(50);
  }
  Serial.println();
  delay(100);
  Serial.print("Response: ");
  if (mySerial.available())
      Serial.print(mySerial.read());
  Serial.println("\nChip erase command successful !");
}

// IAP Program command
void iap_program() {
  Serial.println("Starting Chip program command");
  Serial.println("Sending commands");
  mySerial.write(0x57); delay(50);
  mySerial.write(0xab); delay(50);

  mySerial.write(0x80); delay(50);  // cmd
  mySerial.write(0x40); delay(50);  // len
  mySerial.write((byte) 0x00); delay(50);  // rev0
  mySerial.write((byte) 0x00); delay(50);  // rev1
  
  checksum = 0x80 + 0x40;
  for(int i = 0; i < 64; i++){
    mySerial.write(data3[i]);
    Serial.print(data3[i], HEX);
    Serial.print(" ");
    checksum += data3[i];
  }

  // Send checksum
  mySerial.write(checksum);
  Serial.println();
  Serial.print("Checksum: ");
  Serial.println(checksum, HEX);
  
  delay(200);

  Serial.print("Response: ");
  if (mySerial.available())
      Serial.print(mySerial.read());

  Serial.println("\nChip program command successful !");
  
}

// IAP end command
void iap_end(){
  Serial.println("Starting IAP end command");
  Serial.println("Sending commands");
  for(int i = 0; i < 7; i++){
    mySerial.write(data[i]);
    Serial.print(data[i],HEX);
    Serial.print(" ");
    delay(50);
  }
  Serial.println();
  delay(100);
  Serial.print("Response: ");
  if (mySerial.available())
      Serial.print(mySerial.read());

  Serial.println("\nIAP end command successful !");
}

// Resetting the VSDSM board
void reset_vsdsm(){
  Serial.println("Resetting VSDSquadron-Mini board");
  digitalWrite(VSD_RESET_PIN, LOW);
  delay(500);
  digitalWrite(VSD_RESET_PIN, HIGH);
  Serial.println("Done resetting");
}

// Flash
void flash_file()
