//========================================================================================FIRMWARE INFORMATION===================
/*
openHW project (?) Denis
https://github.com/DenisDx/openHW
/*----------------------------------------------------------------------------------
#HOW TO BUILD AND FLASH
1. Intall Ardino board framework
  https://www.arduino.cc/en/Main/Software
2. Install additional libraries for you board:
  Go to File > Preferences : Enter library json URL into the “Additional Board Manager URLs” and click "ok"
  * arduino (or compatible board): do not need to intall additional libraries 
  * esp8266: http://arduino.esp8266.com/stable/package_esp8266com_index.json 
  * esp32 (Heltek web kit) : https://dl.espressif.com/dl/package_esp32_index.json  
  * etc (see your board manual)
  if you wish to work with different boadrs, you can set mutily urls like 
    https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
3. install micro-ecc library: go to Sketch=>Include Library=>Manage Libraries; find uECC library and install it
4. Perform additional steps (you should restart arduino application after the changes): 
   ---For AVR (Arduino UNO, etc):---
   A.1. change #define uECC_SUPPORT_COMPRESSED_POINT 1 to #define uECC_SUPPORT_COMPRESSED_POINT 0 in uECC.h
   A.2. make modification to the board's settings for protect the memory and increase buffer size:
      1) locate your arduino directory (in windows, it is C:\Program Files\arduino...)              
      2) copy the core directory <base Arduino folder>\hardware\arduino\avr\cores\arduino to a different name, for example 
         <base Arduino folder>\hardware\arduino\avr\cores\arduinohw
      3) edit the hardware serial file in the new folder. It could be called HardwareSerial.h or USBAPI.h depends on version
         you must add two lines on the beginning:
           #define SERIAL_TX_BUFFER_SIZE 129
           #define SERIAL_RX_BUFFER_SIZE 150
      4) open <base Arduino folder>\hardware\arduino\cores\arduino\avr\boards.txt
         find your board (for example, Arduino UNO) (the section will be started from "uno.name=Arduino Uno")
         copy the whole section and rename all preffixes to the new one for example, "uno" -> "unohw"
         change in the new section:
         1: .name parameter (for example, "uno.name=Arduino Uno" will be "unohw.name=Arduino HW"
         2: .build.core change to the folder you created before (for example, "uno.build.core=arduino" will be "unohw.build.core=arduinohw")
5. Now let's make changes in the main file
   5.1. Set your board in FIRMWARE SETTINGS section in the openHW.ino file. 
        (for example, if you board is Heltec ESP32 you should delete "//" in the line "//#define board_Heltec")
        If your board is not listed, you should configure it below in the boards ection or do nothing (screen and buttons will not work in this case)
   5.2. Make sure that DEBUG option is turned off if you are not a developer.     
        This is in the second section right after the board selection.
        if you see the row "#define DEBUG 1" - change it to "//#define DEBUG 1"
        otherwise it will be possible to see your private keys just using command getConfig()
   
6. Connect your board. Select COM port and your board type in Tools menu. If you created a new platfor (recommended) you have to choose it.

7. Press arrow (flash) button to flash the board.

8. Check if firmware works

9. Protect EEPROM memory data from firmware changes (and do other protection thinks)
 ---For AVR (Arduino UNO, etc):---
    A: You will need an additional device to make less or more good protection. It costs less than usd$1
    first, you have to set EESAVE = 1 (off, there 1 means off) to prevent EEP reading using a changed firmware
    second, it would be a good to change DWEN, SPIEN and RSTDSBL (or only DWEN and RSTDSBL) bits.
       DWEN = 1(off); SPIEN=1(off);  RSTDSBL = 0 (on). The last two options will make firmware updating impossible

    For most of the available Arduino boards this point can be omitted (the system security will be reduced, be sure to use the Password, 
    if you lose the key - remove the paring on the host with it), because by default the memory cleaning when flashing is enabled  and debugging is disabled.
    You can easily check this by flashing the device, setting a private key and flashing it again. 
    If the private key is disappeared and the key returns to the uninitialized state - that's okay, changing EESAVE bit is not required.

    if you not pass this step you private key can be read from the chip easily
    BE VERY CAREFUL HERE, YOU CAN BRICK YOUR BOARD
    universal calculator: http://www.engbedded.com/fusecalc/

    A partial solution may be to buy a board with the EESAVE flag sets to 1 (off). It is a default value for many boards.
    In this case if intruders wants to read your key he must at least disassembly device and use additional equipment.
    DWEN is usually turned off (1) by default.

# Usage example protocol
">" means we send the text to the device (do not type ">" symbol itself
"<" means the device's answer (do not type "<" symbol itself
## Initial setup:
> 

----------------------------------------------------------------------------
CREDS
 * https://github.com/kamaljohnson/Arduino-SHA256/
*/
//========================================================================================FIRMWARE SETTINGS=====================
//Board: (if nothing changed then default settings will be applied. You can see details in the boards section)
//#define board_Heltec_WiFi_Kit_32
        //use WiFi_Kit_32 board for https://dl.espressif.com/dl/package_esp32_index.json  library

//#define board_Heltec_WiFi_Kit_8  
         //= HTIT-W8266 //https://heltec.org/project/wifi-kit-8/  //library: In the Arduino IDE, in the Tools > Board menu choose  choose board: NodeMCU 1.0 (ESP-12E Module)
         //alternative from Heltec:   https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.1/package_heltec_esp8266_index.json  ; board name Wifi_Kit_8

//#define board_WEMOS_LoLin32_Oled //use WEMOS_LoLin32 board. screen connected 
   //= HW-630
   //? = Wemos® Nodemcu Wifi
   //? = wemos D-duino


//---------------------------------------linkage settings--------------------
//Use SHA256 (needs 450+ bytes RAM)
#define SHA256_SUPPORTED 1
//Debug mode:
//#define DEBUG 1

//---------------------------------------defaults ---------------------------
#define DROP_IF_WRONG_PIN true
#define SERIAL_TIMEOUT 500 //milliseconds
#define PIN_TIMEOUT 100000 //100 sec
#define BUTTON_TIMEOUT_MS 10000 //10 sec
//Maximum PIN length
#define PIN_MAX_SIZE 8
#define PASSWORD_LEN 16

//THIS IS THE END OF THE SETTINGS.
#define uECC_OPTIMIZATION_LEVEL 3
// #pragma GCC optimize ("-O0")
#ifdef __AVR__
  #pragma GCC optimize ("-Og")  //we must do it for memory saving
#endif
// #pragma GCC diagnostic warning "-fpermissive"
// #pragma message ("DO NOT FORGET TO REVIEW FIRMWARE SETTINGS")

//==============================================================================================================================
#define VERSION F("0.5")

//============================================================== Includes  =====================
#include <types.h>
#include <uECC.h>
#include <uECC_vli.h>
#include <EEPROM.h>

//============================================================== boards =========================
//Heltec: board_Heltec
//U8g2 for screeen
//Esp32 for the board
//pro_micro / uno ... define nothing




//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//#ifdef board_Heltec_WiFi_Kit_32
//#define OPTION_screen_OLED U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//#elif defined(board_Heltec_WiFi_Kit_8)
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 5, /* reset=*/ 4); 
//#elif defined(board_WEMOS_LoLin32_Oled)
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//#endif



#ifdef board_Heltec_WiFi_Kit_32
  #define OPTION_screen_OLED U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
  #define OPTION_screen_placement 1 //128X64 default
  #define OPTION_button 0 //pinMode(OPTION_button,INPUT_PULLUP);
#endif
#ifdef board_Heltec_WiFi_Kit_8
  #define OPTION_screen_OLED U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 5, /* reset=*/ 4);
  #define OPTION_screen_placement 2 //128X32 default
#endif
#ifdef board_WEMOS_LoLin32_Oled
  #define OPTION_screen_OLED U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ 15);
  #define OPTION_screen_placement 1 //128X64 default
  #define OPTION_button 0 //pinMode(OPTION_button,INPUT_PULLUP);
#endif  


//============================================================== helper section  ================
//unsigned char tBuf[80]; //universal buf. DO NOT use it if you are going to call any functions while using it
uint32_t crc32b(const unsigned char *message, size_t len) {
   uint32_t b, crc, mask;

   crc = 0xFFFFFFFF;
   //memset(&crc,0xFF,sizeof(crc));
   
   for (int i=0; i<len; i++){
      b = message[i];               // Get next byte.
      crc = crc ^ b;
      for (int j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
};

bool memnchr(const byte *ptr, byte symbol, size_t len) { //returns if there is any OTHER symbol
  for (size_t i=0;i<len;i++) if (*(ptr+i)!=symbol) return true;
  return false;
};

void hex8Serial(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
  //Serial.print("0x"); 
  for (int i=0; i<length; i++) { 
    if (data[i]<0x10) Serial.print("0");
    Serial.print(data[i],HEX); 
  }
}


//============================================================== Types  ========================


struct Settings
//typedef struct __attribute__((packed)) Settings
//will be stored in EEPROM
{
  uint8_t privKey0[32];
  uint8_t privKey1[32]; //Private key for plausible denial
  
  char pin0[9]; //main pin
  char pin1[9]; //pin for plausible denial

  byte attCounter = 0; //attempt counter. Zero it if known pin
  byte mode1 = 0; //if true, we have entered correct PIN1. So, we are working with privkey1 until the pin0 will be entered. it is possible only if no pins entered for one week
  bool passwordProtected = 0; //if the key is encrypted by password. If state.password not entered then show error in case the privkey queried

  //user settings
  char uname[24] = "";
  bool askPinEveryTime = false; //if we need ask pin for every privKey0 access
  bool lockPinAfterMs = 0; //if we need to forget pin 
 
  byte buttonMode = 1; //0: don't use button (only for setPrivateKey) 1: only for pin unlock 2: for each private key access

  //misc
  byte WIF = 0xEF;
  byte Sig = 33;  

  //crc
  uint32_t crc = 0;
};

struct State
//current "session"
{
  unsigned long lastPinEnter = 0; //millis of the last pin enter (no matter if it was correct or not
  char pin[PIN_MAX_SIZE+1]; //pin entered
  unsigned long lastMode1Millis = 0; //last millis attempt for mode 1  
  byte password[PASSWORD_LEN]; //password for keys protection. 16 bytes
  byte passwordLen = 0; //password for keys protection. 16 bytes
  //String commandWaiting; //the command waiting now. "" if there is no command. Will be executed after any event like pin enter or button pressed
  byte buttonMode = 0; //will be loaded from settings
  byte passwordProtected = 0;
};


size_t bufFromHex(const char buf[], size_t len, uint8_t *res) {
  char sym[3]={0,0,0};
  for (int i=0; i<len; i+=2) {
    memcpy(sym,buf+i,2);
    res[i/2] = strtoul(sym,NULL,HEX);
  }
  return len/2;
};

size_t param2buf(const char buf[], size_t len, uECC_word_t *res){
  //detect param and decodes. hex : 0x<data>  code58:default  code64 =data
  //if (str.startsWith("0X")) return bufFromHex(str.substring(2)); //{
    //String tStr = str->substring(2);
    //return bufFromHex(tStr);
  //}
  
};

//======================================================================================================================================================================
//memtools
#ifdef DEBUG
/*
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__


int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
*/

#include "memdebug.h"
#include "memdebug.c"
#endif // DEBUG
void printMemory(bool newline=true){
  #ifdef DEBUG
  //Serial.print(F("Memory: free:"));Serial.println(freeMemory()); 
  //Serial.print(F(" __malloc_heap_start:"));Serial.println((int)__malloc_heap_start); 
  //Serial.print(F(" __brkval:"));Serial.println((int)__brkval);   
    #ifdef __AVR__
    Serial.print(F("#Memory: free:"));Serial.print(getFreeMemory()); 
    
    if (newline) Serial.println();
    #endif // __AVR__
  #endif // DEBUG
};

bool checkMemory(int minVolume) {
  #if defined(__AVR__) && defined(DEBUG)
  if (minVolume>getFreeMemory()) {
    Serial.print(F("ERROR: not enough memory. Needs "));Serial.print(minVolume);Serial.print(F(" but available only "));Serial.println(getFreeMemory()); 
    return 0;
  } else return 1;
  #else
    return 1;
  #endif // __AVR__
};

//======================================================================================================================================================================
//PROGMEM AREA
#define PGMSTR(x) (__FlashStringHelper*)(x) 
const PROGMEM char TEST_KEY_1[] = {"cNpjQkzqzoEvgg1GhHQ7JY2cNjh97rhqS1wDH3m7EDnB1T5hrB5D"};

#if defined(ESP8266)
 size_t getProgmemStr(const char * strname, char result[]) {
    for (byte k = 0; k <strlen_P(strname) ; k++) result[k] = pgm_read_byte_near(strname + k);
    return strlen_P(strname);
  };
#else
  size_t getProgmemStr(const PROGMEM char * strname, char result[]) {
    for (byte k = 0; k <strlen_P(strname) ; k++) result[k] = pgm_read_byte_near(strname + k);
    return strlen_P(strname);
  };
#endif

#ifdef DEBUG
void debugPrint(const __FlashStringHelper * str, bool newline=true, bool printMem=false) {
  Serial.print(F("#"));Serial.print(str);Serial.print(F(" "));
  if (printMem) printMemory(false); 
  if (newline) Serial.println();
};
#else
  #define debugPrint(x, ...)
#endif
//===================================================================================================================================================
//---------------------------decode options-----------------------------------------------------

#ifndef OPTION_button //pinMode(OPTION_button,INPUT_PULLUP);
  #define askForButton(x) 1
#else
  byte askForButton(const bool showHelp = true);
#endif  

#ifdef OPTION_screen_OLED
  #define OPTION_screen 1
#endif  

#ifndef OPTION_screen 
  //empty functions
  void screenInit() {};
  //void screenShow(const char *str) {};
  unsigned char screenStatus[24] = "";

  //kill setScreenStatus if no screen
  #define setScreenStatus(x)
  #define screenShow(x)
#endif

//===================================================================================================================================================

#if defined(__PIC32MX__)
char* F(char input[]) {
  return input;
}
#endif

extern "C" {

int RNG(uint8_t *dest, unsigned size) {
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of 
  // random noise). This can take a long time to generate random data if the result of analogRead(0) 
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = analogRead(0);
      int count = 0;
      while (analogRead(0) == init) {
        ++count;
      }
      
      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

}  // extern "C"



//const struct uECC_Curve_t * curve = uECC_secp256r1();
const struct uECC_Curve_t * curve = uECC_secp256k1();
//const struct uECC_Curve_t * curve = uECC_secp160r1();

State state;
//Settings *ptrSettings = 0;


//input buf data
#define ibufLen 150
char ibuf[ibufLen]; //serial command buf

long int sCmdLastRead = millis(); //last buf reading time
bool  sCmdBFound = 0; //Bracket found
long int pinAwaitingFrom = 0;//from this time we are waiting for a pin. Will be dropped after timeout togeter with state.commandWaiting .
size_t pinPos=0; //position pin started from
size_t cmdLen=0; //length of the buffer used (=next symbol pos)

//ibuf: cmdLen - next symb; pinPos - position pin started from; pinAwaitingFrom -millis; sCmdBFound sCmdLastRead 



void onUpdateSettings(Settings *settings);
void readSettings(Settings *settings); //prototyping magic

void readSettings(Settings *settings)
{
  //uint8_t ta[sizeof(Settings)];
  //for (int i=0; i<sizeof(Settings); i++) ta[i] = EEPROM.read(i);

  
  //Serial.print(F("ta="));for (int i=0; i<sizeof(Settings); i++) {Serial.print(ta[i],HEX); Serial.print(F(" "));}; Serial.println();
  //Serial.println(F("Settings:"));
  
  //Serial.print(F("settings="));
  
  //int x = (uint32_t)ta;//(int)settings->crc;  
  //Serial.println(x); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  
  //Serial.print("AFTER"); printMemory();
  //Serial.print("ta=");Serial.println((int)ta);  
  //Serial.print("&ta[0]=");Serial.println((int)&(ta[0]));  
  
  //Serial.print("&(settings->crc)=");Serial.println((int)&(settings->crc));  
  
  //Serial.println();
  //Serial.print("sizeof(Settings)=");Serial.println(sizeof(Settings));
  //settings->crc = 123; 
  //return;
  #if defined(ESP8266) || defined(ESP31B) || defined(ESP32)
    EEPROM.begin(sizeof(Settings));
    for (int i=0; i<sizeof(Settings); i++) ((byte*)settings)[i] = EEPROM.read(i);
    EEPROM.end();
  #else  
    for (int i=0; i<sizeof(Settings); i++) ((byte*)settings)[i] = EEPROM.read(i);
  #endif  
  //settings loaded
  if (crc32b(((byte*)settings),sizeof(Settings) - sizeof(settings->crc))!=settings->crc) {
     //settings are empty or invalid
     //set defaults  
      memset(settings,0,sizeof(*settings));

      settings->buttonMode = 1; //0: don't use button 1: only for pin unlock 2: for each private key access
      settings->WIF = 0xEF;
      settings->Sig = 33;  
      
      debugPrint(F("Settings empty, set defaults."),1,1);
  } else debugPrint(F("Settings read."),1,1);

  //apply current settings:
  state.buttonMode = settings->buttonMode;
  state.passwordProtected = settings->passwordProtected;
  onUpdateSettings(settings);
};

void writeSettings(Settings *settings);

void writeSettings(Settings *settings)
{
  settings->crc = crc32b(((byte*)settings),sizeof(Settings) - sizeof(settings->crc));

  #if defined(ESP8266) || defined(ESP31B) || defined(ESP32)
    EEPROM.begin(sizeof(Settings));
    for (int i=0; i<sizeof(Settings); i++) 
      if (EEPROM.read(i)!=((byte*)settings)[i]) 
        EEPROM.write(i,((byte*)settings)[i]);
    //EEPROM.commit();
    EEPROM.end();
        
    debugPrint(F("Settings recorded to NVS."),1,1);
  #else
    for (int i=0; i<sizeof(Settings); i++) 
      if (EEPROM.read(i)!=((byte*)settings)[i]) 
        EEPROM.write(i,((byte*)settings)[i]);
    
    debugPrint(F("Settings recorded to EEPROM."),1,1);
  #endif
  /*
  bool changed = false;
  
  settings->crc = crc32b(((byte*)settings),sizeof(Settings) - sizeof(settings->crc));
  for (int i=0; i<sizeof(Settings); i++) 
    if (EEPROM.read(i)!=((byte*)settings)[i]) { 
      EEPROM.write(i,((byte*)settings)[i]);
      changed = true;  
    };  

  if (changed) EEPROM.commit();
  */
  /*
  settings->crc = crc32b(((byte*)settings),sizeof(Settings) - sizeof(settings->crc));
  for (int i=0; i<sizeof(Settings); i++) EEPROM.update(i,((byte*)settings)[i]);
  EEPROM.commit();
  */
      
  
};

void onUpdateSettings(Settings *settings)
{
  //onUpdateSettings(&settings);

  if (!memnchr(settings->privKey0,0,sizeof(settings->privKey0))) 
    setScreenStatus("NO DATA");
  else 
    if (settings->pin0[0]==0) //PIN not set
       setScreenStatus("PIN NOT SET");
    else 
      if (
          (strcmp(settings->pin0,state.pin)==0) 
          ||
          ((strcmp(settings->pin1,state.pin)==0) && (settings->pin1[0]!=0))
         )
         setScreenStatus("UNLOCKED");   
      else   
         if (pinPos>0)
           setScreenStatus("ENTER PIN");
         else
           setScreenStatus("LOCKED");
  
};  

void initialSettingsLoading(void) {
  Settings settings;
  readSettings(&settings);

  //Serial.print(F("settings.crc=")); Serial.println(settings.crc);
  //Serial.print(F("settings.buttonMode=")); Serial.println(settings.buttonMode);

  if (!memnchr(settings.privKey0,0,sizeof(settings.privKey0))) screenShow("No key data.\nPlease set private key");
  else if (settings.pin0[0]==0) screenShow("Pin is not set.\nPlease set pin");
  else screenShow("ready");  
  
};

void setup() {
  #ifdef OPTION_button 
    pinMode(OPTION_button,INPUT_PULLUP);
  #endif   
  
  Serial.begin(115200);
  while (!Serial);

  memset(&state,0,sizeof(State));
  
  Serial.print(F("openHW v"));  Serial.print(VERSION);  Serial.println(F(" Type 'help' for help\n"));
  uECC_set_rng(&RNG);

  screenInit();

  //load settings
  //Serial.print(F("BEFORE ")); printMemory();
  initialSettingsLoading();
  //Serial.print(F("AFFTER ")); printMemory();
  
  pinPos=0; 
  cmdLen=0; 

  #ifdef DEBUG
    Serial.print(F("#setup() done: "));printMemory();
  #endif  
}

//CDC void codeDecodePrivateKey(uint8_t * privkey, uint8_t salt);
void codeDecodePrivateKey(uint8_t * privkey);

//byte __attribute__ ((noinline))  scfStatus() {
byte scfStatus() {
  //(EMPTY/LOCKED/UNLOCKED/NOPIN)
  debugPrint(F("scfStatus called."),1,1);
  Settings settings;
  readSettings(&settings); 

  #ifdef DEBUG
    Serial.print(F("#=======================Status for ")); Serial.println(millis());
    Serial.print(F("#Status for ")); Serial.println(millis());
    Serial.print(F("#lastPinEnter = ")); Serial.println(state.lastPinEnter);
    Serial.print(F("#pin: ")); Serial.println(state.pin);
    Serial.print(F("#lastMode1Millis= ")); Serial.println(state.lastMode1Millis);    
    Serial.print(F("#password= ")); hex8Serial(state.password,state.passwordLen); Serial.println();    
    Serial.print(F("#passwordLen= ")); Serial.println(state.passwordLen);        
    Serial.print(F("#buttonMode= ")); Serial.println(state.buttonMode);        
    Serial.print(F("#passwordProtected= ")); Serial.println(state.passwordProtected);        
    
    
    Serial.println(F("#-----------")); 
    //ibuf,cmdLen-1,&pinPos
    Serial.print(F("#ibuf= ")); hex8Serial((unsigned char*)(ibuf),cmdLen); Serial.println();
    Serial.print(F("#cmdLen= ")); Serial.println(cmdLen);        
    Serial.print(F("#pinPos= ")); Serial.println(pinPos);        
    
  #endif


  if (!memnchr(settings.privKey0,0,sizeof(settings.privKey0))) {
    Serial.println(F("EMPTY"));
    return 0; //no main key
  };
  
  if (settings.pin0[0]==0) {
    Serial.println(F("NOPIN"));
    return 0; //no pin set
  };

  if (
     (strcmp(settings.pin0,state.pin)==0)
     ||
     ((strcmp(settings.pin1,state.pin)==0) && (settings.pin1[0]!=0))
     ) 
   {
    Serial.println(F("UNLOCKED"));
    return 0; //no pin set
   };

  Serial.println(F("LOCKED"));
  return 0;
}

byte scfHelp() {
/*
  for (int i = 0; i < helpMessagesLen; i++) {
    //strcpy_P(tBuf, (char *)pgm_read_word(&(helpMessages[i])));  // Necessary casts and dereferencing, just copy.
    int t = pgm_read_word(&(helpMessages[i]));
    //strcpy_P(tBuf,t);
    //Serial.println((char*)tBuf);
    Serial.print(i);Serial.print(" : "); Serial.println(t);
  };  
  //for (byte i = 0; i<helpMessagesLen)
  //for (byte k = 0; k < strlen_P(helpMessages[i]); k++)  Serial.print(pgm_read_byte_near(helpMessages + k));  
*/  
  Serial.print(F("openHW "));Serial.println(VERSION);
  
  Serial.print(F("help : show this text\n"));
  Serial.print(F("setPrivateKey(<private1>[,<private2>]) : sets private key(s). Data in code58check form.\n"));
  Serial.print(F("  it will erase all privateKeys and set all pins to "".\n"));
  Serial.print(F("  it also drops all passwords. You must set it again (assuming that the old one is empty) .\n"));
  Serial.print(F("getPublicKey : gets current public key\n"));
  Serial.print(F("signMessage(<message>[,bip66padding]) : signs the message. Message in hex. Result in hex\n"));
  Serial.print(F("ECDH(<pubkey>) : create a shared secret for ECDH. Public Key must be uncompressed\n"));
  //Serial.print(F(" (*) signTX(TXdata>,<nOut>) : signs transaction input nOut. Message in hex. Result in hex\n"));
  //Serial.print(F("signTXDigest(<TXdata>[,bip66padding]) : signs transaction digest. Data in hex. Result in hex\n"));
  Serial.print(F("setPin(<pin>) : sets current pin. Pin is text (don't use \")\" in pin)\n"));
  Serial.print(F("setPin2(<pin>) : sets plausible denial pin. Pin is text (don't use \")\" in pin)\n"));
  Serial.print(F("  WARNING: setting any of the pins in P.D. mode will make P.D. key main and will erase main private key."));  

  Serial.print(F("setPassword([<old password hex>,]<new password hex>) : set password for private key in memory\n"));
  Serial.print(F("  the keys will be encrypted in memory with the password\n"));  
  Serial.print(F("  maximum (and recommended) password size: 16 bytes\n"));    
  Serial.print(F("  if you use 1 argument, you just set the current password\n"));  
  Serial.print(F("  if you use 2 arguments, the password will be changed from first to second\n"));
  Serial.print(F("  in PD mode password will be changed only for PD key.\n"));
    
  Serial.print(F("setPinMode(<N>) : sets pin request mode. 0 for ask once; 1 for asking every time\n"));  
  Serial.print(F("setPinLockDelay(<Milliseconds>) : sets time for pin lock after enter\n"));
  Serial.print(F("setButtonMode(<N>) : sets button mode. 0: button is not used; 1: use only for pin confirmatuion; 2: use for Private Key access confirmation\n"));
  Serial.print(F("getConfig([parname]) : Returns configuration. Call getConfig() for all params with names\n"));        

  Serial.print(F("status : Returns current status (EMPTY/LOCKED/UNLOCKED/NOPIN)\n"));
  Serial.print(F("lock : lock device. Returns OK or ERROR (if pin not set))\n"));
  Serial.print(F("unlock : unlock device. Returns OK or ERROR\n"));
  Serial.print(F("helloHW : Returns hello string (HELLO OPENHW <version>)\n"));          
  #ifdef DEBUG
    Serial.print(F("test(<parname>) : show test # (1,...) or calc sha256 \n"));        
  #endif
  Serial.print(F("===automation tips===\n"));
  Serial.print(F("Command is case-insensetive (you can use UPPERCASE or whatever))\n"));  
  Serial.print(F("You can add you pin to the end of the command line (for pin=1234))\n"));
  Serial.print(F("  GetPublicKey1234\n"));  
  Serial.print(F("There are 5 possibles answers:\n"));
  Serial.print(F("0. \"#comment.\" appears only in debug mode. Ignore it\n"));  
  Serial.print(F("1. \"OK: comment.\" Operation is succeseful\n"));    
  Serial.print(F("2. \"ERROR: comment.\" Operation is not succeseful\n"));  
  Serial.print(F("3. \"PIN: comment\" PIN request\n"));    
  Serial.print(F("4. \"PASSWORD: comment\" Password request\n"));      
  Serial.print(F("5. \"BUTTON: comment\" Hardware button pressing request. Don't need any action from the host\n"));  
  Serial.print(F("6. \"DATA\" Requested data\n"));  
  Serial.print(F("===Examples===\n")); 
  Serial.print(F("Status\n"));
  Serial.print(F("  prints current device state\n"));
  Serial.print(F("unlock1234\n"));
  Serial.print(F("  tries to unlock with pin 1234.\n"));
  Serial.print(F("getPublicKey\n"));
  Serial.print(F("  prints current public key (uncompressed). PIN might be requested\n"));  
  Serial.print(F("setPrivateKey(cNpjQkzqzoEvgg1GhHQ7JY2cNjh97rhqS1wDH3m7EDnB1T5hrB5D)\n"));
  Serial.print(F("  Sets main private key (no PD key sets)\n"));  
  Serial.print(F("setPrivateKey(cNpjQkzqzoEvgg1GhHQ7JY2cNjh97rhqS1wDH3m7EDnB1T5hrB5D,cTrcwmE5bi2D6RRpWRLssGgLZweV5QZDyX1KTj5MZw279sjoJSyJ)\n"));  
  Serial.print(F("  Sets main private key and a PD key\n"));    
  Serial.print(F("setPrivateKey(cNpjQkzqzoEvgg1GhHQ7JY2cNjh97rhqS1wDH3m7EDnB1T5hrB5D)\n"));
  Serial.print(F("  Sets main private key (no PD key sets)\n"));  
  Serial.print(F("signMessage(E3FFCC983D047EAB781C31AFA21B71AD15393C3B3EBD9944F278652F85E18266)\n"));  
  Serial.print(F("  signs the digest (it is \"test\" substring digest for Emer magic word)\n"));    
  Serial.print(F("signMessage(E3FFCC983D047EAB781C31AFA21B71AD15393C3B3EBD9944F278652F85E18266,1)\n"));  
  Serial.print(F("  signs the digest for transaction padding (use it for sign transactions)\n"));     
  Serial.print(F("setPassword(,00112233445566778899AABBCCDDEEFF)\n"));  
  Serial.print(F("  sets the password assuming where was no password before\n"));     
  Serial.print(F("setPassword(AABBCCDDEEFF,00112233445566778899AABBCCDDEEFF)\n"));   
  Serial.print(F("  sets the password assuming that old password is AABBCCDDEEFF\n"));     
  Serial.print(F("setPassword(00112233445566778899AABBCCDDEEFF)\n"));   
  Serial.print(F("  just set current passwors (do not change it in EEPROM)\n"));       
  Serial.print(F("  WARNING: There is no way to tell the right password from the wrong one\n"));
  Serial.print(F("  Any password will be accepted, but the private key will be different from the specified one.\n"));  
  Serial.print(F("  Use getPublicKey for determine private key correctness\n"));    
  
};

//=========================================================== cmd functions ================================
//for saving memory:
size_t checkAndDecodePrivateKey(const char *data, size_t len, uint8_t *result) { //res must be 32 bytes
  debugPrint(F("checkAndDecodePrivateKey called."),1,1);

  //res has size = 1(sign) + 32(D) + 1(01) + 4(crc). HD is not supported. BIP39/BIP32 is not supported
  unsigned char res[80]; // with a reserv. we need so much memory for the DecodeBase58 cp*2

  //Serial.print(F("Ready to call DecodeBase58.")); printMemory(); 
  
  int datalen =  decodeBase58((unsigned char *)data, len , res);

  //Serial.print(F("DecodeBase58 called."));printMemory();
  
  //Serial.print(F("res="));  for (int i=0; i<datalen; i++) {Serial.print(res[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));
  

  if (datalen==38) {
    //check crc
    #ifdef SHA256_SUPPORTED
      uint8_t hash[32];
      #ifdef DEBUG
      Serial.print(F("#Ready to calc SHA256. "));printMemory();
      #endif
      if (!checkMemory(350)) return -1;
      SHA256(res,34,hash);
      SHA256(hash,32,hash);
      #ifdef DEBUG
      Serial.print(F("#SHA256 calculated. "));printMemory();
      Serial.print(F("#hash=")); for (int i=0; i<32; i++) {Serial.print(hash[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));      
      #endif
      for (int i=0; i<4; i++) if (hash[i]!=res[34+i]) {
        Serial.println(F("ERROR: Private key not assigned: wrong crc"));
        screenShow("ERROR: wrong crc");
        return 0;
      };
    #endif  
    memcpy(result,res+1,32);
    return 32;
  };    

 
  Serial.println(F("ERROR: Private key assigned: wrong key length"));
  screenShow("ERROR: wrong key length");
  
  return 0;
}

void dropSettings(Settings *settings);

void dropSettings(Settings *settings){
    settings->attCounter = 0; //attempt counter. Zero it if known pin
    settings->mode1 = 0; //if true, we have entered correct PIN1. So, we are working with privkey1 until the pin0 will be entered. it is possible only if no pins entered for one week
    settings->passwordProtected = 0; //if the key is encrypted by password. If state.password not entered then show error in case the privkey queried

    memset(settings->privKey0,0,sizeof(settings->privKey0));
    memset(settings->privKey1,0,sizeof(settings->privKey1));
    settings->pin0[0]=0;
    settings->pin1[0]=0;

    state.passwordProtected = false;
    state.lastPinEnter = 0; //millis of the last pin enter (no matter if it was correct or not
    state.pin[0]=0;
    state.lastMode1Millis = 0; //last millis attempt for mode 1  
    state.passwordLen = 0; //password for keys protection. 16 bytes
}

void savePrivateKey(const uint8_t *data,byte n){
  //safe key #n
  //saving key 0 removes key 1
  Settings settings;
  readSettings(&settings);

  //drop password in any case
  state.passwordProtected = false;
  settings.passwordProtected = false;
  state.passwordLen = 0;

  if (n==0) {
    dropSettings(&settings);
    if (data==NULL) memset(settings.privKey0,0,sizeof(settings.privKey0));
    else memcpy(settings.privKey0,data,32);
    settings.pin0[0]=0;
  } else {
    if (data==NULL) memset(settings.privKey1,0,sizeof(settings.privKey1));
    else memcpy(settings.privKey1,data,32);
    settings.pin1[0]=0;
  };
  writeSettings(&settings);
  onUpdateSettings(&settings);
};

//byte __attribute__ ((noinline)) doSetPrivateKey(const char *data, size_t len) {  --> use -Og... this is not working
byte doSetPrivateKey(const char *data, size_t len) {
  //return 1 for pin request
  if (!(askForButton())) return 0;
  //if (!unlocked()) return 1;  ->We don't need it to set pins
  //debugPrint(F("doSetPrivateKey called."),1,1);
  #ifdef DEBUG
    Serial.print(F("#Set private key: ")); for (int i=0; i<len; i++) Serial.print((char)data[i]); Serial.println(); printMemory();
  #endif  

  //<data>[,<data2>]
  char *cpa = (char*)memchr(data,',',len);

  uint8_t res[32];
  uint8_t pub[64];

  //first key
  if (32==checkAndDecodePrivateKey(data, ((cpa==NULL)?len:(char*)cpa - data), res)) {
    debugPrint(F("ready to uECC_compute_public_key."),1,1);
    
    if (uECC_compute_public_key(res,pub,curve)) {
      //clear privkey 1 first
      savePrivateKey(NULL,1);
      savePrivateKey(res,0);      
      
      if (!cpa) Serial.println(F("OK: Private key assigned"));
      if (!cpa) screenShow("OK: Private key assigned");
      #ifdef DEBUG
        Serial.print(F("#PUBKEY: ")); for (int i=0; i<64; i++) Serial.print(pub[i],HEX); Serial.println();
      #endif 
    } else {
      Serial.println(F("ERROR: Private key assigned: no public key"));
      screenShow("ERROR setting private key 1");
      return 0;
    };  
  };
  
  //PA key
 if (cpa) if (32==checkAndDecodePrivateKey(cpa+1,len+ (data - cpa) -1, res)) {
   debugPrint(F("PA key found, ready to decode."),1,1);
   if (uECC_compute_public_key(res,pub,curve)) {
      savePrivateKey(res,1);
      Serial.println(F("OK: Both keys were assigned"));
      screenShow("OK: Both keys were assigned");
      #ifdef DEBUG
        Serial.print(F("#PUBKEY2: ")); for (int i=0; i<64; i++) Serial.print(pub[i],HEX); Serial.println();
      #endif 
    } else {
      Serial.println(F("ERROR: Only first key was assigned"));
      screenShow("ERROR Only first key was assigned");
      return 0;
    };     
 }
};

//#if defined(ESP8266) || defined(ESP31B) || defined(ESP32)
 void doDelay(unsigned long ms) {
    //#if defined(ESP31B) || defined(ESP32)
    //wdt_disable();
    //#endif
    for (unsigned long i=(ms/100); i>0; i--) {
      delay(100); 
      #ifdef ESP8266
        wdt_reset();
      #endif  
      //delay(1);
    };  
    //#if defined(ESP31B) || defined(ESP32)
    //wdt_enable(0);
    //#endif    
  /*
    wdt_disable();
    delay(ms);
    wdt_enable(0);
  */
    
 /* 
    unsigned long mms = ms;
    while (mms>0) {
      if (mms<1000) {
        delay(mms); 
        mms = 0;
      } else {
        delay(1000);
        #ifdef DEBUG
          Serial.print(F("."));
        #endif  
        mms = mms - 1000;
      };
        
    };
   */ 
 };
    
void unlockedFailed(Settings *settings);

void unlockedFailed(Settings *settings){
  settings->attCounter += 1;
  writeSettings(settings);
  //doDelay((1UL <<(settings->attCounter-1) )*100);
  onUpdateSettings(settings); 
  screenShow("Wrong PIN");
  //drop all
  state.pin[0] = 0;
  //!!pinPos=0;
  if (DROP_IF_WRONG_PIN) cmdLen=0;     
};

void unlockedSuccess(Settings *settings);
void unlockedSuccess(Settings *settings){
  settings->attCounter = 0;
    
  writeSettings(settings);
  onUpdateSettings(settings); screenShow("Access granted");
  if (settings->askPinEveryTime) state.pin[0] = 0;
};

//bool __attribute__ ((noinline)) unlocked() 
bool unlocked() 
//returns if the 
// INCREASES ATTEMPT COUNTER AND MAKES DELAY
{ 
  debugPrint(F("unlocked() called."),1,1);
  Settings settings;
  readSettings(&settings); 
  if (settings.pin0[0]==0) {
    state.pin[0] = 0; //set current pin to ""
    return true;
  }
  if (state.pin[0]==0) return false;
  
  //byte attCounter = 0; //attempt counter. Zero it if known pin
  //bool mode1 = 0; //if true, we have entered correct PIN1. So, we are working with privkey1 until the pin0 will be entered. it is possible only if no pins entered for one week
  //unsigned long lastMode1Millis = 0; //last millis attempt for mode 1

  //do delay
  doDelay((1UL <<(settings.attCounter-1) )*100);
   
  //if pin1 entered
  if (strcmp(settings.pin1,state.pin)==0) {
    //pin1 used
    settings.mode1 = true;
    unlockedSuccess(&settings);
    debugPrint(F("access granted: pin1."),1,1);    
    return true;
  }

  //5 days delay for come back to the correct pin
  if (settings.mode1 && ((millis()-state.lastMode1Millis) <(1000*432000))) {
    //too early to try
    state.lastMode1Millis = millis();
    unlockedFailed(&settings);
    debugPrint(F("access denied: mode1."),1,1);
    return false;
  };

  //perhaps it is pin0?
  if (strcmp(settings.pin0,state.pin)==0) {
    //pin0 used
    settings.mode1 = false;
    state.lastMode1Millis = 0;
    settings.attCounter = 0;
    unlockedSuccess(&settings);
    debugPrint(F("access granted: mode0 "),1,1);
    return true;
  };

  //wrong password
  unlockedFailed(&settings);  
  debugPrint(F("access denied: mode0."),1,1);
  return false;  
};

bool getPrivateKey(uint8_t res[32]) 
{ //returns private key corresponding current pin, 0key if pin0[0]==0
  debugPrint(F("getPrivateKey called."),1,1);  
  Settings settings;
  readSettings(&settings);

  if (!memnchr(settings.privKey0,0,sizeof(settings.privKey0))) return 0; //no main key

  int n=-1;
  if (settings.pin0[0]==0) n=0;
  else if (strcmp(settings.pin0,state.pin)==0) n=0;
  else if (strcmp(settings.pin1,state.pin)==0) n=1; 
  else return false;

  if (n==0) memcpy(res,settings.privKey0,sizeof(settings.privKey0));
  else      memcpy(res,settings.privKey1,sizeof(settings.privKey1));

  //CDC codeDecodePrivateKey(res,n);
  codeDecodePrivateKey(res);

  debugPrint(F("getPrivateKey success"),1,1);  
  return true;
};

bool getPublicKey(uint8_t pub[64]) {
  //returns if success.
  debugPrint(F("getPublicKey called."),1,1);
  uint8_t res[32];
  if (getPrivateKey(res)) 
    return uECC_compute_public_key(res,pub,curve);
  else return false;
};

bool checkPassword() {
  if ((state.passwordProtected) && (state.passwordLen==0)) {
    Serial.println(F("ERROR: password not set"));
    screenShow("ERROR: password not set");    
    return false;
  } else return true;
};

byte doGetPublicKey() {
  debugPrint(F("doGetPublicKey called."),1,1);
  //return 1 for pin request
  
  if (!checkPassword()) return 0;

  //if ((state.buttonMode>1)||((state.buttonMode==1)&&(state.pin[0]==0))) if (!(askForButton())) return 0;
  if (state.buttonMode>1) if (!(askForButton())) return 0;
  if (!unlocked()) {
    debugPrint(F("doGetPublicKey :!unlocked"),1,1);
    if ((state.buttonMode==1)&&(pinPos==0)) if (!(askForButton())) return 0;
    return 1; 
  }
  

  uint8_t pub[64];
  if (getPublicKey(pub)) {
    hex8Serial(pub,64); Serial.println();
    screenShow("Public key requested");    
  } else {
    Serial.println(F("ERROR: can't access public key"));
    screenShow("ERROR: can't access public key");    
  };
  return 0;
};

byte doGetConfig(const char *data, size_t len){
  #ifndef DEBUG
    //if ((state.buttonMode>1)||((state.buttonMode==1)&&(state.pin[0]==0))) if (!(askForButton())) return 0;
    //if (!unlocked()) return 1; 
    if (state.buttonMode>1) if (!(askForButton())) return 0;
    if (!unlocked()) {
      if ((state.buttonMode==1)&&(pinPos==0)) if (!(askForButton())) return 0;
      return 1; 
    }
  #endif
  Settings settings;
  readSettings(&settings);

  if (len>0) {
    Serial.println(F("ERROR: Sorry, not supported yet"));
    return 0;
  };
    
  Serial.println(F("OK: openHW configuration:"));


  #ifdef DEBUG
   Serial.print(F("#privKey0:")); hex8Serial(settings.privKey0,32); Serial.println();
   Serial.print(F("#privKey1:")); hex8Serial(settings.privKey1,32); Serial.println();
   Serial.print(F("#pin0:")); Serial.println(settings.pin0);
   Serial.print(F("#pin1:")); Serial.println(settings.pin1);
   Serial.print(F("#attCounter:")); Serial.println(settings.attCounter);
   Serial.print(F("#mode1:")); Serial.println(settings.mode1);
  #endif

  Serial.print(F("Name:")); Serial.println(settings.uname);
  Serial.print(F("Ask pin every time:")); Serial.println(settings.askPinEveryTime);
  Serial.print(F("Lock pin after (ms):")); Serial.println(settings.lockPinAfterMs);
  Serial.print(F("Button mode: "));
  switch (settings.buttonMode) {
    case 0:
      Serial.println(F("do not use button"));
      break;
    case 1:
      Serial.println(F("use only for unlock pin"));
      break;
    case 2:
      // statements
      Serial.println(F("to be used at each protected operation"));
      break;
    default:      
      Serial.println(settings.buttonMode);
      break;
  }
  Serial.print(F("WIF:")); Serial.println(settings.WIF,HEX);
  Serial.print(F("Sig:")); Serial.println(settings.Sig,HEX);

  #ifdef DEBUG
    Serial.print(F("#CRC:")); Serial.println(settings.crc,HEX);
  #endif
  
  return 0;
};

byte scfUnlock(){
  //if (!unlocked()) {
  //  if ((state.buttonMode>1)||((state.buttonMode==1)&&(state.pin[0]==0))) if (!(askForButton())) return 0;
  //  return 1;
  //}
  if (!unlocked()) {
    #ifdef DEBUG
      Serial.print(F("scfUnlock: !unlocked pinpos="));Serial.println(pinPos);
    #endif  
    if ((state.buttonMode>0)&&(pinPos==0)) if (!(askForButton())) return 0;
    return 1; 
  }

  Settings settings;
  readSettings(&settings); 
  
  if (settings.pin0[0]==0) {
    screenShow("WARNING: pin not set");
    Serial.println(F("ERROR: pin not set"));
  } else {
    screenShow("unlocked by user");
    Serial.println(F("OK: unlocked"));
  };

  return 0; 
}; 

void updateSettings(){
  Settings settings;
  readSettings(&settings); 
  onUpdateSettings(&settings);
};

byte scfLock(){
  
  Settings settings;
  readSettings(&settings); 
  state.pin[0] = 0;
  onUpdateSettings(&settings);
  if (settings.pin0[0]==0) {
    screenShow("lock error: pin not set");
    Serial.println(F("ERROR: pin not set"));
  } else {
    screenShow("locked by user");
    Serial.println(F("OK: locked"));
  };
  
  return 0;  
}; 

byte doSetPin(const char *data, size_t len, byte pinNo){
  debugPrint(F("doSetPin called."),1,1);
  if (!unlocked()) return 1;
  //if (state.buttonMode>0) 
  if (!(askForButton())) return 0; 

  //if we in mode 1 (pin = pin1): move priv1 to priv0 and set pin0; erase pin1 and priv1
  //otherwise just set pin0 or pin1
  Settings settings;
  readSettings(&settings);

  if (settings.mode1) {
    memcpy(settings.privKey0,settings.privKey1,sizeof(settings.privKey0));
    //CDC codeDecodePrivateKey(settings.privKey0,1);codeDecodePrivateKey(settings.privKey0,0);
    memcpy(settings.pin0,settings.pin1,sizeof(settings.pin0));
    memset(settings.privKey1,0,sizeof(settings.privKey1));
    memset(settings.pin1,0,sizeof(settings.pin1));
  };

  //setting pin
  //first of all: decode it. trim by PIN_MAX_SIZE
  
  if (pinNo==0) {
    //main pin
    memset(settings.pin0,0,sizeof(settings.pin0));
    memcpy(settings.pin0,data,len);
  } else {
    memset(settings.pin1,0,sizeof(settings.pin1));
    memcpy(settings.pin1,data,len);
  };

  //now current pin is pin0. We are in normal mode.
  memcpy(state.pin,settings.pin0,sizeof(state.pin));
  
  settings.mode1 = false;
  state.lastMode1Millis = 0;
  settings.attCounter = 0;
  
  writeSettings(&settings);
  onUpdateSettings(&settings);

  Serial.println(F("OK: PIN has been set up"));
  screenShow("The PIN was changed");
  return 0;  
};

byte doECDH(const char *data, size_t len){
  debugPrint(F("doECDH called."),1,1);
  #ifdef DEBUG
    Serial.print(F("#data="));  for (size_t i=0; i<len; i++) Serial.print((char)data[i]); Serial.print(F(" len= ")); Serial.println(len);
  #endif

  
  if (len!=128) {
    Serial.println(F("ERROR: wrong public key length"));
    screenShow("ERROR: wrong pubkey");    
    return 0;  
  };

  if (!checkPassword()) return 0;
  //if ((state.buttonMode>1)||((state.buttonMode==1)&&(state.pin[0]==0))) if (!(askForButton())) return 0;
  //if (!unlocked()) return 1;
  if (state.buttonMode>1) if (!(askForButton())) return 0;
  if (!unlocked()) {
    if ((state.buttonMode==1)&&(pinPos==0)) if (!(askForButton())) return 0;
    return 1; 
  }
  
      
  uint8_t pub[64];
  uint8_t privkey[32];
  uint8_t res[32];
  
  bufFromHex(data, len, pub);  

  if (getPrivateKey(privkey)) {
    //Returns 1 if the shared secret was generated successfully, 0 if an error occurred.
    if ( 
         uECC_shared_secret(
           pub, //const uint8_t *public_key,
           privkey, //const uint8_t *private_key,
           res,  //uint8_t *secret,
           curve //uECC_Curve curve
         )
       ){
          hex8Serial(res,32); Serial.println();
          screenShow("ECDH secret made");        
        } else {
          Serial.println(F("ERROR: ECDH secret creation error"));
          screenShow("ERROR: ECDH secret error");    
        };
  } else {
    Serial.println(F("ERROR: can't access private key"));
    screenShow("ERROR: can't access private key");    
  };  
  return 0;
}

#ifdef OPTION_button
byte askForButton(const bool showHelp) {
  //wait for the button pressed. returns 1 if pressed 
  //1. show messages
  //2. wait until pressed or timeout
  // returns if pressed
  if (showHelp) {
     setScreenStatus("PRESS BUTTON");
     Serial.println(F("BUTTON: press hardware button"));
     screenShow("Press Hardware Button");    
  };

 byte res=0;
 unsigned long stAt = millis();
 while (((res=digitalRead(OPTION_button))) && ((millis()-stAt)<BUTTON_TIMEOUT_MS )) {
  delay(1);
  #ifdef ESP8266
    wdt_reset();
  #endif  
 };
 
  
 if (showHelp) {
   updateSettings(); //set normal state
   if (!res) {
     screenShow("Button pressed");
   } else {
     Serial.println(F("ERROR: button not pressed"));
     screenShow("ERROR: button timeout");        
   }  
 };
 return 1 - res;  
}
#endif



//CDC void codeDecodePrivateKey(uint8_t * privkey, uint8_t salt){
void codeDecodePrivateKey(uint8_t * privkey){
  //just xor for now. it is safe IF we use the password ONLY with THIS device and THIS key
  if (!(state.passwordProtected)) return;
  
  if (!memnchr(privkey,0,32)) return; //not set

  uint8_t hash[32];

 /* //CDC 
  hash[0] = salt;
  for (int i=0; i<31; i++) hash[i+1] = state.password[i % state.passwordLen];
  
  #ifdef SHA256_SUPPORTED
    if (!checkMemory(350)) return;
    //SHA256(state.password,state.passwordLen,hash);
    SHA256(hash,32,hash);
  // # else
  // for (int i=0; i<32; i++) hash[i] = state.password[i % state.passwordLen];
  #endif
 */ 
  #ifdef SHA256_SUPPORTED
    if (!checkMemory(350)) return;
    SHA256(state.password,state.passwordLen,hash);
  #else
    for (int i=0; i<32; i++) hash[i] = state.password[i % state.passwordLen];
  #endif
  
  #ifdef DEBUG
    Serial.print(F("#codeDecodePrivateKey :")); hex8Serial(state.password,state.passwordLen); Serial.print(F(" ; state.passwordLen=")); Serial.println(state.passwordLen);
    Serial.print(F("#hash="));  hex8Serial(hash,32); Serial.println();
  #endif

    
  for (int i=0; i<32; i++) privkey[i] = privkey[i] ^ hash[i];
};

byte doSetPassword(const char *data, size_t len){
  if (!unlocked()) return 1; 
  // old[,new]

  //if (!askForButton()) {
  //    Serial.println(F("ERROR: button not pressed"));
  //    screenShow("ERROR: button not pressed");
  //    return 0; 
  //}
  
  //<data>[,<data2>]
  char *cpa = (char*)memchr(data,',',len);

  int dataLen = len;
  if (cpa!=NULL) dataLen=(cpa-data);
  if (dataLen>0) {
    if (!(state.passwordProtected)) {
      Serial.println(F("ERROR: not password protected"));
      screenShow("ERROR: not password protected");      
      return 0;
    };    
    if ((dataLen/2)>PASSWORD_LEN) {
      Serial.println(F("ERROR: password too long"));
      screenShow("ERROR: password too long");          
      return 0;
    };
    bufFromHex(data, dataLen, state.password);
    state.passwordLen = (dataLen/2);
  } else {
    state.passwordLen = 0;
  };

  #ifdef DEBUG
    Serial.print(F("#doSetPassword : state.password= ")); hex8Serial(state.password,state.passwordLen); Serial.println();
    Serial.print(F("#state.passwordLen=")); Serial.println(state.passwordLen);
  #endif

  if (cpa) {
    //change password mode 
    if ((!(state.passwordProtected)) && ((data - cpa)!=0)) {
      Serial.println(F("ERROR: password was not protected"));
      screenShow("ERROR: password was not protected");      
      return 0;
    };

    //recrypt passwords.
    //if we are in CP mode: reencrypt only CP private key. Otherwise both.
    //decrypt them first
    Settings settings;
    readSettings(&settings);

    if (!memnchr(settings.privKey0,0,sizeof(settings.privKey0))) {
      Serial.println(F("ERROR: private kay not set"));
      screenShow("ERROR: private kay not set");      
      return 0;    
    };

    debugPrint(F("doSetPassword codeDecodePrivateKey... "),1,1);
    //CDC codeDecodePrivateKey(settings.privKey1,1);
    codeDecodePrivateKey(settings.privKey1);
    //CDC if (! settings.mode1) codeDecodePrivateKey(settings.privKey0,0);    //decrypt settings.privKey0
    if (! settings.mode1) codeDecodePrivateKey(settings.privKey0);    //decrypt settings.privKey0

    debugPrint(F("doSetPassword setting new private keys... "),1,1);
    //the new password started from cpa+1  and the length is len+ (data - cpa) -1
    //set it
    if ((len+ (data - cpa) -1)==0) {
      //no new passwords 
      state.passwordLen = 0;
      settings.passwordProtected = false;
      state.passwordProtected = false;
    } else {

      #ifdef DEBUG
        Serial.print(F("#doSetPassword : cpa+1= ")); hex8Serial((uint8_t *)(cpa+1),len+ (data - cpa) -1); Serial.println();
        Serial.print(F("#len= ")); Serial.print(len);Serial.print(F("; len+ (data - cpa) -1 = ")); Serial.println(len+ (data - cpa) -1);
      #endif      
      
      if (((len+ (data - cpa) -1) / 2)>PASSWORD_LEN) {
        Serial.println(F("ERROR: new password too long"));
        screenShow("ERROR: new password too long");          
        return 0;
      };
      
      bufFromHex(cpa+1, len+ (data - cpa) -1, state.password);
      state.passwordLen = (len+ (data - cpa) -1) / 2;      

      settings.passwordProtected = true;
      state.passwordProtected = true;      
    };

    debugPrint(F("doSetPassword encrypt the keys again... "),1,1);
    //encrypt the keys again
    //CDC codeDecodePrivateKey(settings.privKey1,1);
    codeDecodePrivateKey(settings.privKey1);
    //CDC if (! settings.mode1) codeDecodePrivateKey(settings.privKey0,0);    //decrypt settings.privKey0
    if (! settings.mode1) codeDecodePrivateKey(settings.privKey0);    //decrypt settings.privKey0

    //and save them
    writeSettings(&settings);
    onUpdateSettings(&settings);

    Serial.println(F("OK: password has been changed"));
    screenShow("The password was changed");    
  } else {
    Serial.println(F("OK: Password set up")); 
  };
  return 0;
}

byte doSetButtonMode(const char *data, size_t len){
  debugPrint(F("doSetButtonMode called."),1,1);
  if (len!=1) {
    Serial.println(F("ERROR: ButtonMode 0,1,2 are allowed"));
    screenShow("ERROR set ButtonMode");      
    return 0;
  };
  if (!(askForButton())) return 0;

  Settings settings;
  readSettings(&settings); 

  //0..9
  settings.buttonMode = data[0] - '0';
  
  writeSettings(&settings);
  onUpdateSettings(&settings);

  Serial.println(F("OK: ButtonMode set"));
  screenShow("ButtonMode set");    

  return 0;
}

byte doSignMessage(const char *data, size_t len){
  debugPrint(F("doSignMessage called."),1,1);
  //return 1 for pin request
  if (!checkPassword()) return 0;  
  //if ((state.buttonMode>1)||((state.buttonMode==1)&&(state.pin[0]==0))) if (!(askForButton())) return 0;
  //if (!unlocked()) return 1; 
  if (state.buttonMode>1) if (!(askForButton())) return 0;
  if (!unlocked()) {
    if ((state.buttonMode==1)&&(pinPos==0)) if (!(askForButton())) return 0;
    return 1; 
  }
  
  
  //<hexdata>[,<bip66padding>]
  char *cpa = (char*)memchr(data,',',len);

  bool bip66padding = false; 
  int dataLen = len;
  if (cpa!=NULL) {
    bip66padding = ('0'!=cpa[1]);
    dataLen=(cpa-data);
  }

  //calc hash
  uint8_t hash[dataLen/2];
  bufFromHex(data, dataLen, hash);

  #ifdef DEBUG
    Serial.print(F("#Signing ")); hex8Serial(hash,dataLen/2); Serial.println();
    if (bip66padding) Serial.println(F("#bip66padding mode"));
    Serial.print(F("#dataLen=")); Serial.println(dataLen);
    if (bip66padding) {Serial.print(F("#bip66padding key=")); Serial.println(cpa[1]);};
  #endif

  uint8_t privkey[32];
  if (getPrivateKey(privkey)) {
    /*
    int uECC_sign(const uint8_t *private_key,
              const uint8_t *message_hash,
              unsigned hash_size,
              uint8_t *signature,
              uECC_Curve curve);
    */ 
    uint8_t sign[64];
    byte opres = 0;
    byte tc = 0;
    do {
      opres = uECC_sign(
        privkey,   //private key
        hash,  //hash
        dataLen/2, //hash size
        sign,
        curve
      );  
      tc++;      
    } while ( (opres) && (bip66padding) && ((sign[0]>=0x80)||(sign[32]>=0x80)) && (tc<200) );
    
    if ((opres)&&(tc<200))  
    {
      hex8Serial(sign,64); Serial.println();
      screenShow("Signature made");        
    } else {
      Serial.println(F("ERROR: signing error"));
      screenShow("ERROR: signing error");    
      
    };
  } else {
    Serial.println(F("ERROR: can't access private key"));
    screenShow("ERROR: can't access private key");    
  };  
  return 0;
}

byte doTest(const char *data, size_t len) {
  //return 1 for pin request
  Serial.println(F("-----------------------------------------------------------------------------------------"));
  Serial.print(F("TEST called: ")); for (int i=0; i<len; i++) Serial << (char)data[i]; Serial.println();

 
 if ((len==1)&&(memcmp(data,"1",1))==0)  {  //"1"
   Serial.println(F("----- TEST 1: 100 times doSetPrivateKey(\"cNpjQkzqzoEvgg1GhHQ7JY2cNjh97rhqS1wDH3m7EDnB1T5hrB5D\") -----"));
   for (int i=0; i<100; i++) {
     Serial.print(i); Serial.print(F(": "));
     char tmps[54];
     getProgmemStr(TEST_KEY_1,tmps); doSetPrivateKey(tmps,52);
   };  
 } else {
  #ifdef SHA256_SUPPORTED
    /*
    int l=str.length();
    if (l>64) l=64;
    byte arr[64];
    for (int i=0; i<l; i++) arr[i] = str.c_str()[i];
    //test(gggggggggfffffffffffffffffffffffffffffffffg) 
    Serial.print("data=");  for (int i=0; i<l; i++) {Serial.print(arr[i],HEX); Serial.print(" ");};  Serial.println(" ");
    for (int v=0; v<10; v++) {
      uint8_t hash[32];
      SHA256(arr,l,hash);
      Serial.print(v); 
      Serial.print(": hash="); for (int i=0; i<32; i++) {Serial.print(hash[i],HEX); Serial.print(" ");};  Serial.println(" ");
      //SHA256(hash,32,hash);  
    };
    */
    Serial.print(F("#data="));  for (int i=0; i<len; i++) {Serial.print(data[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));
    for (int v=0; v<100; v++) {
      uint8_t hash[32];
      SHA256((uint8_t*)data,len,hash);
      Serial.print(v); 
      Serial.print(F(": hash=")); for (int i=0; i<32; i++) {Serial.print(hash[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));
      //SHA256(hash,32,hash);  
    };
    
  #else
    Serial.println(F("SHA256_SUPPORTED is turned off"));
  #endif    
 }; 
  
};
//=========================================================== main =========================================


int xmemcmp(const __FlashStringHelper * str, const char *data, size_t len) {

  if (strlen_P((PGM_P)str)>len) return -127;
//  #ifdef DEBUG
//    Serial.print(F("#XXXXXXXXXXXX: finding \"")); Serial.print(str); Serial.print(F("\" in ")); 
//    for (int i=0; i<len; i++) Serial.print(data[i]);  Serial.println();
//  #endif

  return memcmp_P(data,(PGM_P)str,strlen_P((PGM_P)str));

  //char d[strlen_P((PGM_P)str)+1];  d[strlen_P((PGM_P)str)]=0; memcpy(d,data,strlen_P((PGM_P)str));
  //return strcmp_P(d,(PGM_P)str);

  //int res=0;
  //for (byte k = 0; ((k <len)&&(res==0))  ; k++) 
  //    res = (char)pgm_read_byte_near(str + k) - (char*)data[k];  
};

//#if defined(ESP8266)
//#else
//#endif

bool doSerialCommand(const char *data, size_t len, size_t *pinPos) {

  if (data[0]==10) debugPrint(F("doSerialCommand called data[0]=\\n."),1,1);
  #ifdef DEBUG
 //   //Serial.print(F("#doSerialCommand: len=")); Serial.print(len); Serial.print(F(" data="));  hex8Serial((uint8_t*)data,len);  Serial.println();
//    Serial.print(F("#*****doSerialCommand: len=")); Serial.print(len); Serial.print(F(" data="));  for (size_t i=0; i<len; i++) Serial.print(data[i]);  debugPrint(F(" "),1,1);// Serial.println();
  #endif
  
  //we must set pinPos = Len if we need pin
  if (*pinPos>0) {
    //we have called with a pin
        
    byte pinSize = len - *pinPos;
    if (pinSize>PIN_MAX_SIZE) pinSize = PIN_MAX_SIZE;

    if (((char *)(data+*pinPos))[0]==10) {
      //cut \n from the beginning
      memcpy(state.pin,data+*pinPos+1,pinSize-1);
      state.pin[pinSize-1] = 0;       
    } else {
      memcpy(state.pin,data+*pinPos,pinSize);
      state.pin[pinSize] = 0;
    };

    #ifdef DEBUG
      Serial.print(F("["));Serial.print(state.pin);Serial.print(F("] *pinPos="));Serial.println(*pinPos,DEC);
    #endif      
  };

  //if (sCmd=="HELP") scfHelp();
  //else if ((sCmd.startsWith("SETPRIVATEKEY("))&&(sCmd.endsWith(")"))) doSetPrivateKey(sCmd.substring(14,sCmd.length()-1));
  //else if ((sCmd.startsWith("TEST("))&&(sCmd.endsWith(")"))) doTest(sCmd.substring(5,sCmd.length()-1));
  size_t res = 0; 

  size_t mlen = len;
  if (*pinPos>0) mlen = *pinPos;
  #ifdef DEBUGxxx
     Serial.print(F("#*****doSerialCommand: mlen=")); Serial.print(mlen); Serial.print(F(" data="));  for (size_t i=0; i<mlen; i++) Serial.print(data[i]);  debugPrint(F(" "),1,1);// Serial.println();
  #endif      
  
  
  if (xmemcmp(F("HELP"),data,mlen) ==0) res = scfHelp();
 
  else if (xmemcmp(F("HELLOHW"),data,mlen) ==0) {Serial.print(F("HELLO OPENHW v")); Serial.println(VERSION);}
  else if (xmemcmp(F("STATUS"),data,mlen) ==0) res = scfStatus();
  else if (xmemcmp(F("LOCK"),data,mlen) ==0) res = scfLock();
  else if (xmemcmp(F("UNLOCK"),data,mlen) ==0) res = scfUnlock();  

  else if (
     (xmemcmp(F("SETPRIVATEKEY("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSetPrivateKey(data+14,mlen-15);
  //getPublicKey
  else if (xmemcmp(F("GETPUBLICKEY"),data,mlen) ==0) res = doGetPublicKey();
  //getConfig
  else if (
     (xmemcmp(F("GETCONFIG("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doGetConfig(data+10,mlen-11);  
  //setButtonMode   
  else if (
     (xmemcmp(F("SETBUTTONMODE("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSetButtonMode(data+14,mlen-15);    
  else if (
     (xmemcmp(F("SIGNMESSAGE("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSignMessage(data+12,mlen-13);
  else if (
     (xmemcmp(F("SETPASSWORD("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSetPassword(data+12,mlen-13);            
  else if (
     (xmemcmp(F("ECDH("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doECDH(data+5,mlen-6);            
  else if (
     (xmemcmp(F("SETPIN("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSetPin(data+7,mlen-8,0);     
  else if (
     (xmemcmp(F("SETPIN2("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doSetPin(data+8,mlen-9,1);          
  else if (
     (xmemcmp(F("TEST("),data,mlen) ==0)
     &&(memcmp(data+mlen-1,")",1)==0)
     ) res = doTest(data+5,mlen-6);
  else return 0;

  if (res==1) {
    //there are 2 possible situations:
    // 1. pin requested
    // 2. pin failed. in this case cmdLen = 0
    
    if (cmdLen>0) {
      *pinPos = len; //request pin
      debugPrint(F("Asking for PIN."),1,1);
      Serial.println(F("PIN: please provide your pin"));
      setScreenStatus("ENTER PIN");
      screenShow("Please provide PIN");
    } else {
      debugPrint(F("PIN failed."),1,1);
      Serial.println(F("ERROR: incorrect pin"));
      setScreenStatus("LOCKED");
      screenShow("incorrect pin");      
    };
    return 0;
  };
 
  //#ifdef DEBUG
  //  Serial.print(F("#:::: doSerialCommand success. cmdLen=")); Serial.println(cmdLen);
  //#endif
  return 1;
 
}

void loop() {
  
  //read sCmd buffer
  //if not empty and more than 0.1 sec no data -> clean it
  //ibuf: cmdLen - next symb; pinPos - position pin started from; pinAwaitingFrom -millis; sCmdBFound sCmdLastRead 
  
  //if ((sCmd!="") && ((millis() - sCmdLastRead)>SERIAL_TIMEOUT)) sCmd = "";
 
  //cmd waiting timeout
  if ((pinPos==0) && (cmdLen>0) && ((millis() - sCmdLastRead)>SERIAL_TIMEOUT)) {
    //Serial.println(cmdLen); Serial.println(ibuf[0],HEX);
    cmdLen = 0;
    Serial.println(F("ERROR: command awaiting timeout"));
  }

  //pin timeout
  if ((pinPos>0) && (cmdLen>0) && ((millis() - sCmdLastRead)>PIN_TIMEOUT)) {
     //going to locked mode
     setScreenStatus("LOCKED");
     screenShow("pin entering timeout");
     cmdLen = 0;  
     Serial.println(F("ERROR: pin awaiting timeout"));
  }
  
  while (Serial.available() > 0) {
    //if (sCmd=="") sCmdBFound = 0;
    if (cmdLen==0) {
        sCmdBFound = 0; 
        //if (pinPos) debugPrint(F("#pinPos drop")); 
        pinPos = 0; 
        //LOCKED or other state must be indicated by the procedure who cleared cmdLen
    }; 

    if (cmdLen>=ibufLen) {
      while (Serial.available() > 0) Serial.read();
      Serial.println(F("ERROR: too long command"));
      cmdLen = 0;
      continue;
    };

    ibuf[cmdLen] = char(Serial.read()); cmdLen++;
    sCmdLastRead = millis();
    //just ignore \r
    if (ibuf[cmdLen-1]==13) {
      cmdLen--;
      continue;
    };
    
    sCmdBFound = sCmdBFound || (ibuf[cmdLen-1]=='(');
    if ((!sCmdBFound) && (ibuf[cmdLen-1]>='a')&&(ibuf[cmdLen-1]<='z') && (pinPos==0)) ibuf[cmdLen-1] = ibuf[cmdLen-1] + 'A' - 'a';

    

    //do the command if it is finished
    //sCmd.toUpperCase() don't need 
    if (pinPos==0) {
      //waiting for command mode
      //#ifdef DEBUG  
      //Serial.print(F("# ?loop0: pinPos=")); Serial.print(pinPos); Serial.print(F(" cmdLen=")); Serial.print(cmdLen);   Serial.print(F(" ibuf[cmdLen-1]=")); Serial.println((byte)ibuf[cmdLen-1]);  
      //#endif 
      if (doSerialCommand(ibuf,cmdLen,&pinPos)) cmdLen = 0; //it will set pinPos in case of we need pin. call it again when pin received
    } else {
      //pin will be ended by # or \n (but first \n will be ignored and must be cutted from the pin later)
      //#ifdef DEBUG  
      //Serial.print(F("# !loop1: pinPos=")); Serial.print(pinPos); Serial.print(F(" cmdLen=")); Serial.print(cmdLen);   Serial.print(F(" ibuf[cmdLen-1]=")); Serial.println((byte)ibuf[cmdLen-1]);  
      //#endif 
      if ((ibuf[cmdLen-1]=='#')|| ((ibuf[cmdLen-1]==10) &&(cmdLen!=(pinPos+1))) ) {
        doSerialCommand(ibuf,cmdLen-1,&pinPos); //do not send # or \n
        cmdLen = 0;
        #ifdef DEBUG
          //Serial.print(F("# PIN entered: \""));Serial.print();Serial.println(F("\""));
          Serial.println(F("#PIN char received"));
        #endif  
      };
    };

    //cut \n for end of command or end of PIN - but only if the pin is not empty
    //if ((cmdLen>0) && (ibuf[cmdLen-1]==10) && ((pinPos==0) || ((pinPos+1)!=cmdLen))) 
    if ((cmdLen>0) && (ibuf[cmdLen-1]==10) && (pinPos==0)) 
    #ifndef DEBUGxxxx
      cmdLen = 0; //the line was terminated but it was not correct
    #else  
      {
       Serial.print(F("#::::::::drop cmdLen #10end::: cmdLen="));Serial.print(cmdLen);Serial.print(F(" pinPos="));Serial.println(pinPos);
        cmdLen = 0;
      };
    #endif
    if (cmdLen==0) debugPrint(F("main loop cmdLen==0."),1,1);
  };
  
/*  
  const struct uECC_Curve_t * curve = uECC_secp160r1();
  uint8_t private1[21];
  uint8_t private2[21];
  
  uint8_t public1[40];
  uint8_t public2[40];
  
  uint8_t secret1[20];
  uint8_t secret2[20];
  
  unsigned long a = millis();
  uECC_make_key(public1, private1, curve);
  unsigned long b = millis();
  
  Serial.print("Made key 1 in "); Serial.println(b-a);
  a = millis();
  uECC_make_key(public2, private2, curve);
  b = millis();
  Serial.print("Made key 2 in "); Serial.println(b-a);

  a = millis();
  int r = uECC_shared_secret(public2, private1, secret1, curve);
  b = millis();
  Serial.print("Shared secret 1 in "); Serial.println(b-a);
  if (!r) {
    Serial.print("shared_secret() failed (1)\n");
    return;
  }

  a = millis();
  r = uECC_shared_secret(public1, private2, secret2, curve);
  b = millis();
  Serial.print("Shared secret 2 in "); Serial.println(b-a);
  if (!r) {
    Serial.print("shared_secret() failed (2)\n");
    return;
  }
    
  if (memcmp(secret1, secret2, 20) != 0) {
    Serial.print("Shared secrets are not identical!\n");
  } else {
    Serial.print("Shared secrets are identical\n");
  }



----------------------------------------------------------------------------
DELETE IT

If you want to change the defaults for uECC_CURVE and uECC_ASM, you must change them in your Makefile or similar so that uECC.c is compiled with the desired values (ie, compile uECC.c with -DuECC_CURVE=uECC_secp256r1 or whatever).

* When compiling for an ARM/Thumb-2 platform with `uECC_OPTIMIZATION_LEVEL` >= 3, you must use the `-fomit-frame-pointer` GCC option (this is enabled by default when compiling with `-O1` or higher).
* When compiling for AVR, you must have optimizations enabled (compile with `-O1` or higher).
* When building for Windows, you will need to link in the `advapi32.lib` system library.


For 8-bits AVR  I also have Arduino UNO. Constantly get same linker errors (try with -O1 or -Os flags). After many attempts to resolve this, I found by chance what change from
define uECC_SUPPORT_COMPRESSED_POINT 1
to
define uECC_SUPPORT_COMPRESSED_POINT 0
//in uECC.h resolve this issue. Maybe this helps somebody.

//Useful:
//http://wikihandbk.com/wiki/ESP8266:%D0%9F%D1%80%D0%BE%D1%88%D0%B8%D0%B2%D0%BA%D0%B8/Arduino/PROGMEM

https://russiansemiresearch.com

//stm
http://tqfp.org/stm32/zashita-ot-schityvaniya-proshivki-stm32.html

//arm
http://www.hobbytronics.co.uk/arduino-serial-buffer-size
http://www.engbedded.com/fusecalc/

http://www.getchip.net/posts/068-kak-pravilno-proshit-avr-fyuzy-fuse-bit/comment-page-3/
*/
}
