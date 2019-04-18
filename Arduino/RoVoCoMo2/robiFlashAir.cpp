#include "robiFlashAir.h"

#include <WiFi.h>
#include <FS.h>
#include <SD.h>

const char* ssid     = "micfa03";
const char* password = "qfsw7028";
String host = "flashair";

String BOUNDARY_STR="========================";
byte upViceData[]={ 0xFE,0xFF,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }; 
unsigned long totalseconds;
String upTimestring="";

void writeFile(fs::FS &fs, const char * path, byte * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.write(message,16)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void saveRemoteLog(int id) {
  upViceData[2]=id&0xFF;
  upViceData[3]=(id>>8)&0xFF;
  writeFile(SD,"/REMOTE.LOG",upViceData);
}

void createTimeStamp() {
  int year=2019;// = (year()-1970) << 9;//(dt.getFullYear() - 1980) << 9;
  int month=1;// = month() << 5;//(dt.getMonth() + 1) << 5;
  int date=1;// = day();//dt.getDate();
  
  unsigned long totalseconds=millis()/1000;
  int hours=totalseconds/3600; // = hour() << 11;//dt.getHours() << 11;
  int minites=(totalseconds-hours*3600)/60; // = minute() << 5;//dt.getMinutes() << 5;
  int seconds=totalseconds-hours*3600-minites*60;// = second();//Math.floor(dt.getSeconds() / 2);
  upTimestring = "0x" + String(year + month + date,16) + String(hours + minites + seconds,16);
}

void sendRemoteLog(int id) {
  
  //Remote.log保存
  saveRemoteLog(id);
  
  //タイムスタンプ
  //createTimeStamp();
  
  //Remote.logファイルオープン
  File file = SD.open("/REMOTE.LOG", FILE_READ);

  String start_request = "--"+BOUNDARY_STR+"\r\n" + 
    "Content-Disposition: form-data; name=\"file\"; filename=\"REMOTE.LOG\"\r\n" +
    "Content-Transfer-Encoding: binary\r\n\r\n";
  String end_request = "\r\n--"+BOUNDARY_STR+"--\r\n";
  String close_request = "Connection: close\r\n\r\n";
  
  uint16_t full_length;
  full_length = start_request.length() + file.size() + end_request.length();
  
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host.c_str(), httpPort)) {
    Serial.println("Connected FILED!");
    return;
  }
  Serial.println("Connected ok!");

  Serial.println("");
  Serial.println("Send REMOTE.LOG file");
  client.print(String("POST ")+
    "/upload.cgi HTTP/1.1\r\n"+ //?UPDIR=/ADDON
    "Host: "+host+"\r\n"+ 
    //"Connection: keep-alive\r\n"+ 
    //"Connection: close\r\n\r\n");
    //"User-Agent: RoVoCoMo2\r\n"+ 
    "Content-Type: multipart/form-data; boundary="+BOUNDARY_STR+"\r\n"+ 
    "Content-Length: "+String(full_length)+"\r\n\r\n");

  client.print(start_request);
  while (file.available()){
    client.write(file.read());
    Serial.print(">");
  }
  Serial.println("");
  Serial.println(">>><<<");
  client.print(end_request);
  //client.print(close_request);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void changeRemoteLogDir() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host.c_str(), httpPort)) {
    Serial.println("Connected FILED!");
    return;
  }
  Serial.println("Connected ok!");
  
  Serial.println("");
  Serial.println("Change dirctory to /ADDON");
  client.print(String("GET ")+
    "/upload.cgi?UPDIR=/ADDON HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
}

void setWriteProtect(int onoff) {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host.c_str(), httpPort)) {
    Serial.println("Connected FILED!");
    return;
  }
  Serial.println("Connected ok!");

  String onoffstr[]={"ON","OFF"};
  
  Serial.println("");
  Serial.println("Write protect"+onoffstr[onoff]);
  client.print(String("GET ")+
    "/upload.cgi?WRITEPROTECT="+onoffstr[onoff]+" HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
}

void deleteRemoteLog() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host.c_str(), httpPort)) {
    Serial.println("Connected FILED!");
    return;
  }
  Serial.println("Connected ok!");

  Serial.println("");
  Serial.println("Delete file../ADDON/REMOTE.LOG");
  client.print(String("GET ")+
    "/upload.cgi?DEL=/ADDON/REMOTE.LOG HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
}
