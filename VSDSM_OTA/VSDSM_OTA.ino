#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <FS.h>   // Include the SPIFFS library

#define VSD_RESET_PIN D5
SoftwareSerial mySerial(D1,D2);   // RX , TX

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

File fsUploadFile;              // a File object to temporarily store the received file
String file_name = "";

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

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  mySerial.begin(38400);
  pinMode(VSD_RESET_PIN, OUTPUT);
  digitalWrite(VSD_RESET_PIN, HIGH);
  delay(10);
  Serial.println('\n');

  wifiMulti.addAP("vpb", "password");   // add Wi-Fi networks you want to connect to

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  SPIFFS.begin();                           // Start the SPI Flash Files System

  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    if (!handleFileRead("/upload.html"))                // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.on("/upload", HTTP_POST,                       // if the client posts to the upload page
    [](){},
    handleFileUpload                                    // Receive and save the file
  );

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.on("/list", handleFileList);
  server.on("/flash_vsd", handleFileName);

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleFileName(){
  file_name = "/" + server.arg("file_name");
//  server.send(200,"text/plain", file_name);
  server.send(200, "text/plain", file_name + " is being flashed. Please do not refresh the browser.");
  delay(2000);
  flash_file();
}

// Handle File lists
void handleFileList(){
  String path = "/";
  uint8_t count = 0;
  Dir dir = SPIFFS.openDir(path);
  String output = "<!DOCTYPE html>";
  output += "<html><head><title> Files </title></head><body>";
  output += "<h3>Select a file to flash :</h3>";
  output += "<form method=\"get\" action=\"/flash_vsd\">";
  while(dir.next()){
    File entry = dir.openFile("r");
    if(String(entry.name()).substring(1).endsWith(".bin")){
      count += 1;
      
      output += "<input type=\"radio\" id=\"file"+ String(count)+"\" name=\"file_name\" value=\"";
      output += String(entry.name()).substring(1);
      output += "\">";
      output += "<label for=\"file"+ String(count) +"\"> " + String(entry.name()).substring(1)+ "</label><br>";
    }
    entry.close();
  }

  output += "<input type=\"submit\" value=\"Submit\"></form></body></html>";
  server.send(200, "text/html", output);
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path = pathWithGz;                                      // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      Serial.println("File upload failed");
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
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
void flash_file(){
  File file = SPIFFS.open(file_name, "r");
  if(!file){
      Serial.println("Failed to open file for reading");
      iap_end();
      return;
  }
  
  Serial.println("\nClearing RX buffer");
  delay(1000);
  while(mySerial.available() > 0)
      Serial.println(mySerial.read());
  reset_vsdsm();
  delay(250);
  iap_chip_erase();
  delay(250);

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
  delay(500);

  server.sendHeader("Location","/success2.html");      // Redirect the client to the success page
  server.send(303);
}
