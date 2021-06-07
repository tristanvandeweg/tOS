#include <Arduino.h>
#include <LiquidCrystal.h>
//#include <Wire.h>
//#include <menus.h>
//#include <SoftwareSerial.h>

#define ProMicro
#ifdef ProMicro
#define Serial Serial1
#endif

// Options
#define BLOCK_CURSOR "Cursor "
bool blockCursor = false;
#define ERROR_MESSAGES "Errors "
bool errors = true;
// -------

bool connectedToCPU = false;
bool enableMenu = true;
//MenuItem menu[];
//void menus[];
int curMenu = 0;
int minMenu = 0;
int maxMenu = 1;
int funcMenu = 1;
bool curMenuPrinted = false;
String IORead;
bool serialRecv = false;
char serialChar;
String input;
bool inputEnabled = false;

int cursorX = 0;
int cursorY = 0;
int oldCursorX = 0;

//SoftwareSerial IOSerial = SoftwareSerial(8, 9);

LiquidCrystal lcd = LiquidCrystal(2, 3, 4, 5, 6, 7);
#define BL_PIN 8
#define ERROR_PIN 10

// Characters

byte logoT[] = {
  B00000,
  B00000,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00000
};

byte logoTopLeft[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11100,
  B11110,
  B10111,
  B10011
};

byte logoTopRight[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00001,
  B00001,
  B00001,
  B00001
};

byte logoBottomLeft[] = {
  B10011,
  B10111,
  B11110,
  B11100,
  B11111,
  B00000,
  B00000,
  B00000
};

byte logoBottomRight[] = {
  B11001,
  B11101,
  B01111,
  B00111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte basicArrowUp[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B01110,
  B11111
};

byte basicArrowDown[8] = {
  B11111,
  B01110,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte scrollBarTop[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B11111,
  B01010,
  B01010,
  B01010
};

byte scrollBarBottom[8] = {
  B01010,
  B01010,
  B01010,
  B11111,
  B10001,
  B10001,
  B10001,
  B11111
};

byte scrollBarMiddle[8] = {
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010
};

byte scrollBarMiddleBottomWheel[8] = {
  B01010,
  B01010,
  B01010,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte scrollBarMiddleTopWheel[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01010,
  B01010,
  B01010
};

byte scrollBarTopWheel[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111,
  B11111
};

byte scrollBarBottomWheel[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B10001,
  B10001,
  B10001,
  B11111
};

byte blockSelect[] = {
  B00000,
  B00000,
  B11110,
  B11110,
  B11110,
  B11110,
  B00000,
  B00000
};

void setup() {
  lcd.begin(20, 4);
  connectedToCPU = false;
  //Wire.begin(0x4F);
  Serial.begin(19200);
  Serial.setTimeout(50);
  pinMode(BL_PIN, OUTPUT);
  pinMode(ERROR_PIN, INPUT_PULLUP);
  if(!enableMenu){
    curMenu = -1;
    funcMenu = 0;
    digitalWrite(BL_PIN, LOW);
  }else{
    digitalWrite(BL_PIN, HIGH);
  }
}

bool menuUp(){
  if(curMenu < maxMenu){
    curMenu++;
    funcMenu = curMenu + 1;
    curMenuPrinted = false;
    return true;
  }
  return false;
}

bool menuDown(){
  if(curMenu > minMenu){
    curMenu--;
    funcMenu = curMenu + 1;
    curMenuPrinted = false;
    return true;
  }
  return false;
}

/*class MenuItem
{
private:
  
public:
  MenuItem();
  ~MenuItem();
  void MenuFunc;
};

MenuItem::MenuItem(void m_MenuFunc)
{
  MenuFunc = 
}

MenuItem::~MenuItem()
{
}*/

// LCD Functions ---------------------------------------------------------------------------------------------------------------

void clearLCD(){
  inputEnabled = false;
  lcd.clear();
  lcd.home();
  cursorX = 0;
  cursorY = 0;
  lcd.noCursor();
  lcd.noBlink();
}

void setCursor(){
  lcd.setCursor(cursorX, cursorY);
}

void clearLineInput(){
  clearLCD();
  cursorX = 0;
  cursorY = 0;
  setCursor();
  lcd.print(input.substring(0, 19));
  if(input.length() > 19){
    cursorY++;
    setCursor();
    lcd.print(input.substring(20, 38));
    if(input.length() > 38){
      cursorY++;
      setCursor();
      lcd.print(input.substring(39, 57));
      cursorX = input.length() - 38;
    }else{
      cursorX = input.length() - 19;
    }
  }else{
    cursorX = input.length();
  }
}

const long blinkSpeed = 500;
long cursorControl = 0;
bool cursorBool = false;
void blinkCursor(){ // Cursor blinking
  if(millis() - cursorControl > blinkSpeed){
    cursorControl = millis();
    if(!cursorBool){
      if(!blockCursor){
        lcd.cursor();
      }else{
        lcd.blink();
      }
      cursorBool = true;
    }else{
      if(!blockCursor){
        lcd.noCursor();
      }else{
        lcd.noBlink();
      }
      cursorBool = false;
    }
  }
}

void endLine(){
  cursorX = 0;
  cursorY++;
  setCursor();
}

void printLCD(String _print, unsigned int _lineLimit){ // Print
  oldCursorX = cursorX;
  setCursor();
  if(_print.length() + cursorX >= _lineLimit){
    lcd.print(_print.substring(0, (oldCursorX - _lineLimit) * -1));
    cursorX = 0;
    cursorY++;
    setCursor();

    if(cursorY > 3){
      clearLCD();
      curMenuPrinted = false;
      return;
    }
    if (_print.substring((oldCursorX - _lineLimit) * -1).length() >= _lineLimit)
    {
      lcd.print(_print.substring((oldCursorX - _lineLimit) * -1, _lineLimit * 2));
      cursorX = 0;

    }else{
      lcd.print(_print.substring((oldCursorX - _lineLimit) * -1));
      cursorX += _print.substring((oldCursorX - _lineLimit) * -1).length();
      setCursor();
    }
    
  }else{
    lcd.print(_print);
    cursorX += _print.length();
  }
}

void clearScreen(bool _input){
  clearLCD();
  if(_input){
    printLCD(input, 19);
  }
}

void createScrollBar(){ //Draw a scrollbar
  lcd.createChar(0, scrollBarTop);
  lcd.createChar(1, scrollBarBottom);
  lcd.createChar(2, scrollBarMiddle);
  lcd.setCursor(19, 0);
  lcd.write(byte(0));
  lcd.setCursor(19, 1);
  lcd.write(byte(2));
  lcd.setCursor(19, 2);
  lcd.write(byte(2));
  lcd.setCursor(19, 3);
  lcd.write(byte(1));
  setCursor();
  //scrollToPosition(_pos);
}

void scrollToPosition(int _pos){ //Draw a scrollbar in a position
  createScrollBar();
  switch (_pos)
  {
  case 0:
    lcd.createChar(3, scrollBarTopWheel);
    lcd.setCursor(19, 0);
    lcd.write(byte(3));
    break;
  
  case 1:
    lcd.createChar(3, scrollBarMiddleTopWheel);
    lcd.setCursor(19, 1);
    lcd.write(byte(3));
    break;
  
  case 2:
    lcd.createChar(3, scrollBarMiddleBottomWheel);
    lcd.setCursor(19, 1);
    lcd.write(byte(3));
    break;
  
  case 3:
    lcd.createChar(3, scrollBarMiddleTopWheel);
    lcd.setCursor(19, 2);
    lcd.write(byte(3));
    break;
  
  case 4:
    lcd.createChar(3, scrollBarMiddleBottomWheel);
    lcd.setCursor(19, 2);
    lcd.write(byte(3));
    break;
  
  case 5:
    lcd.createChar(3, scrollBarBottomWheel);
    lcd.setCursor(19, 3);
    lcd.write(byte(3));
    break;
  default:
    break;
  }
  setCursor();
}

void handleInput(){
  Serial.println(input);
}

// Menu functions -------------------------------------------------------------------------------------------------------------------------------------------------

void Startscreen(){ //Screen on startup
  if(!curMenuPrinted){
    lcd.createChar(0, logoT);
    clearLCD();
    lcd.write(byte(0));
    lcd.print("OS");
    lcd.createChar(4, logoTopLeft);
    lcd.createChar(5, logoTopRight);
    lcd.createChar(6, logoBottomLeft);
    lcd.createChar(7, logoBottomRight);
    lcd.setCursor(0, 3);
    lcd.print("Starting up");
    lcd.setCursor(0, 1);
    lcd.write(byte(4));
    lcd.setCursor(1, 1);
    lcd.write(byte(5));
    lcd.setCursor(0, 2);
    lcd.write(byte(6));
    lcd.setCursor(1, 2);
    lcd.write(byte(7));
    Serial.println("R");
    curMenuPrinted = true;
  }
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("Starting up   ");
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("Starting up.  ");
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("Starting up.. ");
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("Starting up...");
  Serial.print("R");
}

void Error(int _code, String _text){ //Error message printing
  if(errors){
    clearLCD();
    lcd.print("Error ");
    lcd.print(_code);
    lcd.setCursor(0, 1);
    lcd.print(_text);
    lcd.setCursor(0, 2);
    lcd.print("Go back to home?");
    //curMenuPrinted = true;
    curMenuPrinted = false;
    while(digitalRead(ERROR_PIN)){}
  }
}

void Home(){ //Homescreen
  if(!curMenuPrinted){
    clearLCD();
    createScrollBar();
    cursorX = 0;
    cursorY = 0;
    setCursor();
    inputEnabled = true;
    curMenuPrinted = true;
  }
  blinkCursor();
}

int option = 0;
int optionScroll = 0;
void Settings(int _arg){ //Settings screen
  switch (_arg)
  {
  case 1:
    lcd.setCursor(12, option - optionScroll);
    lcd.print(" ");
    option--;
    break;
  case 2:
    lcd.setCursor(12, option - optionScroll);
    lcd.print(" ");
    option++;
    break;
  case 3:
    switch (option)
    {
    case 0:
      blockCursor = !blockCursor;
      lcd.setCursor(13, 0 - optionScroll);
      if(blockCursor){
        lcd.print("BOX ");
      }else{
        lcd.print("LINE");
      }
      break;

    case 1:
      errors = !errors;
      lcd.setCursor(13, 1 - optionScroll);
      if (errors)
      {
        lcd.print("ON ");
      }else{
        lcd.print("OFF");
      }
      
    default:
      break;
    }
    break;
  default:
    if(!curMenuPrinted){
      option = 0;
      optionScroll = 0;
      clearLCD();
      createScrollBar();
      lcd.home();
      lcd.print(BLOCK_CURSOR);
      lcd.setCursor(13, 0 - optionScroll);
      if(blockCursor){
        lcd.print("BOX ");
      }else{
        lcd.print("LINE");
      }
      lcd.setCursor(0, 1 - optionScroll);
      lcd.print(ERROR_MESSAGES);
      lcd.setCursor(13, 1 - optionScroll);
      if(errors){
        lcd.print("ON ");
      }else{
        lcd.print("OFF");
      }
      lcd.createChar(4, blockSelect);
      curMenuPrinted = true;
    }
    lcd.setCursor(12, option - optionScroll);
    lcd.write(4);
    break;
  }
}

String tabs[4] = {"", "", "", ""};
String options1[15] = {""};
String options2[15] = {""};
String options3[15] = {""};
String options4[15] = {""};
int menuScroll = 0;
int tab = 0;
void CustomMenu(){
  if(!curMenuPrinted){
    clearLCD();
    if(tabs[1] == ""){
      for (int i = 0; i < 15; i++)
      {
        if(i - menuScroll >= 0 && i - menuScroll <= 3){
          lcd.setCursor(0, i - menuScroll);
          lcd.print(options1[i]);
        }
      }
    }else{
      for (int i = 0; i < 4; i++)
      {
        lcd.setCursor(15, i);
        lcd.print(tabs[i]);
      }

      switch (tab)
      {
      default:
      case 0:
        for (int i = 0; i < 15; i++)
        {
          if(i - menuScroll >= 0 && i - menuScroll <= 3){
            lcd.setCursor(0, i - menuScroll);
            lcd.print(options1[i]);
          }
        }
        break;
      case 1:
        for (int i = 0; i < 15; i++)
        {
          if(i - menuScroll >= 0 && i - menuScroll <= 3){
            lcd.setCursor(0, i - menuScroll);
            lcd.print(options2[i]);
          }
        }
        break;
      case 2:
        for (int i = 0; i < 15; i++)
        {
          if(i - menuScroll >= 0 && i - menuScroll <= 3){
            lcd.setCursor(0, i - menuScroll);
            lcd.print(options3[i]);
          }
        }
        break;
      case 3:
        for (int i = 0; i < 15; i++)
        {
          if(i - menuScroll >= 0 && i - menuScroll <= 3){
            lcd.setCursor(0, i - menuScroll);
            lcd.print(options4[i]);
          }
        }
        break;
      }
    }
    curMenuPrinted = true;
  }
}

void getMenu(){ //Run appropriate menu function
  switch (curMenu)
  {
  case 0:
    Startscreen();
    break;
  case 1:
    Home();
    break;
  case 2:
    Settings(0);
    break;
  case 3:
    CustomMenu();
    break;
  default:
    Error(0, "Menu not found");
    break;
  }
}

void loop() { //Main loop
  //menu[funcMenu](); // Run active menu
  getMenu();
  while (Serial.available() > 0) //Handle serial data
  {
    serialRecv = true;
    IORead = Serial.readString();
    //Serial.println(IORead);
  }
  if(serialRecv){
    serialChar = IORead.charAt(0);
    //Serial.println(serialChar);
    switch (serialChar)
    {
    case 'c': //Text input
      if(inputEnabled){
        input += IORead.substring(1);
        /*oldCursorX = cursorX;
        cursorX += IORead.substring(1).length();
        //Serial.println(cursorX);
        if(cursorX == 19){
          cursorX = 0;
          cursorY++;
          if(cursorY > 3){
            clearLineInput();
          }
          setCursor();
        }
        if(cursorX > 19){
          lcd.print(IORead.substring(1, 19 - oldCursorX + 1));
          Serial.println(19 - oldCursorX + 1);
          cursorX = 0;
          cursorY++;
          if(cursorY > 3){
            clearLineInput();
          }
          setCursor();
          lcd.print(IORead.substring(19 - oldCursorX + 1));
        }else{
          lcd.print(IORead.substring(1));
        }*/
        printLCD(IORead.substring(1), 19);
      }
      break;

    case 'e': //Enter / Confirm input
      if(inputEnabled){
        handleInput();
        input = "";
        cursorX = 0;
        cursorY++;
        if(cursorY > 3){
          clearScreen(false);
        }
        setCursor();
      }else if(curMenu == 2){
        Settings(3);
      }
      break;

    case 'C': //Clear screen
      if(inputEnabled){
        clearLCD();
        input = "";
        cursorX = 0;
        cursorY = 0;
        setCursor();
        curMenuPrinted = false;
      }
      break;

    case 'u': //Up
      if(curMenu == 2){
        Settings(1);
      }
      break;

    case 'd': //Down
      if(curMenu == 2){
        Settings(2);
      }
      break;

    case 'm': //Switch menu screen
      curMenu = IORead.substring(1).toInt();
      curMenuPrinted = false;
      //Serial.println(curMenu);
      break;

    case 's': //Scroll
      scrollToPosition(IORead.substring(1).toInt());
      break;
    
    case 't':
      tabs[0] = IORead.substring(1, 5);
      tabs[1] = IORead.substring(6, 10);
      tabs[2] = IORead.substring(11, 15);
      tabs[3] = IORead.substring(16, 20);
      break;

    case 'o':
    Serial.println(IORead.charAt(1));
    Serial.println(IORead.charAt(2));
      switch ((int)IORead.charAt(1) - 48)
      {
      case 0:
          options1[(int)IORead.charAt(2) - 48] = IORead.substring(3);
        break;
      case 1:
          options2[(int)IORead.charAt(2) - 48] = IORead.substring(3);
        break;
      case 2:
          options3[(int)IORead.charAt(2) - 48] = IORead.substring(3);
        break;
      case 3:
          options4[(int)IORead.charAt(2) - 48] = IORead.substring(3);
        break;
      
      default:
        break;
      }
      break;
    
    case 'A':
      connectedToCPU = true;
      break;

    default: //Error
      if(connectedToCPU){
        Error(1, "Error reading Serial");
      }
      break;
    }
    serialRecv = false;
  }
}