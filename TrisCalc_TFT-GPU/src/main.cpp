#define DEBUG

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//  VARS
MCUFRIEND_kbv lcd;       // The TFT LCD Display
String IORead;           // The data received from the CPU
bool serialRecv = false; // Received data from CPU?
char serialChar;         // Command identifier, the first char sent by the CPU

//  CONFIG
bool errors = true;          // Are error messages on?
bool statusBar = true;       // Is the status bar enabled?
bool connectedToCPU = false; // Has the CPU Sent the startup message?

//  STATUS VARS
uint8_t wifiStrength = 0; // Strength of the WiFi connection
uint8_t CPUUsage = 0;     // CPU Usage in %

// STATUS UPDATE VARS
unsigned long updateSpeed = 2500; // Update every 'updateSpeed' milliseconds
unsigned long lastUpdate = 0;     // When was the last status update
bool statusUpdated = false;       // Was new status data received

//  DEFAULT COLORS
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//  COLOR VARS
uint16_t textColor = YELLOW;   // Color of the text
uint16_t bgColor = BLACK;      // Color of the Background
uint16_t barColor = 0xDDDD;    // Background color of the status bar
uint16_t barTextColor = BLACK; // Text color of the status bar

//  GRAPH VARS
uint16_t graphBG = WHITE;   // Background color of the graph screen
uint16_t graphAxes = BLACK; // Color of the axes of the graph screen
uint16_t graph1 = BLUE;     // Color of Graph 1
uint16_t graph2 = RED;      // Color of Graph 2
uint16_t graph3 = GREEN;    // Color of Graph 3
uint16_t graph4 = YELLOW;   // Color of Graph 4
uint16_t graph5 = MAGENTA;  // Color of Graph 5

uint16_t *graphX; // Array of values in the current graph
uint16_t *graphY; // Array oof values in the current graph

void clearLCD() // Clear the screen to the background color
{
  lcd.fillScreen(bgColor);
  lcd.setCursor(0, 0);
}

void print(int _fontSize, String _msg) // Print message on screen
{
  lcd.setTextSize(_fontSize);
  lcd.setTextColor(textColor);
  lcd.print(_msg);
}

void drawGraph(uint16_t *_valX, uint16_t *_valY, uint8_t _ID) // Draw graph
{
  uint16_t _color;
  switch (_ID)
  {
  default:
  case 1:
    _color = graph1;
    break;
  case 2:
    _color = graph2;
    break;
  case 3:
    _color = graph3;
    break;
  case 4:
    _color = graph4;
    break;
  case 5:
    _color = graph5;
    break;
  }
  lcd.fillScreen(graphBG);
  for (uint16_t _X = 0; _X < sizeof(_valX); _X++)
  {
    if (_X + 1 <= sizeof(_valX))
    {
      lcd.drawLine(_valX[_X], _valY[_X], _valX[_X + 1], _valY[_X + 1], _color);
    }
  }
}

void drawStatusBar() // Draw the status bar
{
  lcd.fillRect(0, lcd.height() - 25, lcd.width(), 25, barColor);
  lcd.setCursor(5, lcd.height() - 20);
  lcd.setTextSize(2);
  lcd.setTextColor(barTextColor);
  char tmp[128];
  strcpy(tmp, "CPU:");
  char tmpCPU[4];
  String(CPUUsage).toCharArray(tmpCPU, 4);
  strcat(tmp, tmpCPU);
  strcat(tmp, "% WiFi:");
  char tmpWiFi[4];
  String(wifiStrength).toCharArray(tmpWiFi, 4);
  strcat(tmp, tmpWiFi);
  lcd.print(tmp);
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  lcd.begin(lcd.readID());
  lcd.setRotation(1);
  lcd.fillScreen(BLACK);
  lcd.setCursor(0, 0);
  lcd.setTextColor(YELLOW);
  lcd.setTextSize(1);
  lcd.print("Hello World!");
  drawStatusBar();
}

void Error(int _code, String _text) // Draw error message if enabled
{
  if (errors)
  {
    clearLCD();
    lcd.println(String("Error: ") + _code);
    lcd.println(_text);
  }
}

void loop()
{
  if (statusBar && millis() >= lastUpdate + updateSpeed && statusUpdated)
  {
    lastUpdate = millis();
    drawStatusBar();
    statusUpdated = false;
  }

  while (Serial.available() > 0)
  {
    serialRecv = true;
    IORead = Serial.readString();
  }
  if (serialRecv)
  {
    serialChar = IORead.charAt(0);
    switch (serialChar)
    {
    case 'I':
      // TODO Load image from given path and draw it
      break;
    case 'S': // Get Status
      statusUpdated = true;
      switch (IORead.charAt(1))
      {
      case 'C':
        CPUUsage = IORead.substring(2).toInt();
        break;
      case 'W':
        wifiStrength = IORead.substring(2).toInt();
        break;
      default:
        Error(2, "Error getting status");
        break;
      }
      break;
    case 'g': // Graph function
      switch (IORead.charAt(1))
      {
      case 'x': // get X
        graphX[IORead.substring(2).toInt()] = IORead.substring(3).toFloat();
        break;
      case 'y': // get Y
        graphY[IORead.substring(2).toInt()] = IORead.substring(3).toFloat();
        break;
      default: // draw
        drawGraph(graphX, graphY, IORead.substring(2).toInt());
        break;
      }
      break;
    case 'T': // Task mgr
      lcd.fillScreen(BLACK);
      statusUpdated = true;
      lastUpdate = 0;
      lcd.setCursor(0, 0);
      lcd.setTextColor(YELLOW);
      lcd.setTextSize(2);
      lcd.println("Tasks");
      lcd.setTextSize(1);
      lcd.print(IORead.substring(1));
      #ifdef DEBUG
      Serial.println(IORead.substring(1));
      #endif
      break;
    case 's': // Change settings
      switch (IORead.charAt(1))
      {
      case 'E':
        errors = true;
        break;
      case 'e':
        errors = false;
        break;
      case 'C': // change colors
        switch (IORead.charAt(2))
        {
        case 'b':
          bgColor = IORead.substring(3).toInt();
          break;
        case 't':
          textColor = IORead.substring(3).toInt();
          break;
        case 'B':
          barColor = IORead.substring(3).toInt();
          break;
        case 'T':
          barTextColor = IORead.substring(3).toInt();
          break;
        case 'g':
          graphBG = IORead.substring(3).toInt();
          break;
        case 'A':
          graphAxes = IORead.substring(3).toInt();
          break;
        case '1':
          graph1 = IORead.substring(3).toInt();
          break;
        case '2':
          graph2 = IORead.substring(3).toInt();
          break;
        case '3':
          graph3 = IORead.substring(3).toInt();
          break;
        case '4':
          graph4 = IORead.substring(3).toInt();
          break;
        case '5':
          graph5 = IORead.substring(3).toInt();
          break;
        default:
          Error(3, "Error changing settings");
          break;
        }
        break;

      default:
        break;
      }
      break;
    case 'A': // Connected
      connectedToCPU = true;
      break;

    default:
      if (connectedToCPU)
      {
        Error(1, "Error reading Serial");
      }
      break;
    }
    serialRecv = false;
  }
}