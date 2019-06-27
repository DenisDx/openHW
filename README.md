openHW project 0.3
=================
Open hardware wallet. Supports Arduino AVR / ESP/ STM chips (atmega328 included), Emercoin / Bitcoin / Ethereum etc compatible

## openHW

##How to use


##WARNING
#1. No chip guarantees the security of your data when it is stolen.
The system uses a PIN code that is strong enough to protect the data (if the instructions are followed) in case the chip is not physically opened.
However, any chip can be hacked with special equipment and the data can be read. 
All existing devices on the market are exposed to this vulnerability. The only difference is the complexity of their hacking.

#2.Chips that are most vulnerable to physical hacking
Protecting the memory of some chips at the moment can be easily hacked by physically exposing the chip 
(costs around #1000 with 50% chance of success)
Storing your private keys on these chips is comparable in security to storing paper with a password in a locked box.
If this chip is stolen, all assets should be immediately transferred to another private key and the old one should not be used.
Below is a list of unsafe chips:
Atmel Atmega***
STM32F1xx
STMicroelectronics STR7xxxxx

Chips that are not known to be unsafe at the time of writing this document:
ESP32
ESP8256


##How to compile
#1. Intall Ardino board framework
  https://www.arduino.cc/en/Main/Software
#2. Install additional libraries for you board:
  Go to File > Preferences : Enter library json URL into the “Additional Board Manager URLs” and click "ok"
  * arduino (or compatible board): do not need to intall additional libraries 
  * esp8266: http://arduino.esp8266.com/stable/package_esp8266com_index.json 
  * esp32 (Heltek web kit) : https://dl.espressif.com/dl/package_esp32_index.json
  
  
  if you wish to work with different boadrs, you can set mutily urls like 
    https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
  
#3. Install additional libraries for the script
   install micro-ecc library: go to Sketch=>Include Library=>Manage Libraries; find uECC library and install it
   For arm arduino (for example, arduino UNO) you must modify intalled micro-ecc library: 
     change "define uECC_SUPPORT_COMPRESSED_POINT 1" to "define uECC_SUPPORT_COMPRESSED_POINT 0" in uECC.c
   for STM or ESP boadrs it is not needed to make any changes (but you can do it if you wish)

#4. Configure your board.
   open the project.  Open "openHW.ino" file. go to the "FIRMWARE SETTINGS" section (it is on the beginnig of the file)
   and uncomment the row with your board 
   (for example, if you board is Heltec ESP32 you should delete "//" in the line "//#define board_Heltec")
   if you can't find your board in the list, do not uncomment any lines. 
   
#5. Connect you board
   for some boards you need some additional procedures before flashing

#6. Setup your board
	Open Arduino gui
	Select your board in the "tools" section. Select corrrect COM port
	
#7. Flash the firmware	

#8. Set board fuses to protect it from JTAG reading / for EPPROM erasing
This is a very important part of the process. If you don't do it, it will be easy for intruders to get your private keys from the device.
If the process fails at this stage, you can brick your device. Be careful.

#9. Use with pleasure!

##Dependencies
arduino libraries
micro ecc library https://github.com/kmackay/micro-ecc (you need install it into your system)
for sha256 hashing used https://github.com/kamaljohnson/Arduino-SHA256/ (with modifications) (included) 

##License

This "Software" is Licensed Under  **`MIT License (MIT)`** .

##Version history

0.3
minor changes
arduino memory problem solved (-Og added... this is only way)

0.2
Fully functional (?). 
TODO: 
  - ECDH
  - JTAG disable / EEPROM erase manual
  - Password support
  - setPinMode, setPinLockDelay, setButtonMode support

0.1 
Initial version