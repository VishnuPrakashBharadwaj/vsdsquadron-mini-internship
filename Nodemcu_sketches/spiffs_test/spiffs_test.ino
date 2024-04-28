#include "FS.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);

  if(!SPIFFS.begin()){
    Serial.println("An error has occurred while mounting the SPIFFS");  
    return;
  }

    
  Serial.println("\n\n---BEFORE REMOVING---");
  listAllFiles();
  /*
  File file = SPIFFS.open("/blink_slow.bin", "r");
  if(!file){
      Serial.println("Failed to open file for reading");
      return;
  }

  Serial.println();
  Serial.println("File Contents: ");
  uint32_t count = 0;
  while(file.available()){
    Serial.print (file.read(), HEX);
    Serial.print(" ");
    count++;
    if(count % 16 == 0)
      Serial.println();
  }
  file.close();
  */
}

void loop() {
  // put your main code here, to run repeatedly:

}

void listAllFiles(){
 
  String path = "/";
  Serial.println("------------FILES------------");
  Dir dir = SPIFFS.openDir(path);
  while(dir.next()){
    File entry = dir.openFile("r");
    Serial.println(String(entry.name()).substring(1));
    entry.close();
  }
 
}
