//========================================================================================FIRMWARE INFORMATION===================
/*


/*----------------------------------------------------------------------------------
HOW TO USE
1. For AVR (Arduino UNO, etc): change #define uECC_SUPPORT_COMPRESSED_POINT 1 to #define uECC_SUPPORT_COMPRESSED_POINT 0 in uECC.h
2. Set your board in FIRMWARE SETTINGS section 

----------------------------------------------------------------------------
CREDS
 * https://github.com/kamaljohnson/Arduino-SHA256/


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


*/
//========================================================================================FIRMWARE SETTINGS=====================
//Board: (if nothing changed then default settings will be applied. You can see details in the boards section)
//#define board_Heltec

//---------------------------------------linkage settings--------------------
//Use SHA256 (needs 450+ bytes RAM)
#define SHA256_SUPPORTED 1
//Debug mode:
#define DEBUG 1

//---------------------------------------defaults ---------------------------
#define SERIAL_TIMEOUT 100
#define PIN_TIMEOUT 100000 //100 sec
//Maximum PIN length
#define PIN_MAX_SIZE 8

//THIS IS THE END OF THE SETTINGS.
#define uECC_OPTIMIZATION_LEVEL 3
// #pragma GCC optimize ("-O0")
// #pragma GCC diagnostic warning "-fpermissive"
// #pragma message ("DO NOT FORGET TO REVIEW FIRMWARE SETTINGS")

//==============================================================================================================================


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
#ifdef board_Heltec
  #define OPTION_screen 1
#endif
//============================================================== helper section  ================
//unsigned char tBuf[80]; //universal buf. DO NOT use it if you are going to call any functions while using it
uint32_t crc32b(const unsigned char *message, size_t len) {
   uint32_t b, crc, mask;

   crc = 0xFFFFFFFF;
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
  bool mode1 = 0; //if true, we have entered correct PIN1. So, we are working with privkey1 until the pin0 will be entered. it is possible only if no pins entered for one week

  //user settings
  char uname[24] = "";
  bool askPinEveryTime = false; //if we need ask pin for every privKey0 access
  bool lockPinAfterMs = 0; //if we need to forget pin 

  byte buttonMode = 1; //0: don't use button 1: only for pin unlock 2: for each private key access

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
  //String commandWaiting; //the command waiting now. "" if there is no command. Will be executed after any event like pin enter or button pressed
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
    Serial.print(F("Memory: free:"));Serial.print(getFreeMemory()); 
    
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

size_t getProgmemStr(const PROGMEM char * strname, char result[]) {
  for (byte k = 0; k <strlen_P(strname) ; k++) result[k] = pgm_read_byte_near(strname + k);
  return strlen_P(strname);
};

#ifdef DEBUG
void debugPrint(const __FlashStringHelper * str, bool newline=true, bool printMem=false) {
  Serial.print(str);Serial.print(F(" "));
  if (printMem) printMemory(newline); else if (newline) Serial.println();
};
#else
  #define debugPrint(x)
#endif
//===================================================================================================================================================
//---------------------------decode options-----------------------------------------------------
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
Settings *ptrSettings = 0;


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
  for (int i=0; i<sizeof(Settings); i++) ((byte*)settings)[i] = EEPROM.read(i);
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
  onUpdateSettings(settings);
};

void writeSettings(Settings *settings);

void writeSettings(Settings *settings)
{
  settings->crc = crc32b(((byte*)settings),sizeof(Settings) - sizeof(settings->crc));
  for (int i=0; i<sizeof(Settings); i++) 
    if (EEPROM.read(i)!=((byte*)settings)[i])  
      EEPROM.write(i,((byte*)settings)[i]);  
  debugPrint(F("Settings recorded."),1,1);
};

void onUpdateSettings(Settings *settings)
{
  //onUpdateSettings(&settings);

  if (!memnchr(settings->privKey0,0,sizeof(settings->privKey0))) {
    setScreenStatus("NO DATA");
  } else {
    if (settings->pin0[0]==0) //PIN not set
       setScreenStatus("PIN NOT SET");
    else 
      if (
          (strcmp(settings->pin0,state.pin)==0) 
          ||
          (strcmp(settings->pin1,state.pin)==0)
         )
         setScreenStatus("UNLOCKED");   
      else   
         setScreenStatus("LOCKED");   
  };
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
  Serial.begin(115200);
  while (!Serial);

  memset(&state,0,sizeof(State));
  
  Serial.print(F("openHW v0.1   Type 'help' for help\n"));
  uECC_set_rng(&RNG);

  screenInit();

  //load settings
  //Serial.print(F("BEFORE ")); printMemory();
  initialSettingsLoading();
  //Serial.print(F("AFFTER ")); printMemory();
  
  pinPos=0; cmdLen=0; 

  #ifdef DEBUG
    Serial.print(F("setup() done: "));printMemory();
  #endif  
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
  Serial.print(F("openHW v0.1\n"));
  Serial.print(F("help : show this text\n"));
  Serial.print(F("setPrivateKey(<private1>[,<private2>]) : sets private key(s). Data in code58check form.\n"));
  Serial.print(F("  it will erase all privateKeys and set all pins to "".\n"));
  Serial.print(F("getPublicKey : gets current public key\n"));
  Serial.print(F("signMessage(<message>) : signs the message. Message in hex. Result in hex\n"));
  //Serial.print(F(" (*) signTX(TXdata>,<nOut>) : signs transaction input nOut. Message in hex. Result in hex\n"));
  Serial.print(F("signTXDigest(<TXdata>) : signs transaction digest. Data in hex. Result in hex\n"));
  Serial.print(F("setPin(<pin>) : sets current pin. Pin is text (don't use \")\" in pin)\n"));
  Serial.print(F("setPin2(<pin>) : sets plausible denial pin. Pin is text (don't use \")\" in pin)\n"));
  Serial.print(F("  WARNING: setting any of the pins in P.D. mode will make P.D. key main and will erase main private key."));  
  Serial.print(F("setPinMode(<N>) : sets pin request mode. 0 for ask once; 1 for asking every time\n"));  
  Serial.print(F("setPinLockDelay(<Milliseconds>) : sets time for pin lock after enter\n"));
  Serial.print(F("setButtonMode(<N>) : sets button mode. 0: button is not used; 1: use only for pin confirmatuion; 2: use for Private Key access confirmation\n"));
  Serial.print(F("getConfig([parname]) : Returns configuration. Call getConfig() for all params with names\n"));        
 
};

//=========================================================== cmd functions ================================
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
    Serial.print(F("data="));  for (int i=0; i<len; i++) {Serial.print(data[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));
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
      Serial.print(F("Ready to calc SHA256. "));printMemory();
      if (!checkMemory(350)) return -1;
      SHA256(res,34,hash);
      SHA256(hash,32,hash);
      Serial.print(F("SHA256 calculated. "));printMemory();
      Serial.print(F("hash=")); for (int i=0; i<32; i++) {Serial.print(hash[i],HEX); Serial.print(F(" "));};  Serial.println(F(" "));      
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

void savePrivateKey(const uint8_t *data,byte n){
  //safe key #n
  //saving key 0 removes key 1
  Settings settings;
  readSettings(&settings);
  if (n==0) {
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

byte doSetPrivateKey(const char *data, size_t len) {
  //return 1 for pin request
  //if (!unlocked()) return 1;  ->We don't need it to set pins
  #ifdef DEBUG
    Serial.print(F("Set private key: ")); for (int i=0; i<len; i++) Serial.print((char)data[i]); Serial.println(); printMemory();
  #endif  

  //<data>[,<data2>]
  void *cpa = memchr(data,',',len);

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
        Serial.print(F("PUBKEY: ")); for (int i=0; i<64; i++) Serial.print(pub[i],HEX); Serial.println();
      #endif 
    } else {
      Serial.println(F("ERROR: Private key assigned: no public key"));
      screenShow("ERROR setting private key 1");
      return 0;
    };  
  };
  
  //PA key
 if (cpa) if (32==checkAndDecodePrivateKey(cpa+1,len+ data - (char*)cpa -1, res)) {
   debugPrint(F("PA key found, ready to decode."),1,1);
   if (uECC_compute_public_key(res,pub,curve)) {
      savePrivateKey(res,1);
      Serial.println(F("OK: Both keys were assigned"));
      screenShow("OK: Both keys were assigned");
      #ifdef DEBUG
        Serial.print(F("PUBKEY2: ")); for (int i=0; i<64; i++) Serial.print(pub[i],HEX); Serial.println();
      #endif 
    } else {
      Serial.println(F("ERROR: Only first key was assigned"));
      screenShow("ERROR Only first key was assigned");
      return 0;
    };     
 }
};

bool unlocked() 
//returns if the 
// INCREASES ATTEMPT COUNTER AND MAKES DELAY
{ 
  Settings settings;
  readSettings(&settings); 
  if (settings.pin0[0]==0) return true;
  if (state.pin[0]==0) return false;
  
  //byte attCounter = 0; //attempt counter. Zero it if known pin
  //bool mode1 = 0; //if true, we have entered correct PIN1. So, we are working with privkey1 until the pin0 will be entered. it is possible only if no pins entered for one week
  //unsigned long lastMode1Millis = 0; //last millis attempt for mode 1
   
  
  if (strcmp(settings.pin1,state.pin)==0) {
    //pin1 used
    settings.mode1 = true;
    settings.attCounter = 0;
    writeSettings(&settings);
    return true;
  }

  //5 days delay for come back to the correct pin
  if (settings.mode1 && ((millis()-state.lastMode1Millis) <(1000*432000))) {
    //too early to try
    state.lastMode1Millis = millis();
    settings.attCounter += 1;
    writeSettings(&settings);
    delayMicroseconds((1UL <<(settings.attCounter-1) )*100);
    return false;
  };

  //perhaps it is pin0?
  if (strcmp(settings.pin0,state.pin)==0) {
    //pin0 used
    settings.mode1 = false;
    state.lastMode1Millis = 0;
    settings.attCounter = 0;
    writeSettings(&settings);
    return true;
  };

  //wrong password
  settings.attCounter += 1;
  writeSettings(&settings);
  //delayMicroseconds(100*(unsigned long)((unsigned long)1 <<(settings.attCounter-1) ));
  delayMicroseconds((1UL <<(settings.attCounter-1) )*100);
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

byte doGetPublicKey() {
  debugPrint(F("getPublicKey called."),1,1);
  //return 1 for pin request
  if (!unlocked()) return 1; 
  uint8_t pub[64];
  if (getPublicKey(pub)) {
    hex8Serial(pub,64); Serial.println();
    screenShow("Public key requested");    
  } else {
    Serial.println(F("ERROR: can't access public key"));
    screenShow("ERROR: can't access public key");    
  };
};

byte doGetConfig(const char *data, size_t len){
  #ifndef DEBUG
    if (!unlocked()) return 1; 
  #endif
  Settings settings;
  readSettings(&settings);

  if (len>0) {
    Serial.println(F("ERROR: Sorry, not supported yet"));
    return 0;
  };
    
  Serial.println(F("OK: openHW configuration:"));


  #ifdef DEBUG
   Serial.print(F("privKey0:")); hex8Serial(settings.privKey0,32); Serial.println();
   Serial.print(F("privKey1:")); hex8Serial(settings.privKey1,32); Serial.println();
   Serial.print(F("pin0:")); Serial.println(settings.pin0);
   Serial.print(F("pin1:")); Serial.println(settings.pin1);
   Serial.print(F("attCounter:")); Serial.println(settings.attCounter);
   Serial.print(F("mode1:")); Serial.println(settings.mode1);
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
    Serial.print(F("CRC:")); Serial.println(settings.crc,HEX);
  #endif
  
  return 0;
};

byte doSignMessage(const char *data, size_t len){
  debugPrint(F("doSignMessage called."),1,1);
  //return 1 for pin request
  if (!unlocked()) return 1; 

  //calc hash
  uint8_t hash[len/2];
  bufFromHex(data, len, hash);

  #ifdef DEBUG
    Serial.print(F("Signing ")); hex8Serial(hash,len/2); Serial.println();
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
    if (
      uECC_sign(
        privkey,   //private key
        hash,  //hash
        len/2, //hash size
        sign,
        curve
      )  
    ) {
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

//=========================================================== main =========================================

bool doSerialCommand(const char *data, size_t len, size_t *pinPos) {
  //we must set pinPos = Len if we need pin
  if (*pinPos>0) {
    //we have called with a pin
    byte pinSize = len - *pinPos;
    if (pinSize>PIN_MAX_SIZE) pinSize = PIN_MAX_SIZE;
    memcpy(state.pin,data+*pinPos,pinSize);
    state.pin[pinSize] = 0;
  };

  //if (sCmd=="HELP") scfHelp();
  //else if ((sCmd.startsWith("SETPRIVATEKEY("))&&(sCmd.endsWith(")"))) doSetPrivateKey(sCmd.substring(14,sCmd.length()-1));
  //else if ((sCmd.startsWith("TEST("))&&(sCmd.endsWith(")"))) doTest(sCmd.substring(5,sCmd.length()-1));
  size_t res = 0; 
  if (memcmp(data,"HELP",4) ==0) res = scfHelp();
  else if (
     (memcmp(data,"SETPRIVATEKEY(",14) ==0)
     &&(memcmp(data+len-1,")",1)==0)
     ) res = doSetPrivateKey(data+14,len-15);
  //getPublicKey
  else if (memcmp(data,"GETPUBLICKEY",12) ==0) res = doGetPublicKey();
  //getConfig
  else if (
     (memcmp(data,"GETCONFIG(",10) ==0)
     &&(memcmp(data+len-1,")",1)==0)
     ) res = doGetConfig(data+10,len-11);  
  else if (
     (memcmp(data,"SIGNMESSAGE(",12) ==0)
     &&(memcmp(data+len-1,")",1)==0)
     ) res = doSignMessage(data+12,len-13);       
  else if (
     (memcmp(data,"TEST(",5) ==0)
     &&(memcmp(data+len-1,")",1)==0)
     ) res = doTest(data+5,len-6);
  else return 0;

  if (res==1) *pinPos = len; //request pin
  return 1;
}

void loop() {
  
  //read sCmd buffer
  //if not empty and more than 0.1 sec no data -> clean it
  //ibuf: cmdLen - next symb; pinPos - position pin started from; pinAwaitingFrom -millis; sCmdBFound sCmdLastRead 
  
  //if ((sCmd!="") && ((millis() - sCmdLastRead)>SERIAL_TIMEOUT)) sCmd = "";
 
  //cmd waiting timeout
  if ((pinPos==0) && (cmdLen>0) && ((millis() - sCmdLastRead)>SERIAL_TIMEOUT)) {
    Serial.println(cmdLen); Serial.println(ibuf[0],HEX);
    cmdLen = 0;
    Serial.println(F("ERROR: command awaiting timeout"));
  }

  //pin timeout
  if ((pinPos>0) && (cmdLen>0) && ((millis() - sCmdLastRead)>PIN_TIMEOUT)) {
     cmdLen = 0;  
     Serial.println(F("ERROR: pin awaiting timeout"));
  }
  
  while (Serial.available() > 0) {
    //if (sCmd=="") sCmdBFound = 0;
    if (cmdLen==0) {sCmdBFound = 0; pinPos = 0;}; 

    if (cmdLen>=ibufLen) {
      while (Serial.available() > 0) Serial.read();
      Serial.println(F("ERROR: too long command"));
      cmdLen = 0;
      continue;
    };

    ibuf[cmdLen] = char(Serial.read()); cmdLen++;
    sCmdBFound = sCmdBFound || (ibuf[cmdLen-1]=='(');
    if ((!sCmdBFound) && (ibuf[cmdLen-1]>='a')&&(ibuf[cmdLen-1]<='z') && (pinPos==0)) ibuf[cmdLen-1] = ibuf[cmdLen-1] + 'A' - 'a';

    sCmdLastRead = millis();

    //do the command if it is finished
    //sCmd.toUpperCase() don't need 
    if (pinPos==0) {
      //waiting for command mode
      if (doSerialCommand(ibuf,cmdLen,&pinPos)) cmdLen = 0; //it will set pinPos in case of we need pin. call it again when pin received
    } else {
      //pin will be ended by #
      if ((ibuf[cmdLen-1]=='#')||(ibuf[cmdLen-1]==10)) {
        doSerialCommand(ibuf,cmdLen,&pinPos);
        cmdLen = 0;
      };
    };
    if ((cmdLen>0) && (ibuf[cmdLen-1]==10)) cmdLen = 0; //the line was terminated but it was not correct
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
*/
}
