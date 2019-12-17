#include <avr/eeprom.h>
#include <LiquidCrystal_I2C.h>
#include <iarduino_RTC.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include "GyverEncoder.h"

#define CLK 2
#define DT 4
#define SW 6
#define BUZ 5
#define ONE_WIRE_BUS 10
#define LED 9
#define DELTA 1000 //в минутах

OneWire oneWire(ONE_WIRE_BUS);
Encoder enc1(CLK, DT, SW);
LiquidCrystal_I2C lcd(0x27,16,4);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
iarduino_RTC time(RTC_DS1307);

float temp[24], data[24], lTemp;
int tm[24];
boolean triger = 0, beginning = 0;
byte symb0[8] = {B01010, B10101, B10001, B10011, B10101, B11001, B10001, B00000}; //Й
byte symb1[8] = {B10001, B10001, B10011, B10101, B11001, B10001, B10001, B00000}; //И
byte symb2[8] = {B10101, B10101, B10101, B10101, B10101, B11111, B00001, B00001}; //Щ
byte symb3[8] = {B11111, B10001, B11111, B00011, B00101, B01001, B10001, B00000}; //Я
byte symb4[8] = {B11111, B10001, B10001, B10001, B10001, B10001, B10001, B00000}; //П
unsigned long time1;

void setup() {
  Serial.begin(9600);
  lcd.init();                     
  lcd.backlight();// Включаем подсветку дисплея
  lcd.setCursor(0, 0); //сначала место на строке, потом ряд
  lcd.print("Made by");
  lcd.setCursor(0, 1);
  lcd.print("Aleksandr Orlov");
  lcd.setCursor(0, 2);
  lcd.print("Loading...");
  lcd.setCursor(0, 3);
  lcd.print("V1.0");
  sensors.setResolution(insideThermometer, 9);
  sensors.getAddress(insideThermometer, 0);
  pinMode(3, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  attachInterrupt(1, buttonTick, FALLING);
  enc1.setType(TYPE2);
  lcd.createChar(0, symb0);
  lcd.createChar(1, symb1);
  lcd.createChar(2, symb2);
  lcd.createChar(3, symb3);
  lcd.createChar(4, symb4);
  time.begin();
  delay(1300);
  lcd.clear();
  time1 = millis();
  sensors.requestTemperatures();
  lTemp = getTemp();
  readMas(&temp[0], &data[0], &tm[0]);
}

void loop() {
  enc1.tick();
  if (enc1.isLeft() || enc1.isRight())
  {
    Serial.println("Scanned!");
    drawMenu();
  }
  if (triger == 1)      //срабатывание кнопки настройки
  {
    pushSound();
    SetTime();
    triger = 0;
  }
  if (millis() - time1 >= DELTA * 60000 && getTemp() != -127)   //запись значений
  {
    moveMas(&temp[0], &data[0], &tm[0]);
    temp[23] = getTemp();
    data[23]= time.day * 10000 + time.month * 10 + time.year;
    tm[23] = time.Hours * 100 + time.minutes; 
    saveMas(&temp[0], &data[0], &tm[0]);
    time1 = millis();
  }
  else
    if (getTemp() == -127)
      tempError();
  printTemp();
}
