#include <Keypad.h>
#include "U8glib.h"
#include <SoftwareSerial.h>
#include "pitches.h"

SoftwareSerial RFID(12, 14);

const byte ROWS = 4; 
const byte COLS = 3;

String inputString = "";
boolean stringComplete = false;

String lastKey="";
int lastKeyPass=0;

const char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

const int melConnect[] = {
  NOTE_G4, NOTE_B4, NOTE_D5
};

const int melDisconnect[] = {
  NOTE_D5, NOTE_B4, NOTE_G4
};

const int noteDurations[] = {
  8, 8, 8
};

const byte rowPins[ROWS] = {53, 51, 49, 47}; 
const byte colPins[COLS] = {45, 43, 41}; 
const int buzzer=3;
const int lr=5;
const int lg=4;
const int lb=2;

char customKey;
char key;
char displayValue[4] = {'0','0','0','0'};
bool redraw=true;

int screen=-1;
int timeComp=0;
float value=0;
int ledDelay=0;
bool badgeOn = false;
float wallet = 0;
bool timeCompNeedReset = false;

const uint8_t card[] U8G_PROGMEM = {
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x01, 0xE0, 0x00, 
  0x06, 0x00, 0x00,
  0x08, 0x00, 0x00,
  0x10, 0xE0, 0x00,
  0x21, 0x00, 0x00, 
  0x22, 0x00, 0x00,
  0x44, 0xE0, 0x00,
  0x49, 0x80, 0x00,
  0x49, 0x3F, 0xF8, 
  0x49, 0x40, 0x04,
  0x00, 0x40, 0x04,
  0x00, 0x40, 0x04,
  0x00, 0x43, 0x84, 
  0x00, 0x46, 0x04,
  0x00, 0x44, 0x04,
  0x00, 0x44, 0x44,
  0x00, 0x47, 0xC4, 
  0x00, 0x43, 0x04,
  0x00, 0x42, 0x04,
  0x00, 0x42, 0x04,
  0x00, 0x42, 0x14, 
  0x00, 0x43, 0x34,
  0x00, 0x41, 0xE4,
  0x00, 0x40, 0x04,
  0x00, 0x40, 0x04, 
  0x00, 0x3F, 0xF8,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 
  };

const uint8_t cup[] U8G_PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x06, 0x10, 0x00,
0x00, 0x06, 0x10, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x1C, 0x60, 0x00, 0x00, 0x18, 0x60, 0x00,
0x00, 0x18, 0x40, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFC, 0x07, 0xFF, 0xFF, 0xFE, 0x06, 0x00, 0x00, 0xC3,
0x06, 0x00, 0x00, 0xC3, 0x06, 0x00, 0x00, 0xC3, 0x06, 0x00, 0x00, 0xC6, 0x06, 0x00, 0x00, 0xFE,
0x02, 0x00, 0x00, 0xF8, 0x03, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x01, 0x80, 0x01, 0x80, 0x03, 0x80,
0x01, 0xC0, 0x07, 0x00, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 0x1C, 0x00, 0x00, 0x30,
0x0C, 0x00, 0x00, 0x60, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t check[] U8G_PROGMEM = {
0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00, 0x03, 0xFC,
0x00, 0x00, 0x03, 0xF8, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x1F, 0xE0,
0x00, 0x00, 0x1F, 0xC0, 0x00, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x7F, 0x80, 0x30, 0x00, 0xFF, 0x80,
0xF8, 0x00, 0xFF, 0x00, 0xFE, 0x01, 0xFE, 0x00, 0xFF, 0x03, 0xFE, 0x00, 0x7F, 0x87, 0xFC, 0x00,
0x7F, 0xE7, 0xF8, 0x00, 0x3F, 0xFF, 0xF8, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0xE0, 0x00,
0x07, 0xFF, 0xE0, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x01, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0x80, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00
};

const uint8_t cross[] U8G_PROGMEM = {
0xC0, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x07, 0x70, 0x00, 0x00, 0x0E, 0x38, 0x00, 0x00, 0x1C,
0x1C, 0x00, 0x00, 0x38, 0x0E, 0x00, 0x00, 0x70, 0x07, 0x00, 0x00, 0xE0, 0x03, 0x80, 0x01, 0xC0,
0x01, 0xC0, 0x03, 0x80, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x70, 0x0E, 0x00, 0x00, 0x38, 0x1C, 0x00,
0x00, 0x1C, 0x38, 0x00, 0x00, 0x0E, 0x70, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00,
0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x0E, 0x70, 0x00, 0x00, 0x1C, 0x38, 0x00,
0x00, 0x38, 0x1C, 0x00, 0x00, 0x70, 0x0E, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x01, 0xC0, 0x03, 0x80,
0x03, 0x80, 0x01, 0xC0, 0x07, 0x00, 0x00, 0xE0, 0x0E, 0x00, 0x00, 0x70, 0x1C, 0x00, 0x00, 0x38,
0x38, 0x00, 0x00, 0x1C, 0x70, 0x00, 0x00, 0x0E, 0xE0, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x03
};

const uint8_t usb[] U8G_PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x07, 0xF8, 0x00, 0x00, 0x0C, 0x38, 0x00,
0x00, 0x18, 0x00, 0x00, 0x38, 0x30, 0x00, 0x00, 0x7C, 0x60, 0x00, 0x08, 0x7F, 0xFF, 0xFF, 0xFE,
0x7C, 0x02, 0x00, 0x08, 0x38, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x43, 0x80,
0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
U8GLIB_ST7920_128X64 u8g(13, 11, 10, U8G_PIN_NONE);

void draw(void) {
  if(screen==-1){                             //Ecran bloqué (terminal déconnecté)
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(9,12);
    u8g.print("Systeme ERIA");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(15,61);
    u8g.print("Connectez lecteur");
    u8g.drawBitmapP( 48, 25, 4, 16, usb);
  }
  if(screen==0){                              //Ecran d'accueil (Bonjour + tasse)
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30,12);
    u8g.print("Bonjour !");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(13,61);
    u8g.print("SAISISSEZ MONTANT");
    u8g.drawBitmapP( 48, 18, 4, 32, cup);
  }
  if(screen==1){                              //Ecran de saisie de montant
    u8g.setFont(u8g_font_fub17);
    //u8g.print(key);
    for(int i =0;i<2;i++){
      u8g.setPrintPos(12+13*i, 35);
      u8g.print(displayValue[i]);
    }
    u8g.setPrintPos(38, 35);
    u8g.print(',');
    for(int i =2;i<4;i++){
      u8g.setPrintPos(44+13*(i-2), 35);
      u8g.print(displayValue[i]);
    }
    u8g.setPrintPos(75, 35);
    u8g.setFont(u8g_font_fub14);
    u8g.print("EUR");
  }
  if(screen==2){                               //Ecran d'attente de passage badge pour paiement
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30,13);
    u8g.print(value);
    u8g.setPrintPos(75, 13);
    u8g.print("EUR");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(19,58);
    u8g.print("PRESENTEZ BADGE");
    u8g.drawBitmapP( 52, 15, 3, 32, card);
  }
  if(screen==3){                               //Paiement accepté
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30,13);
    u8g.print(value);
    u8g.setPrintPos(75, 13);
    u8g.print("EUR");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(16,58);
    u8g.print("PAIEMENT ACCEPTE");
    u8g.drawBitmapP( 48, 15, 4, 32, check);
  }
  if(screen==4){
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30,13);
    u8g.print(value);
    u8g.setPrintPos(75, 13);
    u8g.print("EUR");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(19,58);
    u8g.print("PAIEMENT REFUSE");
    u8g.drawBitmapP( 48, 15, 4, 32, cross);
  }
  if(screen==5){
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(8,30);
    u8g.print("TRANSACTION ANNULEE");
  }
  if(screen==6){
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30,13);
    u8g.print(wallet);
    u8g.setPrintPos(75, 13);
    u8g.print("EUR");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(31,58);
    u8g.print("VOTRE SOLDE");
    u8g.drawBitmapP( 48, 15, 3, 32, card);
  }
  if(screen==7){
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(19,58);
    u8g.print("BADGE INCONNU");
    u8g.drawBitmapP( 48, 15, 4, 32, cross);
  }
  if(screen==8){                               //Ecran d'attente de passage badge pour paiement
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(15, 13);
    u8g.print("PRESENTEZ");
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(19,58);
    u8g.print("NOUVEAU BADGE");
    u8g.drawBitmapP( 52, 15, 3, 32, card);
  }
  if(screen==9){                               //Paiement accepté
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(16,58);
    u8g.print("BADGE AJOUTE");
    u8g.drawBitmapP( 48, 15, 4, 32, check);
  }
  if(screen==10){
    u8g.setFont(u8g_font_profont11);
    u8g.setPrintPos(19,58);
    u8g.print("CARTE EXISTANTE");
    u8g.drawBitmapP( 48, 15, 4, 32, cross);
  }
}

void setup(){
  RFID.begin(9600);
  Serial.begin(9600);
  //u8g.setRot180();
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  pinMode(buzzer, OUTPUT);
  pinMode(lr, OUTPUT);
  pinMode(lg, OUTPUT);
  pinMode(lb, OUTPUT);
  pinMode(12, INPUT);
}
  
void loop(){
  serialEvent();
  if (stringComplete) {
    if(inputString=="11\n"){
      payment(1);
    }
    if(inputString=="12\n"){
      payment(0);
    }
    if(inputString=="13\n"){
      payment(-1);
    }
    if(inputString.charAt(0)=='1' && inputString.charAt(1)=='4' && inputString.charAt(2)=='&'){
      wallet=(int(inputString.charAt(3))-48)*10+(int(inputString.charAt(4))-48)*1+(int(inputString.charAt(6))-48)*0.1+(int(inputString.charAt(7))-48)*0.01;
      screen=6;
      timeComp=0;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
    }
    if(inputString=="15\n"){
      screen=8;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
    }
    if(inputString=="16\n"){
      screen=7;
      digitalWrite(lr, HIGH);
      digitalWrite(lg, HIGH);
      tone(3, NOTE_G3, 1000);
      timeComp=0;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
    }
    if(inputString=="21\n"){
      addCard(1);
    }
    if(inputString=="22\n"){
      addCard(0);
    }
    if(inputString=="42\n"){
      screen=0;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
      for (int thisNote = 0; thisNote < 3; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(3, melConnect[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1;
        delay(pauseBetweenNotes);
        noTone(3);
      }
    }
    if(inputString=="20\n"){
      screen=0;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
    }
    if(inputString=="43\n"){
      screen=-1;
      u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
      for (int thisNote = 0; thisNote < 3; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(3, melDisconnect[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1;
        delay(pauseBetweenNotes);
        noTone(3);
      }
    }
    inputString = "";
    stringComplete = false;
  }
  
  customKey = customKeypad.getKey();
  
  if (customKey){
    if(screen!=-1 && screen!=2 && screen !=3 && screen!=4 && screen!=8){
      tone(3, NOTE_C6, 8);
      //Serial.println(customKey);
    }
    if(screen==0){
      if(customKey!='#' && customKey!='0' && screen== 0){
        screen=1;
        redraw=true;
      }
    }
    if(screen==1){
      if(customKey=='#'){
        screen=2;
        value=int(displayValue[0]-48)*10.0+int(displayValue[1]-48)*1.0+int(displayValue[2]-48)*0.1+int(displayValue[3]-48)*0.01;
        redraw=true;
      }
      else if(customKey=='*'){
        timeComp=0;
        screen=5;
        redraw=true;
      }
      else{
        addDecimal(customKey);
        redraw=true;
      }
    }
    if(screen==2){
      if(customKey=='*'){
        timeComp=0;
        screen=5;
        redraw=true;
      }
    }
  }
  if(screen==-1){
    digitalWrite(lb, HIGH);
  }
  else{
    digitalWrite(lb, LOW);
  }
  if(screen==1 || screen==0){
    if(ledDelay>=100){
      digitalWrite(lb, HIGH);
      delay(30);
      digitalWrite(lb, LOW);
      ledDelay=0;
    }
  }
  if(screen==0){
    readRFID();
  }
  if(screen==2){
    if(ledDelay>=15){
      digitalWrite(lb, HIGH);
      delay(75);
      digitalWrite(lb, LOW);
      ledDelay=0;
    }
    readRFIDpay(value);
  }
  if(screen==5){
    if(timeComp>=150){
      screen=0;
      displayValue[0]='0';
      displayValue[1]='0';
      displayValue[2]='0';
      displayValue[3]='0';
      timeComp=0;
      value=0;
      redraw=true;
    }
  }
  if(screen==6 || screen==7){
    if(timeComp>=300){
      screen=0;
      displayValue[0]='0';
      displayValue[1]='0';
      displayValue[2]='0';
      displayValue[3]='0';
      timeComp=0;
      value=0;
      digitalWrite(lr, LOW);
      digitalWrite(lg, LOW);
      redraw=true;
      lastKey="";
    }
  }
  if(screen==8){
    readRFID();
  }
  if(redraw){
    u8g.firstPage();  
    do {
      draw();
    } while( u8g.nextPage() );
    redraw=false;
  }
  timeComp=timeComp+1;
  ledDelay=ledDelay+1;
  if(timeCompNeedReset == true){
    if(timeComp>=100){
      digitalWrite(buzzer, LOW);
      if(timeComp>=300){
        digitalWrite(lr, LOW);
        digitalWrite(lg, LOW);
        digitalWrite(lb, LOW);
        screen=0;
        displayValue[0]='0';
        displayValue[1]='0';
        displayValue[2]='0';
        displayValue[3]='0';
        timeComp=0;
        value=0;
        redraw=true;
        timeCompNeedReset=false;
        lastKey="";
      }
    }
  }
  delay(10);
}

void addDecimal(char added){
  displayValue[0]=displayValue[1];
  displayValue[1]=displayValue[2];
  displayValue[2]=displayValue[3];
  displayValue[3]=added;
}

void payment(int state){
  if(state ==1){
    screen=3;
    redraw=true;
    digitalWrite(lg, HIGH);
    tone(3, NOTE_G6, 1000);
    timeComp=0;
    timeCompNeedReset = true;
  }
  else if(state==0){
    screen=4;
    redraw=true;
    digitalWrite(lr, HIGH);
    tone(3, NOTE_G3, 1000);
    timeComp=0;
    timeCompNeedReset = true;
  }
  else{
    screen=7;
    redraw=true;
  }
}

void addCard(int state){
  if(state ==1){
    screen=9;
    redraw=true;
    digitalWrite(lg, HIGH);
    tone(3, NOTE_G6, 1000);
    timeComp=0;
    timeCompNeedReset = true;
  }
  else if(state==0){
    screen=10;
    redraw=true;
    digitalWrite(lr, HIGH);
    tone(3, NOTE_G3, 1000);
    timeComp=0;
    timeCompNeedReset = true;
  }
  else{
    screen=7;
    redraw=true;
  }
}

void serialEvent() {
  while(Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void readRFID(){
  int i;
  int len=0;
  String id;
  while (RFID.available() > 0) 
  {
     i = RFID.read();
     len=len+1;
     id=id+String(i);
     if(i==3 && len==14){
      if(id!=lastKey && millis()-lastKeyPass>=10000){
        Serial.println(id);
        lastKey=id;
        lastKeyPass=millis();
        id="";
        delay(100);
      }
     }
  }
}

void readRFIDpay(float value){
  int i;
  int len=0;
  String id;
  while (RFID.available() > 0) 
  {
     i = RFID.read();
     id=id+String(i);
     len=len+1;
     if(i==3 && len==14){
      if(id!=lastKey && millis()-lastKeyPass>=10000){
        Serial.println(id+"&"+String(value));
        lastKey=id;
        lastKeyPass=millis();
        id="";
        delay(100);
      }
     }
  }
}
