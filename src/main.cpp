#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "SdFat.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 0 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define sizeTrend 30
int in1[sizeTrend];

SdFat SD;
Adafruit_INA219 ina219;
const byte interruptPin = 4;

unsigned long previousMillis = 0;
int timer = 300;
unsigned long interval = 200;
const int chipSelect = 10;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float energy = 0;
File TimeFile;
File VoltFile;
File CurFile;

//////////////////////////////////// Functions are here //////////////////////////////

int convert(int y, int mn, int mx, byte yTrend, byte heightTrend, int *in)
{
  int ny = y;
  ny = map(ny, mn, mx, heightTrend - 1 + yTrend, yTrend);
  //ny=map(ny,mn,mx,yTrend,heightTrend-1+yTrend);
  return ny;
}

void drawTrend(int widthTrend, int heightTrend, int xTrend, byte yTrend, byte cn, int *in)
{
  //добавляем новое значение
  in[sizeTrend - 1] = current_mA;
  byte oldX = 0;
  byte oldY = 0 + yTrend;
  int mn = 1000;
  int mx = 0;
  //сдвигаем график
  for (byte x = 0; x < (sizeTrend - 1); x++)
  {
    in[x] = in[x + 1];
  }

  //поиск мин и макс
  for (byte i = 0; i < sizeTrend; i++)
  {
    if (in[i] > mx)
    {
      mx = in[i];
    }
    if (in[i] < mn)
    {
      mn = in[i];
    }
  }
  if (mn == mx)
  {
    mx = mn + 1;
    mn = mn - 1;
  }
  //формирование буфера вывода
  for (byte x = 0; x < sizeTrend - 1; x++)
  {
    byte y = convert(in[x], mn, mx, yTrend, heightTrend, in);
    byte nxt_x = map(x, 0, sizeTrend - 1, 0, widthTrend);
    //отрисовка тренда
    display.drawLine(xTrend + oldX, oldY, xTrend + nxt_x, y, WHITE);
    oldX = nxt_x;
    oldY = y;

    //отрисовка рамки
    display.drawRect(xTrend, yTrend, widthTrend, heightTrend, WHITE);
  }
  //вывод минимума и максимума
  // display.setCursor(xTrend + widthTrend + 3, yTrend);
  // display.println(mx);
  // display.setCursor(xTrend + widthTrend + 3, yTrend + heightTrend - 8);
  // display.println(mn);
  // display.setCursor(xTrend + widthTrend + 3, yTrend + (heightTrend / 2) - 4);
  // display.println(in[sizeTrend - 1]); //текущее
  // oldX = 0;
  // oldY = convert(in[0], mn, mx, yTrend, heightTrend, in);
}

void displaydata()
{
  int count = timer - (millis() / 1000);

  display.setCursor(0, 0);
  // display.println(loadvoltage);
  // display.setCursor(35, 0);
  // display.println("V");
  // display.setCursor(50, 0);
  display.println(current_mA);
  display.setCursor(50, 0);
  display.println("mA");
  display.drawRect(83, 0, 45, 30, WHITE);
  display.setCursor(90, 3);
  display.println("Timer");
  display.setCursor(87, 20);
  display.println(count);
  display.setCursor(107, 20);
  display.println("sec");
  display.setCursor(0, 10);
  display.println(loadvoltage * current_mA);
  display.setCursor(50, 10);
  display.println("mW");
  display.setCursor(0, 20);
  display.println(energy);
  display.setCursor(50, 20);
  display.println("mWh");
}

void ina219values()
{
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  energy = energy + loadvoltage * current_mA / 3600;
}

void button() {
  Serial.println("button");
}
//////////////////////////////////// Functions are here //////////////////////////////

void setup()
{
  Serial.begin(9600);
  // pinMode(interruptPin, INPUT);
  // attachInterrupt(digitalPinToInterrupt(interruptPin), button, CHANGE);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  ina219.begin();
  SD.begin(chipSelect);

  
}
void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    display.clearDisplay();
    ina219values();
    displaydata();
    drawTrend(128, 30, 0, 33, 5, in1);
    display.display();
  }
}
