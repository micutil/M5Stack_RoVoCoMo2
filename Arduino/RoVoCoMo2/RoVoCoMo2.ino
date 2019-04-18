/*******************************************************************
   RoVoCoMo 2 : Robi Voice Controller by Micono
   for M5Stack

   ver 1.7: 2019/ 4/14 : ロビ２のココロ対応
   ver 1.6: 2019/ 3/28 : 認識語リスト外部ファイル化
   ver 1.5: 2019/ 3/ 8 : ロビライドの操作性ほか
   ver 1.4: 2019/ 3/ 5 : FlashAir対応
   ver 1.3: 2019/ 2/24 : 50音索引化
   ver 1.2: 2019/ 1/27
   ver 1.1: 2019/ 1/25
   ver 1.0: 2019/ 1/ 6

   CC BY-NC-ND
   by Micono Utilities (Michio Ono)
      robi.micutil.com

 *******************************************************************/

/*******************************************************************
   refer to PlayMP3FromSDToDAC for play mp3 file in microSD
*/
/*
  cd ~/Arduino/libraries
  git clone https://github.com/earlephilhower/ESP8266Audio
  git clone https://github.com/Gianbacchio/ESP8266_Spiram
*/

/*******************************************************************

   BLE Advertise
   Modified lucascoelhof/ESP32BleAdvertise
   https://github.com/lucascoelhof/ESP32BleAdvertise/blob/master/examples/SimpleBleAdvertise/SimpleBleAdvertise.ino

*/

/*******************************************************************

    M5Stackで日本語表示
    https://qiita.com/taront/items/7900c88b9e9782c33b08

    茶虎たま吉さんのArduino-KanjiFont-Library-SD
    https://github.com/Tamakichi/Arduino-KanjiFont-Library-SD

*/

#define isM5Stack //M5Stack
//#define isESP32

#ifdef isESP32
  #define isESP32_OLED
  //#define isESP32_OLED_SD
#endif

#ifdef isM5Stack
  #pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
  #include <M5Stack.h>
  #include "M5StackUpdater.h" //for SD updater
  #define useGo10on //50音操作　by CRAFT OYAJI
  #define useAUDIO 
  #define useWiFi
  #define useFACES
  #define hasSD
#endif

#ifdef isESP32 || isESP32_OLED || isESP_OLED_SD
  #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
  #define useWiFi
  #ifndef isESP_OLED_SD
    #define useSPIFFS
  #endif
#endif

#ifdef isESP32_OLED || isESP_OLED_SD
  #define hasOLED
#endif

#ifdef isESP_OLED_SD
  #define useAUDIO
  #define hasSD
#endif

#ifdef hasOLED
  #include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
  #include "OLEDDisplayUi.h"  // Include the UI lib
  // Initialize the OLED display using Wire library
  SSD1306Wire  display(0x3c, 5, 4); //D3, D5);
  OLEDDisplayUi ui  ( &display );
#endif //hasOLED

#include <WiFi.h>

#define useBluetooth
#ifdef useBluetooth
  #define forRobi2
#endif

bool enableFlashAir=false;
#ifdef useWiFi
  //#define useWebServer
  #define useFlashAir
  #ifdef useFlashAir
    #define forRobi1
    #ifndef forRobi2
      #define forRobi2
    #endif
  #endif
#endif

#ifdef useWebServer
  //always false, now...
  bool showQRC = false;
#else //not useWebServer
  bool showQRC = false;
#endif  //useWebServer

#ifdef useWiFi
  bool wifiEnable = false;

  #ifdef useWebServer
    #include <WebServer.h>
    #include "LoadApData.h"
    WebServer server(80);
    boolean isServer = false;
    
    String rootPage;
    /*** SoftAP ***/
    IPAddress mySoftAPIP;
    IPAddress myStaAPIP;
    //#define useMDNS
    #ifdef useMDNS
      #include <ESPmDNS.h>
      const char* mjname = "micrvc";
    #endif //useMDNS
  #endif

  #ifdef useFlashAir
    #include "LoadApData.h"
    #include "robiFlashAir.h"
  #endif
  
#endif //useWiFi

//Bluetooth
#include "QESP32BleAdvertise.h"

//Font
#ifdef isM5Stack
  #define useJPFONT
  #ifdef useJPFONT
    #include "DispJPFont.h"
    #define SD_PN 4
  #endif
#endif

//Audio
#ifdef useAUDIO
  #include "AudioFileSourceSD.h"
  #include "AudioOutputI2S.h"

  //#define useMP3
  #ifdef useMP3
    #include "AudioFileSourceID3.h"
    #include "AudioGeneratorMP3.h"
    AudioGeneratorMP3 *agen;//*mp3;
    AudioFileSourceID3 *id3;
  #else
    #include "AudioGeneratorWAV.h"
    AudioGeneratorWAV *agen;//*wav;
  #endif //useMP3 or WAV

  AudioFileSourceSD *afile;
  AudioOutputI2S *aout;
  
  //Initial speaker volue level
  float spkvol = 0.1;
  bool voicePlay = false;
  bool waitVoice = false;
  bool enableAudio = false;

#else

#endif //useAUDIO

//For Faces
bool startkey = false;
//We can use the faces keyboard only when the voice function is off...
#ifdef useFACES
  #define KEYBOARD_I2C_ADDR     0x08
  #define KEYBOARD_INT          5
#endif

//SD
#ifdef useSPIFFS
  #include <SPIFFS.h>
  fs::SPIFFSFS qbFS = SPIFFS;
#endif

#ifdef hasSD
  fs::SDFS qbFS = SD;
#endif

//BLE
SimpleBLE ble;

byte qboMD[19] = {0, 0, 0, 'D', 'e', 'A', 'G', 'O', 'S', 'T', 'I', 'N', 'I', ' ', 'Q', '-', 'b', 'o'};
byte fileMD[5] = {0x20, 0, 0, 0, 0};

int fileid = 0x482; //Ninshikigo (0x482)
//int fileid=0x2AC; //Last page of Robi English Book 1 (0x2AD)
int target = fileid;
int tgtid = 0;
int voiceID = 22; //ゼロベース

//Send data
int dataType = 0;
int counter = 0;

int rapid = 0;
unsigned long lastrap;
unsigned long nowrap;
unsigned long rapx[] = {100, 100}; //{3000,27000};//

//認識語Voiceデータ名
int voiceMax; //登録数
#define MAX_ID 340
#define MAX_ORDER 32
#define MAX_NEXT 16

String voiceData[MAX_ID];
String voiceFile[MAX_ID];
int ninshikiID[MAX_ID];
int nextData[MAX_ID];
int nextOrder[MAX_NEXT][MAX_ORDER];
int maxOrder[MAX_NEXT];
unsigned int continueOrder[MAX_NEXT];
//String voiceFolder = "voice/Ninshiki";
int openingVoice = 87;
/*
String voiceData[] = {
  "もしもし", "ねぇねぇ聞いて", "ねえロビ君", "ねえ聞いて", "聞いて聞いて", "ロビ君", "ロビちゃん", "ロボ君",
  "ロボット君", "ロボちゃん", "名前は？", "お名前は？", "名前は何？", "自己紹介して", "何ができるの", "特技は？",
  "男の子？", "何歳なの？", "身長は？", "体重は？", "値段は？", "準備はいい？", "あいさつして", "はじめまして",
  "おはよう", "こんにちは", "こんばんは", "おやすみ", "いってきます", "いってくるね", "まだやる", "じゃあまたね",
  "ロビ君またね", "また今度ね", "さようなら", "バイバイ", "ただいま", "帰ったよ", "元気？", "お疲れ様",
  "ご苦労様", "どうだった？", "＜音無＞", "スタート", "開始", "始め", "ストップ", "終了",
  "終わりにしよ", "終わりにする", "止まれ", "止まって", "もういいよ", "おしまい", "ちょっと待って", "前に進んで",
  "歩いてきて", "前に来て", "こっちへおいで", "こっちにおいで", "こっち来て", "こっちに来て", "近くに来て", "こっち向いて",
  "左に来て", "左に行って", "左向いて", "右に来て", "右に行って", "右向いて", "起きてよ", "起きて",
  "立ち上がって", "立って", "座って", "座ってて", "寝転がって", "寝転んで", "後ろ下がって", "後ろに下がって",
  "シュート", "蹴って", "うちわであおいで", "ふりかけ", "ふりかけかけて", "ミュージックスタート", "よかったね", "楽しいね",
  "楽しかったね", "うれしいね", "かしこいね", "かわいいね", "おもしろいね", "おもしろかったねえ", "かっこいいね", "困ったね",
  "困ったよ", "悲しいね", "つらいね", "つらいよ", "悲しいよ", "疲れたよ", "疲れたね", "なぐさめて",
  "励まして", "元気づけて", "いいことがあったよ", "嫌なことがあった", "合格したよ", "だめだった", "誕生日なんだ", "記念日なんだ",
  "クリスマスだよ", "テストなんだ", "試合なんだ", "デートなんだ", "旅行行くんだ", "かぜひいちゃった", "ケンカしたんだ", "飲みすぎちゃった",
  "応援して", "お正月だよ", "晴れるね", "晴れてるね", "雨降るね", "雨降ってるね", "今日暑いね", "今日寒いね",
  "いい天気だね", "テレビつけて", "テレビ見たい", "テレビ見ようか", "テレビ消して", "チャンネル変えて", "チャンネル切替", "音大きくして",
  "音量大きく", "声大きくして", "音小さくして", "音量小さく", "声小さくして", "ダンスして", "ダンスしようよ", "踊って",
  "いっしょに踊ろう", "おそうじして", "きれいにして", "ココロボ", "ココロボにそうじさせて", "タイマー", "アラーム", "目覚まし",
  "時間計って", "１分", "３分", "５分", "７分", "１０分", "１５分", "遊ぼう",
  "遊んで", "いっしょに遊んで", "いっしょに遊ぼう", "何かして", "おもしろいこといって", "ゲーム", "ゲームする", "ゲームしよう",
  "バランスゲーム", "クイズ出して", "クイズする", "クイズ", "ものまね", "ものまねして", "腕立て伏せ", "腹筋運動",
  "歌うたって", "サッカーしよう", "サッカーして", "旗上げ", "旗上げゲームして", "占いして", "占って", "ＣＭやって",
  "コマーシャルやって", "じゃんけん", "じゃんけんしよ", "セキュリティモード", "防犯モード", "お留守番", "留守番して", "ロビ危機一髪",
  "今日の運勢は？", "今日のラッキーカラーは?", "写真撮るよ", "はいチーズ", "はいポーズ", "メッセージモード", "バッテリィー大丈夫？", "電池だいじょうぶ？",
  "充電する？", "充電だいじょうぶ？", "充電しよう", "お腹すいた？", "お腹すいてない？", "眠くない？", "疲れてない？", "もう寝よう",
  "高橋智隆", "エボルタって知ってる？", "キロボって知ってる？", "デアゴスティーニ", "ロボゼロって知ってる？", "デアゴスティーニ", "はい", "いいえ",
  "好きだよ", "嫌いだよ", "どっちでもない", "勝った", "負けた", "あいこ", "いいよ", "オッケー",
  "了解", "分かった", "違うよ", "何か言った？", "何て言った？", "もう一回言って", "何でもない", "おめでとう",
  "ありがとう", "ごめんね", "助かったよ", "がんばったね", "だいじょぶだよ", "だいじょうぶ", "早く帰るよ", "遅くなる",
  "遅くなるよ", "ごはん", "ごはん食べる", "ごはんにする", "お風呂", "お風呂入る", "上手だね", "友だちになって",
  "友だちだよ", "大好き", "トランフォーム", "おはようさん", "言葉の練習", "わかる言葉教えて", "なんでやねん", "ちがうだろ",
  "ロボットのこと教えて", "うまくいったよ", "失敗しちゃった", "伝言モード", "メッセージ伝えて", "好きな人ができた", "ダイエットしなきゃ", "太っちゃた",
  "食べ過ぎちゃった", "今月ピンチ", "すごかったね", "すごく上手だね", "一杯付き合ってよ", "いっしょに飲もうよ", "私のこと好き？", "私のことどう思う？",
  "一杯付き合ってよ", "いっしょに飲もうよ", "スイッチオン", "スイッチオフ"
};
int voiceMax = 276; //登録数
*/
#ifdef useGo10on //50音検索
int characterID[60]; 
int voiceOrder0[MAX_ID];

String characterData[] = {
  "あかさたなはまやらがざだ", "いきしちにひみいりぎじで", "うくすつぬふもゆるぐずど", "えけせてねへめえろげぜば", "おこそとのほもよわごぞぼ"
};
int characterXMax = 12;
/*
int characterID[] = {  
  0, 9, 22, 31, 32, 63, 72, 81, 85, 87,     //あ-こ
  103, 107, 114, 124, 125, 125, 135, 142, 147, 152, //さ-と
  158, 169, 169, 169, 176, 177, 188, 191, 195, 195, //な-ほ
  195, 200, 204, 204, 207, 213, 213, 213, 213, 213, //ま-よ
  214, 214, 216, 217, 226, 230, 231, 231, 231, 236, //ら-ご
  243, 243, 255, 255, 255, 255, 262, 266, 268, 271  //ざ-ぼ
};
int voiceOrder0[] = {
  22, 56, 232, 125, 150, 159, 124, 160, 221, 28, 128, 106, 107, 272, 29, 162, 161, 144, 273, 222, 215, 153, 176, 181, 174, 78, 89, 257, 82, 182, 79, 209, 70, 143, 164, 27, 121, 11, 39, 24, 135, 138, 120, 16, 231, 251, 92, 93,
  189, 71, 240, 204, 53, 239, 223, 203, 244, 48, 145, 136, 139, 49, 245, 37, 91, 90, 117, 94, 97, 100, 44, 219, 192, 193, 210, 4, 111, 126, 127, 217, 146, 169, 112, 170, 171, 118, 81, 25, 26, 184, 61, 63, 95, 96, 265, 137, 140,
  252, 148, 59, 58, 60, 147, 34, 177, 178, 154, 183, 194, 18, 114, 258, 80, 47, 74, 267, 266, 261, 216, 275, 274, 43, 46, 75, 187, 72, 110, 149, 36, 19, 208, 234, 87, 73, 88, 133, 54, 264, 62, 255, 226, 134, 206, 102, 98, 101,
  99, 113, 129, 132, 131, 130, 15, 250, 247, 248, 51, 50, 10, 163, 14, 17, 103, 227, 12, 230, 254, 228, 156, 3, 20, 205, 2, 1, 77, 76, 119, 23, 180, 238, 123, 104, 195, 122, 179, 196, 45, 214, 65, 66, 64, 175, 263, 84, 83, 55,
  30, 33, 57, 220, 85, 68, 69, 67, 151, 260, 197, 173, 229, 207, 0, 52, 172, 86, 116, 224, 190, 191, 5, 32, 256, 212, 6, 7, 9, 8, 253, 270, 271, 225, 235, 167, 38, 105, 165, 166, 108, 40, 233, 241, 242, 243, 155, 13, 186, 201,
  152, 21, 246, 31, 185, 202, 200, 157, 158, 262, 237, 141, 249, 109, 142, 199, 259, 115, 211, 41, 236, 218, 198, 35, 168, 188
};
int nextData[] = {
//RR VVVVVVVVVVVVVVVVVVVVVVV
  0,0,0,0,5,7,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,2,2,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,
  0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,5,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,5,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0
//RR ^^^^^^^^^^^^^^^^^^^^^
};

int voiceOrder1[] = {
  44, 47, 43, 53, 52  //開始,終了,スタート,おしまい,もういいよ
};
int voiceOrder2[] = {
  214, 215           //クイズ はい,いいえ
};
int voiceOrder3[] = {
  219, 220, 221, 53  //じゃんけん 勝った,負けた.あいこ.おしまい
};
int voiceOrder4[] = {
  80, 81, 53, 55, 67, 65 //サッカー シュート,蹴って.おしまい,前に進んで,右に行って,左に行って
};
int voiceOrder5[] = {
  153, 154, 155, 156, 158, 159, 53 //タイマー 1分,3分,5分,7分,10分,15分,おしまい
};
int voiceOrder6[] = {
  53, 110, 121, 37, 111, //伝言 おしまい,誕生日なんだ,お正月だよ,帰ったよ,記念日なんだ
  112, 105.108, 114, 113, //クリスマスだよ,元気づけて,合格したよ,試合なんだ,テストなんだ
  109, 115, 103, 104, 94 //だめだった,デートなんだ,なぐさめて,励まして,かっこいいね
};
//RR vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
int voiceOrder7[] = {
  223, 53, 237, 55, 79, 68, 65,59,  //オッケー,おしまい,大丈夫。前に進んで、後ろに下がって、右に行って、左に行って、こっちらにおいで
  22, 14, 94, 141, 163,  //あいさつして,何ができる、かっこいいね。ダンスして、何かして
  13, 199,24,25,26,246,87,88,92,93 //自己紹介して、電池だいじょうぶ、おはよう、こんにちは、こんばんは。上手だね、楽しいね、楽しかったね、面白いね、面白かったね
};
//RR ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/
int voiceDataNo = -1;
int voiceIDSave = 0;
int characterXPos = 0;
int characterYPos = 0;
int *voiceOrder;

#else
int voiceOrder[300];

#endif //useGo10on 終わり

/********************************************************************/
/********************************************************************/
#ifdef useBluetooth
//function takes String and adds manufacturer code at the beginning
void setQboIdManData()
{
  qboMD[2] = (byte)counter;
  ble.advertise(qboMD, 19);
}

//function takes String and adds manufacturer code at the beginning
void setFileIdManData()
{
  fileMD[2] = (byte)counter;
  fileMD[3] = (byte)(target >> 8);
  fileMD[4] = (byte)(target & 0xFF);
  ble.advertise(fileMD, 5);
}
#endif //useBluetooth

/*************************************************
   音声再生
 ************************************************/
#ifdef useAUDIO
#ifdef useMP3
void playVoice(String voicefpn)
{
  afile = new AudioFileSourceSD(voicefpn.c_str());
  if (afile && afile.isOpen()) id3 = new AudioFileSourceID3(afile);
  if (id3) {
    /*
      aout = new AudioOutputI2S(0, 1); // Output to builtInDAC
      aout->SetGain(spkvol/100.0);
      aout->SetOutputModeMono(true);
      mp3 = new AudioGeneratorMP3();
    */
    voicePlay = agen->begin(id3, aout);
    waitVoice = voicePlay;
  }
}
}

#else

void playVoice(String voicefpn)
{
  afile = new AudioFileSourceSD(voicefpn.c_str());
  if (afile && afile->isOpen()) {
    voicePlay = agen->begin(afile, aout);
    waitVoice = voicePlay;
  }
}

#endif //useMP3 or WAV
#endif //useAUDIO

/*************************************************
   再生音声ファイルID番号
 ************************************************/

#ifdef useAUDIO
#ifdef useMP3
void playQboSoundNum(int id) {
  id = id + fileid - 1;
  String fp = "/GMVRC/D231300525000/DS000.MP3";
  char iddecs[6], idfns[14];
  int fdridx;

  //File name
  sprintf(iddecs, "DS%03d", id & 0x7F);
  fp.replace("DS000", iddecs);
  Serial.print("File name="); Serial.println(iddecs);

  //Folder name
  fdridx = 444 + ((id & 0xF80) >> 7);
  sprintf(idfns, "D231300525%03d", fdridx);
  fp.replace("D231300525000", idfns);

  Serial.print("Voice Path="); Serial.println(fp);
  playVoice(fp);
}

#else

void playQboSoundNum(int id) {
/*  
  String fp = "/voice/Ninshiki/NF000.wav"; //"/voice/Ninshiki/NF00.wav"
  char iddecs[8];

  //File name
  if (id < 10) {
    sprintf(iddecs, "NF%02d", id & 0x1FF);
  } else {
    if (id == 43) return;
    if (id == 202) {
      sprintf(iddecs, "NF%d-2", 200);
    } else {
      sprintf(iddecs, "NF%d", id & 0x1FF);
    }
  }
  fp.replace("NF000", iddecs);
*/
//  String fp = "/" + voiceFolder + "/" + voiceFile[id] + ".wav"; 
  String fp = "/voice/" + voiceFile[id] + ".wav"; 
  //Serial.print("File name="); Serial.println(iddecs);
  Serial.print("Voice Path="); Serial.println(fp);
  playVoice(fp);
}

#endif //useMP3 or WAV
#endif //useAUDIO

/****************************************************
   Draw select mode
*/
int selmode = 1; //5=step5, 1=step1, 0=selmode, -1=play
int pressJump = 0;
int smpX = 50;
int smpS = 96;
int smpY = 220;
int fntsz = 12;

void drawSelMode() {
  if (showQRC) selmode = -1;
  //M5.Lcd.fillRect(0, smpY, 320, 20, 0);

#ifdef useJPFONT
  SDfonts.open(); // フォントのオープン
  switch (selmode) {
    case 0:
      fontDump(smpX, smpY, "(5)", fntsz, TFT_WHITE, false);
      fontDump(smpX + smpS, smpY, "再生", fntsz, TFT_GREEN, false);
      fontDump(smpX + smpS * 2, smpY, "(1)", fntsz, TFT_WHITE, false);
      break;
      
    case 1:
     #ifdef useGo10on //50音検索
      fontDump(smpX, smpY, " ↓ ", fntsz, TFT_WHITE, false);
//RR vvvvvvvvvvvvvvvvvv     
      if (voiceDataNo != 0)
         fontDump(smpX + smpS * 2, smpY, "　↑ ", fntsz, TFT_WHITE, false);
     else
//RR ^^^^^^^^^^^^^^^^^     
        fontDump(smpX + smpS * 2, smpY, "索引", fntsz, TFT_CYAN, false);
     #else //not useGo10on
      fontDump(smpX, smpY, " - ", fntsz, TFT_WHITE, false);
      fontDump(smpX + smpS * 2, smpY, " + ", fntsz, TFT_WHITE, false);
     #endif //useGo10on 終わり     
      fontDump(smpX + smpS, smpY, "選択", fntsz, TFT_YELLOW, false);
      break;
      
    case 2: //useGo10on //50音検索
      fontDump(smpX, smpY, "　↓ ", fntsz, TFT_WHITE, false);
      fontDump(smpX + smpS, smpY, "選択", fntsz, TFT_YELLOW, false);
      fontDump(smpX + smpS * 2, smpY, "→　", fntsz, TFT_WHITE, false);
      break;
    
    case 5:
      fontDump(smpX, smpY, " -5", fntsz, TFT_WHITE, false);
      fontDump(smpX + smpS, smpY, "選択", fntsz, TFT_YELLOW, false);
      fontDump(smpX + smpS * 2, smpY, " +5", fntsz, TFT_WHITE, false);
      break;
      
    case -1://QRcode
      fontDump(smpX, smpY, "SFT", fntsz, TFT_WHITE, false);
      fontDump(smpX + smpS, smpY, "OK", fntsz, TFT_YELLOW, false);
      fontDump(smpX + smpS * 2, smpY, "STA", fntsz, TFT_WHITE, false);
      break;
      
  }
  SDfonts.close(); // フォントのクローズ
#endif //useJPFONT

}

/***********************************************************
   Draw id and voice
*/
int posXa = 20;
int posXb = 65;
int posY = 125;

void drawVoiceID(int y, int id) {
  char idstr[4];
  
  sprintf(idstr, "%03d", ninshikiID[voiceOrder[id]]);  
//  sprintf(idstr, "%03d", (voiceOrder[id] + 1) & 0x1FF);

  //String idstr=String(id);

#ifdef useJPFONT
  int fntcol = TFT_BLUE;
  if (id == voiceID) fntcol = TFT_YELLOW;
  //M5.Lcd.fillRect(0, y, 320, 30, 0);
  fontDump(posXa, y + 4, (char*)idstr, 14, TFT_WHITE, false);
  int xp = fontDump(posXb, y, (char*)voiceData[voiceOrder[id]].c_str(), 24, fntcol, false);
  M5.Lcd.fillRect(xp, y, 320, 30, 0);
#endif //useJPFONT

}

void drawListVoiceID() {
  if (showQRC) return;
  //M5.Lcd.fillRect(0, posY, 320, 150, 0);

#ifdef useJPFONT
  int id = voiceID;
  int i;

  SDfonts.open(); // フォントのオープン
  
#ifdef useGo10on //50音検索
  int count;
  if (voiceMax < 5) {
    M5.Lcd.fillRect(0, posY - 60 + voiceMax * 30, 320, 150 - voiceMax * 30, 0);
    count = id + voiceMax - 1;
  }
  else {
    count = id + 4;
  }
  for (i = id; i <= count; i++) {
    if (i < 0) {
      drawVoiceID(posY + 30 * (i - id - 2), voiceMax + i);
    } else if (i >= voiceMax) {
      drawVoiceID(posY + 30 * (i - id - 2), i - voiceMax);
    } else {
      drawVoiceID(posY + 30 * (i - id - 2), i);
    }
  }
  if (voiceMax < 5)
  {
    delay((5-voiceMax)*100);     //タイミング調整
  }
  
#else //not useGo10on //50音検索
  for (i = id - 2; i <= id + 2; i++) {
    if (i < 0) {
      drawVoiceID(posY + 30 * (i - id), voiceMax + i);
    } else if (i >= voiceMax) {
      drawVoiceID(posY + 30 * (i - id), i - voiceMax);
    } else {
      drawVoiceID(posY + 30 * (i - id), i);
    }
  }
  
#endif //useGo10on 終わり

  SDfonts.close(); // フォントのクローズ
  
#endif //useJPFONT

}

#ifdef useGo10on //50音検索
void drawCharacterTable() {
  if (showQRC) return;
  int i, y;
  SDfonts.open(); // フォントのオープン
  for (i = 0; i < 5; i++) {
    y = posY + 30 * i - 60;
    //int xp = fontDump(0, y, (char*) characterData[i].c_str(), 24, TFT_BLUE, false);
    int xp = fontDump(12, y, (char*) characterData[i].c_str(), 24, TFT_BLUE, false);
    M5.Lcd.fillRect(xp, y, 320, 30, 0);
  }
  SDfonts.close(); // フォントのクローズ
}

void drawCharacterPos(int color) {
  if (showQRC) return;
  int x, y, fntcol;
  uint8_t buf[MAXFONTLEN];
  if (color == 1)
    fntcol = TFT_CYAN;
  else
    fntcol = TFT_BLUE;
  String c =  characterData[characterYPos].substring(characterXPos * 3, (characterXPos + 1) * 3);
 // Serial.print("drawCharacterPos="); Serial.print( characterYPos); Serial.print( ","); Serial.print(characterXPos); Serial.print( ","); Serial.println(c);
  SDfonts.open(); // フォントのオープン
  //x = characterXPos * 24;
  x = characterXPos * 24 + 12;
  y = posY + 30 * characterYPos - 60;
  int xp = fontDump(x, y, (char*)c.c_str(), 24, fntcol, false);
  //  M5.Lcd.fillRect(xp, y, 24, 30, 0);
  SDfonts.close(); // フォントのクローズ
}

void changeVoiceData(int no)
{
  if(voiceDataNo==no) return;
  Serial.print("changeVoiceData="); Serial.println(no);
  if (no != 0) {
    voiceIDSave = voiceID;
    voiceID = 0;
  }
  if (no == 0)
  {
     voiceOrder = voiceOrder0;
     voiceID = voiceIDSave;  
  }
  else
  {
    voiceOrder = nextOrder[no - 1];
  }
  voiceMax = maxOrder[no];
 /* 
  switch (no) {
    case 0:
      voiceOrder = voiceOrder0;
      voiceMax = 272;
      voiceID = voiceIDSave;
      break;
    case 1:
      voiceOrder = voiceOrder1;
      voiceMax = 5;
      break;
    case 2:
      voiceOrder = voiceOrder2;
      voiceMax = 2;
      break;
    case 3:
      voiceOrder = voiceOrder3;
      voiceMax = 4;
      break;
    case 4:
      voiceOrder = voiceOrder4;
      voiceMax = 6;
      break;
    case 5:
      voiceOrder = voiceOrder5;
      voiceMax = 7;
      break;
    case 6:
      voiceOrder = voiceOrder6;
      voiceMax = 14;
      break;
  //RR vvvvvvvvvvvvvvvvvvvvvvvvv
     case 7:
      voiceOrder = voiceOrder7;
      voiceMax = 23;
      break;
//RR ^^^^^^^^^^^^^^^^^^^^^^^^
    default:
      break;
  }
*/  
  voiceDataNo = no;
}
#endif //useGo10on 終わり

/*
   Load voice list
*/
void setPickupVoiceList(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  int   i, v;
  while (file.available())
  {
    //Serial.write(file.read());
    v = file.read();
    Serial.print(v, DEC);
    v = v * 256 + (int)file.read();
    Serial.print(v, DEC);
    if (v > 0 && v <= 276) {
      voiceOrder[i++] = v;
    }
  }
  voiceMax = i; //数
  voiceID = 0;

  file.close();
}

void readVoiceData(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  char buf[64];
  int   v;
  int col = 0;
  int i = 0;
  int j = 0;
  voiceMax = 0;
  
  while (file.read() != 10);  //ヘッダー読み飛ばし
/*  
  while ((v = file.read()) != 10)  //ヘッダー情報読込
  {
   if (v == ',')
     {
       buf[j] = 0;
       if (col == 2)
          voiceFolder = String(buf);
        else if (col == 3)
          openingVoice = atoi(buf);
        col++;
        j = 0;
     }
     else if (v > 32)
     {
       buf[j++] = (char) v;
     }
  }

  col = 0;
  j = 0;
*/
  while (file.available())
  {
     v = file.read();

     if (v == ',')
     {
        buf[j] = 0;
       if (col == 0)
       {
          if (j > 0)
            voiceOrder0[voiceMax++] = atoi(buf);
       }
       else if (col == 1)
       {
          voiceData[i] = String(buf);
       }
        else if (col == 2)
       {
          voiceFile[i] = String(buf);
       }
       else if (col == 3)
       {
          ninshikiID[i] = atoi(buf);
       }
       else if (col == 4)
       {
         if (j > 0)
            characterID[atoi(buf)] = i;
       }
       j = 0;
        col++;
     }
     else if (v == 10)
     {
        if ((col == 5) && (j > 0))
        {
          buf[j] = 0;
          v = atoi(buf);
          
          if (v < 0)
          {
               v = v * -1;
               continueOrder[v/100-1] |= 1 << (v%100);
          }
          if (v < 100)
          {
            nextData[i] = v;
          }
          else
          {
            nextOrder[v/100-1][v%100] = i;
            maxOrder[v/100]++;
          }
        }
        col = 0;
        j = 0;
        i++;
     }
     else if (v > 32)
     {
       buf[j++] = (char) v;
     }
  }
  file.close();
  
  maxOrder[0] = voiceMax;
  v = voiceMax - 1;
  for (i = 59; i > 0; i--)
  {
    if (characterID[i] == 0)
      characterID[i]  = v;
    else
      v = characterID[i];
  }
}

void readCharacterData(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  char buf[64];
  int v;
  int i = 0;
  int j = 0;
  while (file.read() != 10);  //ヘッダー読み飛ばし  
  while (file.available())
  {
    v = file.read();
    if (v == 10)
    {
      buf[j] = 0;
      characterData[i++] = String(buf);
      characterXMax = j / 3;
      j=0;
    }
    else if (v >= ' ')
    {
      buf[j++] = (char) v;
    }
  }
  file.close(); 
}


#ifdef useWebServer

/**************************************
   serverStart
   Web Server 開始
***************************************/

void serverStart() {

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //makeRootPage();
  server.begin();
  delay(500);
  Serial.println("'HTTP Server started...");//M5.Lcd.println("'HTTP Server started...");
  Serial.println("");//M5.Lcd.println("");

  isServer = true;

}

void drawAPQRCode(int n) {
  //do not show url, now
  return;
  
  String apip = "";

  int xa = 10, xb = 170;
  if (n != 0) xa = xb = 90;

  //M5.Lcd.qrcode(const char *string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);

  if (n <= 0 && mySoftAPIP[0] >= 0x20 && mySoftAPIP[0] != '?') {
    apip = "http://" + IPAddressToStr(mySoftAPIP) + "/rvc2.htm";
#ifdef isM5Stack
    M5.Lcd.qrcode(apip, xa, 70, 140);
#endif
#ifdef hasOLED
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 30, apip);
    display.display();
#endif //hasOLED
  }
  if (n >= 0 && myStaAPIP[0] >= 0x20 && myStaAPIP[0] != '?') {
    apip = "http://" + IPAddressToStr(myStaAPIP) + "/rvc2.htm";
#ifdef isM5Stack
    M5.Lcd.qrcode(apip, xb, 70, 140);
#endif
#ifdef hasOLED
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 45, apip);
    display.display();
#endif //hasOLED
  }
}

#endif //useWebServer

/*

*/
/*
  #ifdef useAUDIO
  void playVoiceTask(void *args) {
  //Play voice
  for(;;) {
    if(voicePlay) {
      aout->SetGain(spkvol);
      while(agen->isRunning()){
        if (!agen->loop()) {
          Serial.println("Stop");
          agen->stop();
          voicePlay=false;
          aout->SetGain(0);
          aout->stop();
          delete afile;
        }
      }
      selmode=1;
      drawListVoiceID();
      drawSelMode();
    }
    delay(1);
  }
  }
  #endif
*/
/*
  #ifdef useWebServer
  void webServerTask(void *args) {
  for(;;) {
    // Check if a client has connected
    server.handleClient();
    delay(2000);
  }
  }
  #endif
*/

#ifdef useBluetooth
void advertisingTask(void *args) {
  for (;;) {
    //Which information do you send Q-bo ID or File ID?
    unsigned long nowrap = millis();
    if (lastrap <= nowrap) {
      dataType += 1;
      if (dataType > 1) {
        dataType = 0;
      }
      if (counter == 0) dataType = 0;
      dataType = (dataType++ % 2);
      lastrap = nowrap + rapx[dataType];
    }

    //If the counter is less than zero, send Q-bo ID forcely.
    if (dataType > 0 && counter > 0 && waitVoice == false)
    {
      //set advertise for File ID
      setFileIdManData();
    } else {
      //Set advertise for Qbo ID
      setQboIdManData();
    }
    delay(50);
  }
}
#endif //useBluetooth

#ifdef useFlashAir
void sendRemoteLogTask(void *args) {
  if(enableFlashAir) {
    for (;;) {
      if(tgtid>0 && waitVoice == false) {
        Serial.print("ID:#");Serial.println(tgtid);
        sendRemoteLog(tgtid);
        tgtid=0;
      }
      delay(50);
    }
  }
}
#endif //useFlashAir

void setMultiTask() {

#ifdef useBluetooth
  //xTaskCreatePinnedToCore
  xTaskCreate(
    advertisingTask,     // Function to implement the task
    "advertisingTask",   // Name of the task
    4096,      // Stack size in words
    NULL,      // Task input parameter
    1,         // Priority of the task
    NULL);//,      // Task handle.
  //1);        // Core where the task should run

#endif //useBluetooth

#ifdef useFlashAir
if(enableFlashAir) {
  //xTaskCreatePinnedToCore
  xTaskCreate(
    sendRemoteLogTask,     // Function to implement the task
    "sendRemoteLogTask",   // Name of the task
    4096,      // Stack size in words
    NULL,      // Task input parameter
    1,         // Priority of the task
    NULL);//,      // Task handle.
  //1);        // Core where the task should run
}
#endif //useFlashAir

  /*
    //xTaskCreatePinnedToCore
    xTaskCreate(
                playVoiceTask,     // Function to implement the task
                "playVoiceTask",   // Name of the task
                4096,      // Stack size in words
                NULL,      // Task input parameter
                1,         // Priority of the task
                NULL);//,      // Task handle.
                //1);        // Core where the task should run
  */

  /*
    //xTaskCreatePinnedToCore
    xTaskCreate(
                webServerTask,     // Function to implement the task
                "webServerTask",   // Name of the task
                4096,      // Stack size in words
                NULL,      // Task input parameter
                1,         // Priority of the task
                NULL);//,      // Task handle.
                //1);        // Core where the task should run
  */

}

/*****************************************************
   setup
 *****************************************************/

void setup()
{
#ifdef isM5Stack
  M5.begin();
  //Wire.begin();
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
#endif

  Serial.begin(115200);
  M5.Lcd.setTextSize(2);

  //Start SPIFFS or microSD
#ifdef useSPIFFS
  if (!qbFS.begin()) Serial.println("File IO failed...");
#endif //useSPIFFS

#ifdef hasSD
  if (!SD.begin()) Serial.println("File IO failed...");
#endif //hasSD

#ifdef useAUDIO
  #ifdef hasSD
    enableAudio = SD.exists("/voice/Ninshiki/NF26.wav");
  #endif
  if(M5.BtnC.isPressed()) enableAudio=false;
#endif

#ifdef useWebServer
  //Load Soft AP
  beginWiFi(SOFT_AP_NUM);//loadSoftAP(SD,"/sftap.txt");// APモード

#endif //useWebServer

#ifdef useFlashAir
  if(M5.BtnB.isPressed()) {
    Serial.println("Disable FlashAir...");
  } else {
    // We start by connecting to a WiFi network
    enableFlashAir=beginFlashAir();
    // FlashAir set upload directry
    changeRemoteLogDir();
  }
#endif //useFlashAir

  M5.Lcd.println("");
  if(enableAudio) M5.Lcd.println("Audio: Enabled");
  else M5.Lcd.println("Audio: Disabled");
  if(enableFlashAir) M5.Lcd.println("FlashAir: Enabled");
  else M5.Lcd.println("FlashAir: Disabled");
  
#ifndef useWiFi //Not useWiFi
  //WiFiを使わない設定で、
  //WiFi繋がっていても切らないことにする(19/2/24)
  //WiFi.disconnect(true);//WiFi off
#endif //not useWiFi

  //For Faces
#ifdef useFACES
  Wire.begin();
  pinMode(KEYBOARD_INT, INPUT_PULLUP);
  Serial.println("Faces: Enabled");
  M5.Lcd.println("Faces: Enabled");
#endif

#ifdef useBluetooth
  ble.begin("Q-boM5");  //sets the device name
  ble.setIncludeName(false);
  ble.setIncludeTxpower(false);

  lastrap = millis() + rapx[dataType];

  Serial.println("BLE: Enabled");
  M5.Lcd.println("BLE: Enabled");
#endif //useBluetooth

  delay(1500);
  
/***************************************************/
#ifdef isM5Stack
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(TFT_BLACK);

#ifdef useJPFONT
  SDfonts.init(SD_PN);
  Serial.println(F("sdfonts liblary"));

  //Robi2 BLE Voice Controller
  fontDump(20, 5, "RoVoCoMo2", 24, TFT_CYAN);
  fontDump(20, 30, "BLE & FlashAir", 16);
  fontDump(20, 47, "by Micono v1.7", 12, TFT_GREEN);
#endif //useJPFONT

#endif //isM5Stack

#ifdef hasOLED
  // Initialising the UI will init the display too.
  ui.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "RoVoCoMo2");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 16, "Voice Controller");
  display.display();
#endif //hasOLED
/***************************************************/

#ifdef useWebServer
  //Connect to station access poiint
    beginWiFi(STATION_AP_NUM);//if(loadStaAP(SD,"/staap.txt")==false) loadPreferStaAP();

  if(isServer) serverStart();// HTTP server start
  delay(500);

#endif //useWebServer

#ifdef useGo10on //50音検索

    #ifdef useSPIFFS
      readVoiceData(qbFSD,"/Ninshiki.csv");
      readCharacterData(qbFSD,"/Character.csv");
   #else
      readVoiceData(SD,"/Ninshiki.csv");
      readCharacterData(SD,"/Character.csv");
    #endif   
//  Serial.print("voiceFolder=");Serial.print(voiceFolder);Serial.print("openingVoice=");Serial.println(openingVoice);
  changeVoiceData(0);

#else //not useGo10on
  //Default voice order
  for(int i=0;i<300;i++) voiceOrder[i]=i;
  
  //Pick up voice
  
  #ifdef useSPIFFS
    //setPickupVoiceList(qbFSD,"/PickupVoice.bin");
  #else
    setPickupVoiceList(SD,"/PickupVoice.bin");
  #endif

#endif //useGo10on 終わり

  //delay(1000);

#ifdef useAUDIO  
  if (enableAudio) {
    aout = new AudioOutputI2S(0, 1); // Outaput to builtInDAC
    aout->SetGain(0);//aout->SetGain(spkvol/100.0);
    aout->SetOutputModeMono(true);
  
  #ifdef useMP3
    agen = new AudioGeneratorMP3();
    playVoice("/GMVRC/D231300525448/DS041.MP3");//("/Message/Start_sound.mp3");
  #else
    agen = new AudioGeneratorWAV();
    playQboSoundNum(openingVoice);
  #endif //useMP3 or WAV
  }
#endif //useAUDIO

  //Draw Voice List
  drawListVoiceID();
  drawSelMode();

  setMultiTask();

#ifdef useWebServer
  drawAPQRCode(0);
  #ifdef isESP32
    showQRC = false;
    selmode = 1;
  #endif

#else //not useWebServer
  showQRC = false;
  selmode = 1;

#endif

}

/*****************************************************
   Loop
 *****************************************************/

void loop()
{
  // put your main code here, to run repeatedly:
#ifdef isM5Stack
  M5.update();
#endif

#ifdef useAUDIO
  if (enableAudio) {
    //Play voice
    if (voicePlay) {
      aout->SetGain(spkvol);
      while (agen->isRunning()) {
        if (!agen->loop()) {
          Serial.println("Stop");
          agen->stop();
          voicePlay = false;
          waitVoice = voicePlay;
          aout->SetGain(0);
          aout->stop();
          delete afile;
        }
      }
      selmode = 1;
      drawListVoiceID();
      drawSelMode();
      return;
    }
  }
#endif //useAUDIO

#ifdef useFACES
  if (digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) {
      doFacesKey(Wire.read()); // receive a byte as character
    }
  }
#endif //useFaces

#ifdef isM5Stack

  //Power OFF
  if (M5.BtnA.isPressed() && M5.BtnC.isPressed()) {
//RR VVVVVVVVVVVVVVVVVVVVVVV
#ifdef useGo10on //50音検索
    if (voiceDataNo != 0)
      doBtnAC();
    else
#endif
//RR ^^^^^^^^^^^^^^^^^^^^^^^ 
      M5.powerOFF();
 }
  //A button
  if (M5.BtnA.wasPressed() || M5.BtnA.isPressed()) {
    pressJump++;
    doBtnAB(-1);
    return;
  }

  //C button //<--B button
  if (M5.BtnC.wasPressed() || M5.BtnC.isPressed()) {
    pressJump++;
    doBtnAB(1);
    return;
  }

  //B button //<--C button
  if (M5.BtnB.wasPressed()) {
    doBtnC();
    return;
  }

  if (M5.BtnA.wasReleased() || M5.BtnB.wasReleased() || M5.BtnC.wasReleased()) pressJump = 0;

#endif //isM5Stack

#ifdef useWebServer
  // Check if a client has connected
  server.handleClient();
#endif //useWebServer

  delay(10);
  
}

/****************************************************************/
/*
   Button actions

*/

void checkVoiceID() {
  if (voiceID < 0) {
    voiceID = voiceMax - 1;
  } else if (voiceID >= voiceMax) {
    voiceID = 0;
  }
}

int VoiceID(int n) {
  if (n < 0) {
    n = voiceMax - 1;
  } else if (n >= voiceMax) {
    n = 0;
  }
  return n;
}


//RR vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#ifdef useGo10on //50音検索
void doBtnAC()
{
  changeVoiceData(0);
  selmode = 1;
  drawListVoiceID();
  drawSelMode();
}
#endif
//RR ^^^^^^^^^^^^^^^^^^^^^^^^^^^
void doBtnAB(int w) { //A=-1, C=1
#ifdef useGo10on //50音検索
  if (w == 1) { //Cボタン
//RR vvvvvvvvvvvvvvvvvvvvvvvv    
    if (voiceDataNo == 0)
    {
// RR ^^^^^^^^^^^^^^^^^^^^^^^    
//長押し if (pressJump<2) {
      if (selmode == 2) { //50音表示-->右へ
        drawCharacterPos(0);
        characterXPos++;
 //       if (characterXPos > 11)
       if (characterXPos > characterXMax - 1)
         characterXPos = 0;
        drawCharacterPos(1);
        delay(100);   //長押し
      }
      else { //50音表示に変更
        drawCharacterTable();
        drawCharacterPos(1);
        selmode = 2;
        drawSelMode();  
      }
//長押し  }
    return; //戻る
     }   //RR
   }

  //Aボタンかつ50音表示の場合
  //長押し if (pressJump < 2) {
  if (selmode == 2) { //50音表示-->下へ
    drawCharacterPos(0);
    characterYPos++;
    if (characterYPos > 4)
      characterYPos = 0;
    drawCharacterPos(1);
    delay(100);
    return; //戻る
  }
#endif //useGo10on 終わり

  //50音表示でなくて、Aボタンの場合
  
  switch (selmode) {
  case -1://QRcode
  #ifdef isM5Stack
    M5.Lcd.fillRect(0, posY - 60, 320, 150, 0);
  #endif
  #ifdef useWebServer
    drawAPQRCode(w);
  #endif //useWebServer
    showQRC = false;//true; //Always false, now
    selmode = -1;
    return;
    
  case 0:
  #ifdef useGo10on //50音検索
    selmode = 1;
  #else //useGo10on //50音検索
    selmode = 5;
  #endif //useGo10on 終わり
    drawListVoiceID();
    drawSelMode();
    return;
    
  case 1:
  case 5:
  //長押し#ifndef useGo10on //50音検索
    voiceID += selmode * -w;  
 /*
    if (pressJump < 5) {
      voiceID += selmode * -w;
    } else {
      voiceID += (pressJump / 5) * 5 * selmode * -w;
    }
*/
  //長押し#else //useGo10on //50音検索
  //長押し  if (pressJump > 1)
  //長押し    return;
  //長押し  voiceID++;
  //長押し#endif //useGo10on 終わり
    break;
    
  default:
    return;
  }
  checkVoiceID();
  drawListVoiceID();
}

void doBtnC() { // is B button
  switch (selmode) {
    case -1://QRcode
#ifdef isM5Stack //useJPFONT
      M5.Lcd.fillRect(0, posY - 60, 320, 150, 0);
#endif
      showQRC = false;
      selmode = 1;
      drawListVoiceID();
      drawSelMode();
      return;
      
    case 0:
      //Play
#ifdef isM5Stack //useJPFONT
      fontDump(posXb, posY, (char*)voiceData[voiceOrder[voiceID]].c_str(), 24, TFT_RED);
      fontDump(smpX + smpS * 2, smpY, "再生", fntsz, TFT_RED);
#endif
      selmode = -1;
      break;
      
    case 1:
    case 5:
#ifndef useGo10on //50音検索でない場合
      selmode = 0;
      drawSelMode();
#ifdef isM5Stack //useJPFONT
      fontDump(posXb, posY, (char*)voiceData[voiceOrder[voiceID]].c_str(), 24, TFT_GREEN);
#endif //isM5Stack
      return;
      
#else //useGo10on //50音検索の場合
      selmode = -1;
      break;
      
    case 2: //50音表示の時に選択ボタンを押した場合
      if (voiceDataNo > 0)
        changeVoiceData(0);
      voiceID = characterID[characterXPos * 5 + characterYPos];
      Serial.print("voiceID="); Serial.println(voiceID);
      selmode = 1;
      M5.Lcd.fillRect(0, posY - 60, posXb, 150, 0);
      drawListVoiceID();
      drawSelMode();
      return;
      
#endif //useGo10on 終わり

    default:
      return;
  }

#ifdef useAUDIO
  if (enableAudio) waitVoice = true;
#endif //useAUDIO
  int id = voiceOrder[voiceID];
  tgtid = ninshikiID[id];
  int idz = tgtid - 1;
/*
 int idz= voiceOrder[voiceID];//zero base
 tgtid = idz + 1;
*/
  counter++;
  target = fileid + idz;
  Serial.print("FileID="); Serial.println(target);

#ifdef useGo10on //50音検索
  fontDump(posXb, posY - 60, (char*)voiceData[voiceOrder[voiceID]].c_str(), 24, TFT_RED);
//  fontDump(posXb, posY - 60, (char*)voiceData[idz].c_str(), 24, TFT_RED);  
//RR vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    if (voiceDataNo == 0)
      changeVoiceData(nextData[voiceID]);
    else if ((continueOrder[voiceDataNo - 1] & (1 << voiceID)) == 0)
      changeVoiceData(0);
/*
  if ((voiceDataNo != 7) || (voiceOrder[voiceID] == 237)) {
    if (voiceDataNo == 0)
      changeVoiceData(nextData[voiceID]);
    else
      changeVoiceData(0);
  }
*/
//RR ^^^^^^^^^^^^^^^^^^^^^^^
#endif //useGo10on 終わり

#ifdef useAUDIO //使用

Serial.print("enableAudio="); Serial.println(enableAudio);

  if (enableAudio) {
#ifdef useMP3
    playQboSoundNum(tgtid);
#else
    playQboSoundNum(id);
#endif
    delay(10);
  }
  else {
    delay(500);
    selmode = 1;
    drawListVoiceID();
    drawSelMode();
  }
  
#else //Audio 未使用
  delay(500);
  selmode = 1;
  drawListVoiceID();
  drawSelMode();
  
#endif //useAUDIO 終わり

  //#ifdef useFACES
  startkey = false;
  //#endif

  return;
}

//#ifdef useFACES

void doFacesKey(int key_val) {
  Serial.println(key_val);
  if (key_val != 0) {
    if (key_val >= 0x30 && key_val <= 0x39) {
      if (startkey) {
        voiceID = (voiceID + 1) * 10 + key_val - 0x30 - 1;
      } else {
        startkey = true;
        voiceID = key_val - 0x30 - 1;
      }
      checkVoiceID();
      drawListVoiceID();
      return;
    } else {
      doTenKey(key_val);
      return;
    }
  }
}

void doTenKey(int key_val) {
  switch (key_val) {
    case '=':
    case 191://Select
    case 239://A
    case 223://B
      doBtnC();
      if (selmode == 0) doBtnC();
      return;

    case '+':
    case 253://Down
      selmode = 1;
      doBtnAB(1);
      drawSelMode();
      return;

    case '-':
    case 254://Up
      selmode = 1;
      doBtnAB(-1);
      drawSelMode();
      return;

    case '*':
    case 247://Right
      selmode = 5;
      doBtnAB(1);
      drawSelMode();
      return;

    case '/':
    case 251://Left
      selmode = 5;
      doBtnAB(-1);
      drawSelMode();
      return;

    case 127://Start
      voiceID = 0;
      drawListVoiceID();
      startkey = false;
      return;

    default:
      break;
  }

  startkey = false;
  return;
}

//#endif //useFACES

#ifdef useWebServer

/***************************************
 * IP to String
 * IPを文字に
 * 
***************************************/

String IPAddressToStr(IPAddress ip) {
  char buf[13];
  sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return String(buf);
}

/***************************************
   getContentType
   リンクされてるコンテンツのタイプを取得
*/

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".ttf")) return "application/x-font-ttf";
  return "text/plain";
}

/***************************************
   handleFileRead
   リクエストされたファイルをSPIから読込み処理
*/

bool handleFileRead(String path) {
  //**Serial.println(path);
  //DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (qbFS.exists(path)) {
    File target = qbFS.open(path);
    if (!target) {
      Serial.println("File open error...");
      return false;
    }
    if (target.isDirectory()) path += "index.html"; //if(path.endsWith("/")) path += "index.html";
    target.close();
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  //**Serial.println(path);
  if (qbFS.exists(pathWithGz) || qbFS.exists(path)) {
    //**Serial2.println("exist!");
    //**if(mjFS.exists(pathWithGz)) path += ".gz";
    File file = qbFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  } else {
    Serial.println(path);
    if (path.indexOf("/vckey/") == 0) {
      int key_val = path.substring(7).toInt();
      startkey = true;
      voiceID = VoiceID(key_val - 1);
      doBtnC();
      if (selmode == 0) doBtnC();

    } else if (path.indexOf("/nmkey/") == 0) {
      int key_val = path.substring(7).toInt();
      doFacesKey(key_val);

    } else if (path.indexOf("/ijkey/") == 0) {
      int key_val = path.substring(7).toInt();
      switch (key_val) {
        case 10://Return
          doBtnC();
          if (selmode == 0) doBtnC();
          break;

        case 27://ESC
          startkey = false;
          break;

        case 28://Left
          doTenKey(251);
          break;
        case 29://Right
          doTenKey(247);
          break;
        case 30:
          doTenKey(254);
          break;
        case 31:
          doTenKey(253);
          break;

        default:
          //doFacesKey(key_val);
          //voiceID=VoiceID(key_val-1);
          break;

      }
    }
  }
  return false;
}

#endif //useWebServer

/***************************************/
