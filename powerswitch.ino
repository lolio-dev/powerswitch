#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define potPin A0

const int triggerPin = 7; // Numéro de la broche de déclenchement
const int buttonPin = 8;
const int transPin = 9;

int potValue;

int repeats = 0;
int repeatTime = 0;

bool isRunning = false;
bool isRepeatsSelected = false;
bool isRepeatTimeSelected = false;
bool isButtonPushed = false;
bool buttonState = false;
bool isFinished = false;

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); 


void setup() {
  Serial.begin(9600);
  if (!screen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  screen.clearDisplay(); 
  screen.setTextSize(1.5);          
  screen.setTextColor(WHITE); 
  
  pinMode(triggerPin, OUTPUT); // Définit la broche de déclenchement en sortie
  pinMode(potPin, INPUT);
  pinMode(transPin, OUTPUT);
}

void loop() {
  screen.clearDisplay();  

  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    isButtonPushed = false;
  }
  
  if (!isRunning && !isFinished) {
    oledDisplayCenter("powerswitch");
    
    screen.setCursor(0, 20);
    screen.println("repetitions:");
    screen.println("temps:");
    screen.setCursor(45, 47);
    screen.println("Lancer");
    
    screen.setCursor(116, 20);
    
    screen.print(formatInt(repeats));
    
    screen.setCursor(98, 30);

    screen.print(formatInt(repeatTime / 60));
    screen.print(":");
    screen.print(formatInt(repeatTime % 60));
    
    if (!isRepeatsSelected && !isButtonPushed) {
      repeats = map(analogRead(potPin), 0, 1023, 1, 30);

      if (buttonState == HIGH) {
        isRepeatsSelected = true;
        isButtonPushed = true;
      }
    } else if (!isRepeatTimeSelected && !isButtonPushed) {
      repeatTime = map(analogRead(potPin), 0, 1023, 0, 1200);

      if (digitalRead(buttonPin) == HIGH && !isButtonPushed) {
        isRepeatTimeSelected = true;
        isButtonPushed = true;
      } else {
        isButtonPushed = false;
      }
    } else if (!isButtonPushed){
      screen.drawRect(40, 42, 44, 17, WHITE);

      if (digitalRead(buttonPin) == HIGH) {
        isRunning = true; 
        isButtonPushed = true;
      }
    }
  } else if (isRunning) {
    unsigned long startTime = millis();
    for (int repeat=0; repeat < repeats; repeat++) {
      showRepetInfo(false, startTime, repeat);

      delay(repeatTime * 1000);

      triggerRelais();
     
      showRepetInfo(true, startTime, repeat);

      delay(repeatTime * 1000);
      
      triggerRelais();
    } 
    isRunning = false;
    isFinished = true;
  } else {
    oledDisplayCenter("Serie terminee !"); 

    screen.setCursor(0, 20);
    screen.print("Temps total:");
    screen.setCursor(98, 20);
    screen.print(formatTime(repeats * repeatTime * 2 / 1000));

    screen.setCursor(23, 43);
    screen.println("Nouvelle serie");
    screen.drawRect(18, 38, 93, 17, WHITE);

    if (buttonState == HIGH) {
      isFinished = false;
      isRepeatsSelected = false;
      isRepeatTimeSelected = false;
      isButtonPushed = true;
    }
  }
  screen.display();
}

void oledDisplayCenter(String text) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  screen.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  screen.setCursor((SCREEN_WIDTH - width) / 2, height);
  screen.println(text);
}

String formatTime(unsigned long timeInMillis) {
  unsigned long seconds = timeInMillis / 1000;
  unsigned int minutes = seconds / 60;
  unsigned int hours = minutes / 60;
  
  String timeString = "";
  if (hours < 10) {
    timeString += "0";
  }
  timeString += String(hours);
  timeString += ":";
  if (minutes < 10) {
    timeString += "0";
  }
  timeString += String(minutes);
  
  return timeString;
}

String formatInt(int value) {
  String stringValue = "";

  if (value < 10) {
    stringValue += "0";
  }
  stringValue += String(value);

  return stringValue;
}

void showRepetInfo(bool isPos, unsigned long startTime, int repeat){
  screen.clearDisplay(); 
  oledDisplayCenter("Serie en cours"); 
  
  screen.setCursor(0, 50);      
      
  screen.print(formatInt(repeat + 1));

  unsigned long totalElapsedTime = millis() - startTime; 

  screen.setCursor(98, 50);
  screen.print(formatTime(totalElapsedTime));

  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int crossSize = 20;
  int crossThickness = 4; // Epaisseur des branches de la croix

  if (isPos){
    for (int i = -(crossThickness / 2); i < (crossThickness / 2); i++) {
      int y = centerY + i;
      screen.drawLine(centerX - crossSize / 2, y, centerX + crossSize / 2, y, WHITE);
    }
  
    for (int i = -(crossThickness / 2); i < (crossThickness / 2); i++) {
      int x = centerX + i;
      screen.drawLine(x, centerY - crossSize / 2, x, centerY + crossSize / 2, WHITE);
    }
  } else {
    for (int i = -(crossThickness / 2); i < (crossThickness / 2); i++) {
      int y = centerY + i;
      screen.drawLine(centerX - crossSize / 2, y, centerX + crossSize / 2, y, WHITE);
    }
  }
  screen.display();
}

void triggerRelais() {
  digitalWrite(triggerPin, HIGH);
  delay(100);
  digitalWrite(triggerPin, LOW);
}
