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
#define DELTA 60      //в минутах  (60)
#define UPDATING 4 //в секуднах   обновление экрана (4)
#define DELTA2 2 //в секундах   откат энкодера  (15)
#define REBOOT 30  //время до перезагрузки в днях
#define ENCDEL 800 //в миллисекундах
#define BOOP 5 //пищалка в секундах

OneWire oneWire(ONE_WIRE_BUS);
Encoder enc1(CLK, DT, SW);
LiquidCrystal_I2C lcd(0x27,20,4); //16
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
iarduino_RTC time(RTC_DS1307);

float temp[72], lTemp;
int tm[72], data1[72], data2[72];   //сначала месяц и день потом год
boolean triger = 0, beginning = 0, encFlag = 0;
byte symb0[8] = {B01010, B10101, B10001, B10011, B10101, B11001, B10001, B00000}; //Й
byte symb1[8] = {B10001, B10001, B10011, B10101, B11001, B10001, B10001, B00000}; //И
byte symb2[8] = {B10101, B10101, B10101, B10101, B10101, B11111, B00001, B00001}; //Щ
byte symb3[8] = {B11111, B10001, B11111, B00011, B00101, B01001, B10001, B00000}; //Я
byte symb4[8] = {B11111, B10001, B10001, B10001, B10001, B10001, B10001, B00000}; //П
unsigned long time1, time2, time3;

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
  lcd.print("V1.5");
  sensors.setResolution(insideThermometer, 9);
  sensors.getAddress(insideThermometer, 0);
  pinMode(3, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  attachInterrupt(1, buttonTick, FALLING);
  attachInterrupt(0, isr, CHANGE);
  lcd.createChar(0, symb0);
  lcd.createChar(1, symb1);
  lcd.createChar(2, symb2);
  lcd.createChar(3, symb3);
  lcd.createChar(4, symb4);
  time.begin();
  enc1.setType(TYPE2);
  delay(1600);
  time1 = millis();
  time2 = millis();
  lTemp = getTemp();
  readMas(&temp[0], &data1[0], &data2[0],&tm[0]);
  lcd.clear();
  printTemp();
}

void loop() {
  if (encFlag == 1)     //срабатывание энкодера и прорисовка меню
  {
    time3 = millis();
    while (millis() - time3 <= ENCDEL)
    {
      enc1.tick();
      if (enc1.isLeft())
        drawMenu(70);
      if (enc1.isRight())
        drawMenu(0);
    }
    encFlag = 0; 
  }
  if (triger == 1)      //срабатывание кнопки настройки
  {
    pushSound();
    Settings();
    triger = 0;
  }
  if (millis() - time1 >= DELTA * 60000 && getTemp() != -127)
  {
    moveMas(&temp[0], &data1[0], &data2[0],&tm[0]);
    temp[71] = getTemp();
    data1[71] = time.day * 100 + time.month;
    data2[71] = time.year + 2000;
    tm[71] = time.Hours * 100 + time.minutes; 
    saveMas(&temp[0], &data1[0], &data2[0],&tm[0]);
    time1 = millis();
    if (millis() >= REBOOT * 86400000)
      reboot();
  }
    else
      if (getTemp() == -127)
        tempError();
  if (millis() - time2 >= UPDATING * 1000)
  {
    printTemp();
    time2 = millis();
  }
  Serial.println(encFlag);
}
