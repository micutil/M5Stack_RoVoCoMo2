#include "LoadApData.h"

#include <M5Stack.h>
#include <Preferences.h>
#include <WiFi.h>
#include <FS.h>

//SD

//#define useSPIFFS
#ifdef useSPIFFS
  #include <SPIFFS.h>
extern fs::SPIFFSFS qbFS;// = SPIFFS;
#endif

#define hasSD
#ifdef hasSD
  #include <SD.h>
extern fs::SDFS qbFS;// = SD;
#endif

String apdata[]={ "", "" };

bool loadApData(fs::FS &fs, const char * path) {
  
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return false;
  }
  
  int   i,n=0;
  char  v;
  apdata[0]="";
  apdata[1]="";
  while(file.available())
  {
    //Serial.write(file.read());
    v=(char)file.read();
    if(v==','||v==0x09||v==0x0A||v==0x0D) {
      Serial.println("");
      n=n+1;
    } else if(n<2) {
      Serial.print(v);
      apdata[n].concat(v);
    }
  }
  file.close();
  Serial.println("");
  
  //Connect to AP
  if(apdata[0].length()>0&&apdata[1].length()>0) return true;
  
  apdata[0]="";
  apdata[1]="";
  return false;
}

bool SetPreferFileAp(int type) {
  Preferences preferences;
  bool isAp=false;
  
  switch(type) {
    case 0: //ソフト アクセスポイント
      if(loadApData(qbFS,"/sftap.txt")) {
        preferences.begin("soft-ap-config");
        preferences.putString("SOFT_AP_SSID", apdata[0]);
        preferences.putString("SOFT_AP_PASSWD", apdata[1]);
        preferences.end();
        isAp=true;
      } else {
        preferences.begin("soft-ap-config");
        apdata[0] = preferences.getString("SOFT_AP_SSID");
        apdata[1] = preferences.getString("SOFT_AP_PASSWD");
        preferences.end();
        if(apdata[0].length()>0 && apdata[1].length()>0) isAp=true;
      }
      break;
    
    case 1: //ステーションモード アクセスポイント
      if(loadApData(qbFS,"/staap.txt")) {
        preferences.begin("wifi-config");
        preferences.putString("WIFI_SSID", apdata[0]);
        preferences.putString("WIFI_PASSWD", apdata[1]);
        preferences.end();
        isAp=true;
      } else {
        preferences.begin("wifi-config");
        apdata[0] = preferences.getString("WIFI_SSID");
        apdata[1] = preferences.getString("WIFI_PASSWD");
        preferences.end();
        if(apdata[0].length()>0 && apdata[1].length()>0) isAp=true;
      }
      break;
    
    case 2: //FlashAir アクセスポイント
      if(loadApData(qbFS,"/flaap.txt")) {
        preferences.begin("flashair-ap-config");
        preferences.putString("FLASHAIR_AP_SSID", apdata[0]);
        preferences.putString("FLASHAIR_AP_PASSWD", apdata[1]);
        preferences.end();
        isAp=true;
      } else {
        preferences.begin("flashair-ap-config");
        apdata[0] = preferences.getString("FLASHAIR_AP_SSID");
        apdata[1] = preferences.getString("FLASHAIR_AP_PASSWD");
        preferences.end();
        if(apdata[0].length()>0 && apdata[1].length()>0) isAp=true;
      }
      break;
  }
  return true;
}

bool connectWiFi(int type) {
  if(apdata[0].length()>0 && apdata[1].length()>0) {
    //Serial.print("Connect to ");Serial.println(apdata[0]);
    switch(type) {
      case 0:
        WiFi.softAP(apdata[0].c_str(),apdata[1].c_str());
        break;
      case 1:
      case 2:
        Serial.println();Serial.println();
        Serial.println(apdata[0].c_str()); M5.Lcd.println(apdata[0]);
        Serial.print("Connecting"); M5.Lcd.print("Connecting");
        WiFi.begin(apdata[0].c_str(),apdata[1].c_str());
        
        int64_t timeout = millis() + 5000;
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print("."); M5.Lcd.print(".");
          if (timeout - millis() < 0) return false;
        }
        Serial.println(""); M5.Lcd.println("");
        Serial.println("WiFi connected"); M5.Lcd.println("WiFi connected");
        Serial.println("IP address: "); M5.Lcd.println("IP address: ");
        Serial.println(WiFi.localIP()); M5.Lcd.println(WiFi.localIP());
        break;
    }
    return true;
  }
  return false;
}

bool beginWiFi(int type)
{
	Preferences preferences;
	
	switch(type) {
		case 0://Soft Ap
			preferences.begin("soft-ap-config");
			apdata[0] = preferences.getString("SOFT_AP_SSID");
			apdata[1] = preferences.getString("SOFT_AP_PASSWD");
			break;
		case 1://Station Ap
			preferences.begin("wifi-config");
			apdata[0] = preferences.getString("WIFI_SSID");
			apdata[1] = preferences.getString("WIFI_PASSWD");
			break;
		case 2://FlashAir
			preferences.begin("flashair-ap-config");
			apdata[0] = preferences.getString("FLASHAIR_AP_SSID");
			apdata[1] = preferences.getString("FLASHAIR_AP_PASSWD");
			break;
	}

  if(apdata[0].length()<=0 || apdata[1].length()<=0) {
    SetPreferFileAp(type);
  }
	
	return connectWiFi(type);
  
}

bool beginFlashAir() {
  //Load setting from file or prefer
  bool ds=SetPreferFileAp(FLASHAIR_AP_NUM);
  int maxID=-1;
  int maxInt=0;

  //Scan wifi
  Serial.println("WiFi scan start"); M5.Lcd.println("WiFi scan start");
  int n = WiFi.scanNetworks();

  //Check ssid and rssi
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
      return false;
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      String wifissid=WiFi.SSID(i); Serial.print(wifissid);Serial.print(":");
      int rssi=WiFi.RSSI(i); Serial.println(rssi);

      //Whether is ssid of priset exit.
      if(ds&&wifissid.equals(apdata[0])) {
        Serial.print("Target FlashAir:");Serial.println(apdata[0]);
        return connectWiFi(FLASHAIR_AP_NUM);
      }

      //Pick up ssid which is minimum intensity in flashair card
      if(wifissid.startsWith("flashair_")) {
        if(rssi<maxInt) {
          maxID=i;
          maxInt=rssi;
        }
      }
    } //end for i

    //Connect to flashair
    if(maxID>=0) {
      apdata[0] = WiFi.SSID(maxID);
      apdata[1] = "12345678";
      return connectWiFi(FLASHAIR_AP_NUM);
    }
  }
  return false;
}
