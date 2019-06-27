//screen:
#if defined(board_Heltec_WiFi_Kit_8) || defined(board_Heltec_WiFi_Kit_32)

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


#define u8g_logo_width 24
#define u8g_logo_height 20
static unsigned char u8g_logo_bits[] = {
  0xC0, 0xFF, 0x03, 0xE0, 0xFF, 0x07, 0x60, 0x00, 0x06, 0x30, 0x00, 0x0C, 
  0x18, 0x00, 0x18, 0x98, 0xFF, 0x19, 0x8C, 0xFF, 0x31, 0x0C, 0x98, 0x31, 
  0x06, 0x98, 0x61, 0x83, 0x9F, 0xC1, 0x83, 0x9F, 0xC1, 0x06, 0x80, 0x61, 
  0x0C, 0x80, 0x31, 0x8C, 0xFF, 0x31, 0x98, 0xFF, 0x19, 0x18, 0x00, 0x18, 
  0x30, 0x00, 0x0C, 0x60, 0x00, 0x06, 0xE0, 0xFF, 0x07, 0xC0, 0xFF, 0x03, 
   };


// the OLED used
//https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#ssd1306-128x64_noname

//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#ifdef board_Heltec_WiFi_Kit_32
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#elif defined(board_Heltec_WiFi_Kit_8)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4); 
#endif

char screenStatus[24] = "openHW";

//void setScreenStatus(const char str[]);
void setScreenStatus(const char str[]) {
  for (int i=0;i<23;i++) {
    screenStatus[i] = str[i];
    if (!str[i]) return;
  };
  screenStatus[24] = 0;
};  


void screenInit()
{
  u8g2.begin();
  //https://github.com/olikraus/u8g2/wiki/fntgrpunifont
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  //u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_unifont_t_cyrillic);
  //u8g2.drawUTF8(0, 48, "Тест 2. Serial/wifi");
  //u8g2.sendBuffer();
  //u8g2.drawUTF8(0, 48, "Проверка AbC");
  //u8g2.setFont(u8g2_font_unifont_t_chinese1);
}

void screenShow(const char *str)
{
  u8g2.clearBuffer();

  //fonts:
  //https://github.com/olikraus/u8g2/wiki/fntlistall#u8g2-font-names

  
  u8g2.setFont(u8g2_font_unifont_t_cyrillic);
  
  //draw logo 
  u8g2.drawXBM( 0, 0, u8g_logo_width, u8g_logo_height, u8g_logo_bits);

  //draw status
  //we have 100 px, from 26 to 126
  //draw it in the center
  // y= 6; sym w = 8
  int l =  strlen(screenStatus);
  int x = 26;
  if (l<13)  x = 26 + (13 -l)*4;
  
  u8g2.drawUTF8(x, 16 , screenStatus); //4 + 12

  //draw data
  //different screens - different pars
  #ifdef board_Heltec_WiFi_Kit_32
    //128X64
    #define mfont_h 13 //font height with space
    #define mfont_w 8 //font width with space    
    #define mscr_h 64 //screen height
    #define mscr_w 128

    int y = 36; x = 0;    
  #elif defined(board_Heltec_WiFi_Kit_8)

/*    
     TWO TEXT LINES
     
    //128X32
    u8g2.setFont(u8g2_font_4x6_tr);
    #define mfont_h 6 //font height with space
    #define mfont_w 4 //font width with space
    #define mscr_h 32//screen height
    #define mscr_w 128
*/
    
    //128X32
    u8g2.setFont(u8g2_font_t0_11_tf);
    #define mfont_h 9 //font height with space
    #define mfont_w 6 //font width with space
    #define mscr_h 32//screen height
    #define mscr_w 128
    

    int y = 22+mfont_h; x = 0;
  #endif


  
  //first line is 24, step 12
  //go to next line on \n or limit 128/8=16 symbols
  l =  strlen(str);
  if (l>0)
    for (int i=0; true; i++) {
      if (str[i]==10) {y = y + mfont_h; x = 0;}
      else {
        char tmp[2] = "x"; tmp[0] = str[i];
        u8g2.drawStr(x, y, tmp);
        x += mfont_w;
        if (x>(mscr_w-mfont_w)) {y = y + mfont_h; x = 0;};
      };
      if ((y>mscr_h) || (i>=l)) break;
    };
    
  
  u8g2.sendBuffer();
}

#endif  //board_Heltec
