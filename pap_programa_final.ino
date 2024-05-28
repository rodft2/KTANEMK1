#define TIMER_MINUTES 7
#define TIMER_SECONDS 0

#define PIN_STRIKE_1 50 // strike 1
#define PIN_STRIKE_2 48 // strike 2
#define PIN_STRIKE_3 46  // strike 3
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
bool defused = false, exploded = false;
bool memoryModuleDefused = false, buttonModuleDefused = false, simonModuleDefused = false, whoModuleDefused = false;
bool explodedFromStrikes = false;
int nrStrikes = 0;

char serialCode[10];

void bombExploded();
void addStrike();

// function that generates the serial number for Simon Says
void generateSerialCode() 
{
  char alphanumeric[50] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  for ( int i = 0; i < 7; i++)
    serialCode[i] = alphanumeric[random(0, 35)];
}

// function that adds a strike

// The sounds for the buzzer

#define BUZZER 24

void victoryBuzzer() // the sound of the buzzer when the team wins
{
  delay(700);
  tone(BUZZER, 1000);
  delay(250);
  noTone(BUZZER);
  delay(50);
  tone(BUZZER, 600);
  delay(250);
  noTone(BUZZER);
  delay(50);
  tone(BUZZER, 1000);
  delay(1000);
  noTone(BUZZER);
}

void boomBuzzer() // the sound of the buzzer when the team loses
{
  delay(700);
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, 800);
    delay(300);
    noTone(BUZZER);
    delay(100);

    tone(BUZZER, 500);
    delay(300);
    noTone(BUZZER);

    tone(BUZZER, 300);
    delay(1000);
    noTone(BUZZER);

}
}
void defusedModuleBuzzer() // the sound of the buzzer when a module is defused
{
  tone(BUZZER, 500);
  delay(300);
  noTone(BUZZER);
  delay(100);

  tone(BUZZER, 1000);
  delay(300);
  noTone(BUZZER);
}

void strikeBuzzer() // the sound of the buzzer when the team makes a mistake
{
  tone(BUZZER, 800);
  delay(300);
  noTone(BUZZER);
  delay(100);

  tone(BUZZER, 300);
  delay(300);
  noTone(BUZZER);

}
// Simon Says Module

#define RED_LED_PIN 42
#define GREEN_LED_PIN 40
#define YELLOW_LED_PIN 38
#define BLUE_LED_PIN 36
#define RED_BTN_PIN A1
#define GREEN_BTN_PIN A2
#define YELLOW_BTN_PIN A3
#define BLUE_BTN_PIN A4
#define PIN_SIMON_LED_GREEN 44

unsigned long lastDebounceTimeRed = 0;
unsigned long lastDebounceTimeGreen = 0;
unsigned long lastDebounceTimeYellow = 0;
unsigned long lastDebounceTimeBlue = 0;
unsigned long debounceDelay = 50;

unsigned long currentMillisRed = 0;
unsigned long currentMillisGreen = 0;
unsigned long currentMillisYellow = 0;
unsigned long currentMillisBlue = 0;

int redLedState = 0;
int greenLedState = 0;
int yellowLedState = 0;
int blueLedState = 0;

int ledPins[4] = {42, 40, 38, 36};
int ledStates[4] = {0, 0, 0, 0};

int ledsNumber = 1;
int blinkingTime = 500;
unsigned long previousMillis = 0;
int ledSequence[4];

// array that contains the answers when the serial number contains a vowel
int answersWithVowel[3][4] = {{4, 3, 2, 1},
  {3, 4, 1, 2},
  {2, 3, 4, 1}
};

// array that contains the answers when the serial number doesn't contain a vowel
int answersWithoutVowel[3][4] = {{4, 2, 1, 3},
  {1, 3, 2, 4},
  {3, 4, 1, 2}
};

int currentLed=0, animationDelay = 5000;
unsigned long animationMillis = 0, beforeAnimationMillis;
int buttonsPressed = 0; 

int beforeAnimationDelay = 1000;

// the setup for Simon when it's already defused
void defusedSimonSetup() 
{
  simonModuleDefused = true;
  pinMode(PIN_SIMON_LED_GREEN, OUTPUT);
  digitalWrite(PIN_SIMON_LED_GREEN, HIGH);
}

// function that shuts down the leds after the module is finished
void simonModuleBoom()
{ 
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
}

// function that sets the module as being defused
void simonModuleDefusedPrint()
{ 
   defusedModuleBuzzer();
   simonModuleDefused = true;
   if(whoModuleDefused && simonModuleDefused && memoryModuleDefused && buttonModuleDefused) 
    {
      victoryBuzzer();
      defused = true;
    }
}

// function that checks if there's a vowel in the serial code and returns an answer accordingly
bool checkForVowel() 
{
  for (int i = 0; i < 7; i++)
    if (serialCode[i] == 'A' || serialCode[i] == 'E' || serialCode[i] == 'I' || serialCode[i] == 'O' || serialCode[i] == 'U')
      return 1;
  return 0;
}

// function that checks if the pressed button is the correct one in the answer sequence and
// turns on the specific led
void pressButton(int ledNr, int btnPin, int led, unsigned long &debounceTime, unsigned long &currentMillis, int &ledState) 
{
  int reading = digitalRead(btnPin);

  if (reading == HIGH)
  {
    debounceTime = millis();
    ledState = 1;
  }

  if (millis() - debounceTime > debounceDelay)
  {
    if (ledState == 1)
    {
    currentLed = ledsNumber;
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    buttonsPressed++;
    if(checkForVowel())
    {
      if(answersWithVowel[nrStrikes][ledSequence[buttonsPressed-1]-1] == ledNr)
      {
        if(buttonsPressed == ledsNumber)
          {
            buttonsPressed = 0;
            ledsNumber++;
            beforeAnimationMillis = millis();
            currentLed = -1;
            if(ledsNumber > 4)
              {
                simonModuleDefusedPrint();
                digitalWrite(PIN_SIMON_LED_GREEN, HIGH);
                currentLed = ledsNumber + 2;
              }
          }
      }
        else 
        {
            addStrike();
            if(nrStrikes < 3)
            { 
              currentLed = -1;
              beforeAnimationMillis = millis(); 
              buttonsPressed = 0;
            }
        }
    }
    else 
    {
      if(answersWithoutVowel[nrStrikes][ledSequence[buttonsPressed - 1] - 1] == ledNr)
      {
        if(buttonsPressed == ledsNumber)
          {
            buttonsPressed = 0;
            ledsNumber++;
            currentLed = -1;
            beforeAnimationMillis = millis();
            if(ledsNumber > 4)
              {
                simonModuleDefusedPrint();
                digitalWrite(PIN_SIMON_LED_GREEN, HIGH);
                currentLed = ledsNumber + 2;
              }
          }
      }
      else 
      {
        addStrike();
        if( nrStrikes < 3)
        { 
          currentLed = -1;
          beforeAnimationMillis = millis(); 
          buttonsPressed = 0;
        }
      }
    }
        
      digitalWrite(led, HIGH);
      currentMillis = millis();
      ledState = 2;
    }

    if (millis() - currentMillis >= blinkingTime && ledState == 2) 
    {
      digitalWrite(led, LOW);
      ledState = 0;
    }
  }

}

// function that generates the led sequence that needs to be resolved
void generateLedSequence() 
{
  for (int i = 0; i < 4; i++)
  {
    int x = random(1, 5);
    ledSequence[i] = x;
  }
}

// function that makes a led blink when a button is pressed
void blinkLed(int led,int &ledState) 
{
  unsigned long currentMillis = millis();
    
  if (currentMillis - previousMillis >= blinkingTime) 
  {
    previousMillis = currentMillis;

    if (ledState == LOW) 
    {
      ledState = HIGH;
    } 
    else 
    {
      ledState = LOW;
      currentLed++;
    }
    
    digitalWrite(led, ledState);
  }
}

// function that makes the leds blink in the order dictated by the led sequence that needs to be solved at the current stage
void ledAnimation() 
{
  if(currentLed == -1 && millis() - beforeAnimationMillis > beforeAnimationDelay) 
    currentLed = 0;
  
  if(currentLed == 0) 
    buttonsPressed = 0;
          
  if(currentLed < ledsNumber)
    blinkLed(ledPins[ledSequence[currentLed]-1], ledStates[ledSequence[currentLed]-1]);
  else
  {
    if(currentLed == ledsNumber) 
    {
      animationMillis = millis();
      currentLed++;
    }
    if(currentLed == ledsNumber+1 && millis() - animationMillis > animationDelay) 
      currentLed = 0;
  }
      
}

void simonSetup() 
{
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(PIN_SIMON_LED_GREEN, OUTPUT);
  pinMode(RED_BTN_PIN, INPUT);
  pinMode(GREEN_BTN_PIN, INPUT);
  pinMode(YELLOW_BTN_PIN, INPUT);
  pinMode(BLUE_BTN_PIN, INPUT);

  generateLedSequence();
}

void simonLoop() 
{
  if(!simonModuleDefused)
  {
      ledAnimation(); 
      pressButton(1, RED_BTN_PIN, RED_LED_PIN, lastDebounceTimeRed, currentMillisRed, redLedState);
      pressButton(2, GREEN_BTN_PIN, GREEN_LED_PIN, lastDebounceTimeGreen, currentMillisGreen, greenLedState);
      pressButton(3, YELLOW_BTN_PIN, YELLOW_LED_PIN, lastDebounceTimeYellow, currentMillisYellow, yellowLedState);
      pressButton(4, BLUE_BTN_PIN, BLUE_LED_PIN, lastDebounceTimeBlue, currentMillisBlue, blueLedState);  
    
  }
  
}
//Memory Module

#define PIN_MEMORY_BUTTON_1 A6
#define PIN_MEMORY_BUTTON_2 A7
#define PIN_MEMORY_BUTTON_3 A8
#define PIN_MEMORY_BUTTON_4 A9

#define PIN_MEMORY_LED_1 31
#define PIN_MEMORY_LED_2 33
#define PIN_MEMORY_LED_3 35
#define PIN_MEMORY_LED_4 37
#define PIN_MEMORY_LED_GREEN 39


int previousButton, button; //the previos and the current button
int rightPoz; //the answer to the current stage
int val[6], pos[6]; // the right values and positions of every stage
int stageNumber = 1;
int digits[5]; // the values of the current stage

bool printAnswer = true;
hd44780_I2Cexp memory(0x24);
void turnOffLeds()
{
  digitalWrite(PIN_MEMORY_LED_1, LOW);
  digitalWrite(PIN_MEMORY_LED_2, LOW);
  digitalWrite(PIN_MEMORY_LED_3, LOW);
  digitalWrite(PIN_MEMORY_LED_4, LOW);
  digitalWrite(PIN_MEMORY_LED_GREEN, LOW);
}

// function that generate a random digit from 1 to 4
int randomNumber() 
{
  return (random(20, 100)) / 20;
}

// generates the digits of the current stage
void generateNumbers() 
{
  digits[0] = randomNumber();
  digits[1] = randomNumber();
  digits[2] = randomNumber();
  digits[3] = randomNumber();

  // if the digits repeat:
  while (digits[2] == digits[1]) digits[2] = randomNumber();
  while (digits[3] == digits[1] || digits[3] == digits[2]) digits[3] = randomNumber();

  digits[4] = 10 - digits[3] - digits[2] - digits[1]; // the last digit is the last number from 1-4
}

// display the current digits
void displayNumbers() 
{
  memory.setCursor(8,0);
  memory.print(digits[0]);
   memory.setCursor(0,1);
   memory.print(digits[1]);
   memory.setCursor(6,1);
   memory.print(digits[2]);
   memory.setCursor(10,1);
   memory.print(digits[3]);
   memory.setCursor(16,1);
   memory.print(digits[4]);
}

// function that returns the postion of a digit
int positionOf(int nr)
{
  for (int i = 1; i < 5; i++)
    if (digits[i] == nr) return i;
}

// function that calculates the answer for the current stage
void stage(int nr) 
{
  if (nr == 1)
  {
    if (digits[0] == 1) rightPoz = 2;
    else if (digits[0] == 2) rightPoz = 2;
    else if (digits[0] == 3) rightPoz = 3;
    else if (digits[0] == 4) rightPoz = 4;
  }
  else if (nr == 2)
  {
    if (digits[0] == 1) rightPoz = positionOf(4);
    else if (digits[0] == 2) rightPoz = pos[1];
    else if (digits[0] == 3) rightPoz = 1;
    else if (digits[0] == 4) rightPoz = pos[1];
  }
  else if (nr == 3)
  {
    if (digits[0] == 1) rightPoz = positionOf(val[2]);
    else if (digits[0] == 2) rightPoz = positionOf(val[1]);
    else if (digits[0] == 3) rightPoz = 3;
    else if (digits[0] == 4) rightPoz = positionOf(4);
  }
  else if (nr == 4)
  {
    if (digits[0] == 1) rightPoz = pos[1];
    else if (digits[0] == 2) rightPoz = 1;
    else if (digits[0] == 3) rightPoz = pos[2];
    else if (digits[0] == 4) rightPoz = pos[2];
  }
  else if (nr == 5)
  {
    if (digits[0] == 1) rightPoz = positionOf(val[1]);
    else if (digits[0] == 2) rightPoz = positionOf(val[2]);
    else if (digits[0] == 3) rightPoz = positionOf(val[4]); //3
    else if (digits[0] == 4) rightPoz = positionOf(val[3]); //4
  }

  pos[nr] = rightPoz;
  val[nr] = digits[rightPoz];
}

void memorySetup()
{
  memory.begin(16, 2); // turn off power saving, enables display

memory.clear();
  randomSeed(analogRead(0));

  pinMode(PIN_MEMORY_LED_1, OUTPUT);
  pinMode(PIN_MEMORY_LED_2, OUTPUT);
  pinMode(PIN_MEMORY_LED_3, OUTPUT);
  pinMode(PIN_MEMORY_LED_4, OUTPUT);
  pinMode(PIN_MEMORY_LED_GREEN, OUTPUT);

  turnOffLeds();

  // stage 1
  generateNumbers();
  displayNumbers();
  stageNumber = 1;
}

void changeStage() // the next stage
{
  if (stageNumber == 1) digitalWrite(PIN_MEMORY_LED_1, HIGH);
  else if (stageNumber == 2) digitalWrite(PIN_MEMORY_LED_2, HIGH);
  else if (stageNumber == 3) digitalWrite(PIN_MEMORY_LED_3, HIGH);
  else if (stageNumber == 4) digitalWrite(PIN_MEMORY_LED_4, HIGH);

  generateNumbers();
  stageNumber++;
  displayNumbers();
  printAnswer = true;
}

// function that resets the module
void memoryReset() 
{
  addStrike();
  if (nrStrikes < 3)
  {
    stageNumber = 0;
    changeStage();
    turnOffLeds();
  }
}

// function that returns the pressed button
int memoryPressedButton() 
{
  previousButton = button;
  if (digitalRead(PIN_MEMORY_BUTTON_1) == 1) return 1;
  if (digitalRead(PIN_MEMORY_BUTTON_2) == 1) return 2;
  if (digitalRead(PIN_MEMORY_BUTTON_3) == 1) return 3;
  if (digitalRead(PIN_MEMORY_BUTTON_4) == 1) return 4;
  return 0;
}

// function that checks if a button is pressed
void memoryCheckButton() 
{
  button = memoryPressedButton();
  stage(stageNumber);
  if (printAnswer) // print the answer to the current stage
  {
    Serial.print("Memory Stage ");
    Serial.print(stageNumber);
    Serial.print(" : ");
    Serial.println(rightPoz);
    printAnswer = false;
  }

  if (button != 0 && previousButton != button)
  {
    if (button == rightPoz) // if the right button was pressed
    {
      if (stageNumber == 5) // if it's the last stage, the module is defused
      {
        turnOffLeds();
        digitalWrite(PIN_MEMORY_LED_GREEN, HIGH);
        memory.clear();
        memoryModuleDefused = true;
        defusedModuleBuzzer();
        stageNumber++;

        if (whoModuleDefused && simonModuleDefused && memoryModuleDefused && buttonModuleDefused)
        {
          victoryBuzzer();
          defused = true;
        }

      }
      else
        changeStage(); // else go to the next stage

    }
    else
      memoryReset(); // else reset the module
  }
}

// function that turns off everything but the green led
void memoryModuleBoom() 
{
  digitalWrite(PIN_MEMORY_LED_1, LOW);
  digitalWrite(PIN_MEMORY_LED_2, LOW);
  digitalWrite(PIN_MEMORY_LED_3, LOW);
  digitalWrite(PIN_MEMORY_LED_4, LOW);
  memory.clear();
}

void memoryLoop()
{
  if (!memoryModuleDefused)
    memoryCheckButton();

}

// Who's on First Module

#define LABELS_LENGTH 28

#define PIN_WHO_LED_1 3
#define PIN_WHO_LED_2 4
#define PIN_WHO_LED_3 5
#define PIN_WHO_LED_GREEN 6

#define PIN_WHO_BUTTON_1 A15
#define PIN_WHO_BUTTON_2 A14
#define PIN_WHO_BUTTON_3 A13
#define PIN_WHO_BUTTON_4 A12
#define PIN_WHO_BUTTON_5 A11
#define PIN_WHO_BUTTON_6 A10

hd44780_I2Cexp lcd(0x27);
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
char labels[LABELS_LENGTH][10] = { "YES", "FIRST", "DISPLAY", "OKAY", "SAYS", "NOTHING", "", "BLANK", "NO", "LED", "LEAD", "READ", "RED", "REED", "LEED", "HOLD ON", "YOU", "YOU ARE", "YOUR", "YOU'RE", "UR", "THERE", "THEY'RE", "THEIR", "THEY ARE", "SEE", "C", "CEE" };
char allLabels[LABELS_LENGTH][10] = { "YES", "BLANK", "NOTHING", "DONE", "LIKE", "FIRST", "NEXT", "HOLD", "UHHH", "UH UH", "UH HUH", "LEFT", "MIDDLE", "RIGHT", "OKAY", "PRESS", "NO", "READY", "SURE", "WAIT", "U", "UR", "YOU", "YOUR", "YOU'RE", "YOU ARE", "WHAT", "WHAT?"  };

int indexOfButtonLabels[LABELS_LENGTH][15] = {{ 14, 13, 8, 12, 5, 26, 15, 17, 2, 0}, // the words for the first label, 0 is the first label from allLabels[]
                                              { 19, 13, 14, 12, 1}, // the words for the second label
                                              { 8, 13, 14, 12, 0, 1, 16, 15, 11, 26, 19, 5, 2},
                                              { 18, 10, 6, 27, 23, 21, 24, 7, 4, 22, 20, 25, 9, 3},
                                              { 24, 6, 20, 21, 7, 3, 9, 27, 10, 22, 4},
                                              { 11, 14, 0, 12, 16, 13, 2, 8, 19, 17, 1, 26, 15, 5},
                                              { 27, 10, 9, 23, 7, 18, 6},
                                              { 25, 20, 3, 9, 22, 21, 18, 27, 24, 6, 7},
                                              { 17, 2, 11, 26, 14, 0, 13, 16, 15, 1, 8},
                                              { 21, 20, 25, 24, 6, 9},
                                              { 10},
                                              { 13, 11},
                                              { 1, 17, 14, 26, 2, 15, 16, 19, 11, 12},
                                              { 0, 2, 17, 15, 16, 19, 26, 13},
                                              { 12, 16, 5, 0, 8, 2, 19, 14},
                                              { 13, 12, 0, 17, 15},
                                              { 1, 8, 19, 5, 26, 17, 13, 0, 2, 11, 15, 14, 16},
                                              { 0, 14, 26, 12, 11, 15, 13, 1, 17},
                                              { 25, 3, 4, 24, 22, 7, 10, 21, 18},
                                              { 8, 16, 1, 14, 0, 11, 5, 15, 26, 19},
                                              { 10, 18, 6, 27, 24, 21, 9, 3, 20},
                                              { 3, 20, 21},
                                              { 18, 25, 23, 24, 6, 10, 21, 7, 27, 22},
                                              { 9, 25, 10, 23},
                                              { 22, 24},
                                              { 23, 6, 4, 10, 27, 3, 9, 7, 22, 20, 24, 18, 21, 25},
                                              { 8, 26},
                                              {  22, 7, 24, 23, 20, 3, 9, 4, 25, 10, 21, 6, 27}
                                            };


char words[7][10];
int correctButton, correctLabel, whoLevel = 1;
bool checkLabel[30];

int currentButton, prevButton;

void whoModuleBoom() // the bomb explodes
{
  digitalWrite(PIN_WHO_LED_1, LOW);
  digitalWrite(PIN_WHO_LED_2, LOW);
  digitalWrite(PIN_WHO_LED_3, LOW);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("====================");
  lcd.setCursor(0, 3); lcd.print("====================");
  lcd.setCursor(9, 1); lcd.print("BOMB");
  lcd.setCursor(7, 2); lcd.print("EXPLODED");

}

void whoModuleDefusedPrint() // the module has been defused
{
  digitalWrite(PIN_WHO_LED_1, LOW);
  digitalWrite(PIN_WHO_LED_2, LOW);
  digitalWrite(PIN_WHO_LED_3, LOW);
  digitalWrite(PIN_WHO_LED_GREEN, HIGH);
  defusedModuleBuzzer();
  whoModuleDefused = true;
  if (whoModuleDefused && simonModuleDefused && memoryModuleDefused && buttonModuleDefused)
  {
    victoryBuzzer();
    defused = true;
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("====================");
  lcd.setCursor(0, 3); lcd.print("====================");
  lcd.setCursor(7, 1); lcd.print("MODULE");
  lcd.setCursor(7, 2); lcd.print("DEFUSED");
}

// function that displays the labels
void printLabels() 
{
  lcd.setCursor((20 - strlen(words[0])) / 2, 0);
  lcd.print(words[0]);

  for (int i = 1; i <= 3; i++)
  {
    lcd.setCursor(0, i);
    lcd.print(words[i]);
    lcd.setCursor(20 - strlen(words[3 + i]), i);
    lcd.print(words[3 + i]);
  }

}

// function that returns the index of the label you have to read, depending on the first label
int findCorrectLabel(int nr) 
{
  switch (nr) {
    case 0: return 2;
    case 1: return 4;
    case 2: return 6;
    case 3: return 4;
    case 4: return 6;
    case 5: return 2;
    case 6: return 3;
    case 7: return 5;
    case 8: return 6;
    case 9: return 2;
    case 10: return 6;
    case 11: return 5;
    case 12: return 5;
    case 13: return 3;
    case 14: return 3;
    case 15: return 6;
    case 16: return 5;
    case 17: return 6;
    case 18: return 5;
    case 19: return 5;
    case 20: return 1;
    case 21: return 6;
    case 22: return 3;
    case 23: return 5;
    case 24: return 2;
    case 25: return 6;
    case 26: return 4;
    case 27: return 6;
  }
}

// function that returns the index of a word from allLabels[]
int indexOfLabel(char word[10]) 
{
  for (int i = 0; i < LABELS_LENGTH; i++)
    if (strcmp(word, allLabels[i]) == 0) return i;
}

// function that returns the index of a button
int indexOfButton(int index) 
{
  for (int i = 1; i <= 6; i++)
    if (strcmp(words[i], allLabels[index]) == 0) return i;
}

// function that returns the right button
int findCorrectButton(int index)
{
  int k = 0;
  while (checkLabel[indexOfButtonLabels[index][k]] == 0) k++;
  return indexOfButton(indexOfButtonLabels[index][k]);
}

// function that generates the words of the current stage
void generateWords() 
{
  int number = random(LABELS_LENGTH);
  for (int i = 0; i < 100; i++) number = random(LABELS_LENGTH);

  strcpy(words[0], labels[number]); // the first label
  correctLabel = findCorrectLabel(number);

  for (int i = 0; i < LABELS_LENGTH; i++) checkLabel[i] = 0; // we need this array so the words won't repeat

  for (int i = 1; i <= 6; i++)
  {
    int number = random(LABELS_LENGTH);
    while (checkLabel[number] == 1) number = random(LABELS_LENGTH); // if the words repeat generate another one
    checkLabel[number] = 1;
    strcpy(words[i], allLabels[number]);
  }

  int index = indexOfLabel(words[correctLabel]);
  correctButton = findCorrectButton(index);

  // print the answer to the current stage
  Serial.print("Who's on First Stage ");
  Serial.print(whoLevel);
  Serial.print(" : ");
  Serial.println(correctButton);

}

void nextLevel()
{
  whoLevel++;
  if (whoLevel == 2) digitalWrite(PIN_WHO_LED_1, HIGH);
  else if (whoLevel == 3) digitalWrite(PIN_WHO_LED_2, HIGH);
  else if (whoLevel == 4) digitalWrite(PIN_WHO_LED_3, HIGH);
  else if (whoLevel == 5) whoModuleDefusedPrint();

  if (whoLevel != 5) // if the module hasn't been defused yet, go to the next stage
  {
    lcd.clear();
    generateWords();
    printLabels();
  }
}

// function that resets the module
void whoReset() 
{
  addStrike();
  if (nrStrikes < 3)
  {
    whoLevel = 1;
    digitalWrite(PIN_WHO_LED_1, LOW);
    digitalWrite(PIN_WHO_LED_2, LOW);
    digitalWrite(PIN_WHO_LED_3, LOW);
    lcd.clear();
    generateWords();
    printLabels();
  }
}

void whoSetup()
{
lcd.begin(LCD_COLS, LCD_ROWS);
  // first stage
  generateWords();
  printLabels();

  pinMode(PIN_WHO_LED_GREEN, OUTPUT);
  pinMode(PIN_WHO_LED_1, OUTPUT);
  pinMode(PIN_WHO_LED_2, OUTPUT);
  pinMode(PIN_WHO_LED_3, OUTPUT);

  pinMode(PIN_WHO_BUTTON_1, INPUT);
  pinMode(PIN_WHO_BUTTON_2, INPUT);
  pinMode(PIN_WHO_BUTTON_3, INPUT);
  pinMode(PIN_WHO_BUTTON_4, INPUT);
  pinMode(PIN_WHO_BUTTON_5, INPUT);
  pinMode(PIN_WHO_BUTTON_6, INPUT);
}

// function that returns the pressed button
int whoPressedButton() 
{
  prevButton = currentButton;
  if (digitalRead(PIN_WHO_BUTTON_1) == 1) return 1;
  if (digitalRead(PIN_WHO_BUTTON_2) == 1) return 2;
  if (digitalRead(PIN_WHO_BUTTON_3) == 1) return 3;
  if (digitalRead(PIN_WHO_BUTTON_4) == 1) return 4;
  if (digitalRead(PIN_WHO_BUTTON_5) == 1) return 5;
  if (digitalRead(PIN_WHO_BUTTON_6) == 1) return 6;
  return 0;
}

// function that checks if a button is pressed
void whoCheckButton() 
{
  currentButton = whoPressedButton();
  if (currentButton != prevButton && currentButton != 0)
  {
    if (currentButton == correctButton) nextLevel(); // if you pressed the right answer, go to the next stage
    else whoReset();
  }
}

void whoLoop()
{
  if (!whoModuleDefused)
    whoCheckButton();
}
// The Timer

#define PIN_CLK 14
#define PIN_DIO 15
hd44780_I2Cexp timer(0x26);
unsigned long seconds = 0;
int mins = TIMER_MINUTES, sec = TIMER_SECONDS + 1;
int secb;
void timeSetup() {
  int secb;
  timer.begin(16,2);            // initializes the display
  timer.setBacklight(100);  // set the brightness to 100%
};

// function that displays the time on the clock
void displayTime() 
{
  if (seconds < millis())
  {
    seconds += 1000;
    sec--;
   
    if (sec == -1)
    {
      mins--;
      if (mins == -1)  bombExploded(); // if the time hits zero, the bomb will go off
      else {
      sec = 59;
      
      }
    }

  }
}

void timeLoop() {

  // defused : time left
  // exploded from time : 00:00
  // exploded from strikes : exploded
timer.clear();
  timer.print(mins);
  timer.print(":");
  timer.print(sec);
  if (exploded && explodedFromStrikes)
    timer.print("    BOMB EXPLODED    ");
  else if (!defused && !(exploded && !explodedFromStrikes))
    displayTime();
 
      secb = (sec % 2);
    if (secb == 0) {
    tone(BUZZER, 660, 1);
    }
}
//The Button Module
/* 
I know there is a lot of repetitive code inside this but when I tried to
refactor it, everything stopped working so I decided to keep it like this
*/

#define LCD_BUTTON_CONTRAST 40
#define PIN_THE_BUTTON_LED_GREEN 19
#define BTN_PIN 26
#define BUZZER_PIN 6
#define LEFT_LED_RED_PIN 7
#define LEFT_LED_GREEN_PIN 8
#define LEFT_LED_BLUE_PIN 9
#define RIGHT_LED_RED_PIN 10
#define RIGHT_LED_GREEN_PIN 11
#define RIGHT_LED_BLUE_PIN 12
#define V0_PIN 13

hd44780_I2Cexp lcdButton(0x25);

int btnState = LOW;
int lastBtnState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelayButton = 50;
int moduleFinished = 0;
int leftLedColor;
int rightLedColor;
int btnWordGen;

void setColor (int redPin, int redValue, int greenPin, int greenValue, int bluePin, int blueValue) 
{
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

void buttonModuleDefusedPrint()
{ 
   setColor(LEFT_LED_RED_PIN, 0, LEFT_LED_GREEN_PIN, 0, LEFT_LED_BLUE_PIN, 0);
   setColor(RIGHT_LED_RED_PIN, 0, RIGHT_LED_GREEN_PIN, 0, RIGHT_LED_BLUE_PIN, 0);
   buttonModuleDefused = true;
   if(whoModuleDefused && simonModuleDefused && memoryModuleDefused && buttonModuleDefused) 
    {
      victoryBuzzer();
      defused = true;
    }
}

void buttonModuleBoom()
{ 
  setColor(LEFT_LED_RED_PIN, 0, LEFT_LED_GREEN_PIN, 0, LEFT_LED_BLUE_PIN, 0);
  setColor(RIGHT_LED_RED_PIN, 0, RIGHT_LED_GREEN_PIN, 0, RIGHT_LED_BLUE_PIN, 0);
  lcdButton.clear();
  lcdButton.setCursor(6, 0);
  lcdButton.print("BOMB");
  lcdButton.setCursor(4, 1);
  lcdButton.print("EXPLODED"); 
}

bool checkClock(int value)
{
  int timer = sec;
  while(timer >0)
  {
    if(timer % 10 == value)
    return 1;
    timer = timer / 10;
  }

  timer = mins;
  while(timer >0)
  {
    if(timer % 10 == value)
    return 1;
    timer = timer / 10;
  }
  
  return 0;
}

void printWord() {
  switch(btnWordGen)
  {
    case 1: 
    { //HOLD
      lcdButton.setCursor(5, 0);
      lcdButton.print("HOLD");
    }
    break;
    case 2: 
    { //DETONATE
      lcdButton.setCursor(4, 0);
      lcdButton.print("DETONATE");
    }
    break;
    case 3: 
    { //ABORT
      lcdButton.setCursor(5, 0);
      lcdButton.print("ABORT");
    }
    break;
  }
}

void printModuleDefused() 
{
  buttonModuleDefusedPrint();
  lcdButton.clear();
  lcdButton.setCursor(1, 0);
  lcdButton.print("MODULE DEFUSED");
  lcdButton.setCursor(0, 1);
  lcdButton.print("Serial: ");
  lcdButton.print(serialCode);
}

void buttonSetup()
{
  pinMode(LEFT_LED_RED_PIN, OUTPUT);
  pinMode(LEFT_LED_GREEN_PIN, OUTPUT);
  pinMode(LEFT_LED_BLUE_PIN, OUTPUT);
  pinMode(RIGHT_LED_RED_PIN, OUTPUT);
  pinMode(RIGHT_LED_GREEN_PIN, OUTPUT);
  pinMode(RIGHT_LED_BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);
  pinMode(PIN_THE_BUTTON_LED_GREEN, OUTPUT);
  pinMode(V0_PIN, OUTPUT);

  analogWrite(V0_PIN, LCD_BUTTON_CONTRAST);
  lcdButton.begin(16, 2);

  //generating a seed to use in order to generate random numbers
  randomSeed(analogRead(0));

  //generating the colors for the left led and the right led
  leftLedColor = random(1, 5);
  rightLedColor = random(1, 5);
  btnWordGen = random(1, 4);
 
  switch(leftLedColor)
  {
    case 1: {
      setColor(LEFT_LED_RED_PIN, 0, LEFT_LED_GREEN_PIN, 0, LEFT_LED_BLUE_PIN, 255);
    }
    break;
    case 2: {
      setColor(LEFT_LED_RED_PIN, 255, LEFT_LED_GREEN_PIN, 0, LEFT_LED_BLUE_PIN, 0);
    }
    break;
    case 3: {
      setColor(LEFT_LED_RED_PIN, 0, LEFT_LED_GREEN_PIN, 255,LEFT_LED_BLUE_PIN, 0);
    }
    break;
    case 4: {
      setColor(LEFT_LED_RED_PIN, 255, LEFT_LED_GREEN_PIN, 255, LEFT_LED_BLUE_PIN, 255);
    }
    break;
  }
  switch(rightLedColor){
    case 1: {
      setColor(RIGHT_LED_RED_PIN, 0, RIGHT_LED_GREEN_PIN, 0, RIGHT_LED_BLUE_PIN, 255);
    }
    break;
    case 2: {
      setColor(RIGHT_LED_RED_PIN, 255, RIGHT_LED_GREEN_PIN, 0, RIGHT_LED_BLUE_PIN, 0);
    }
    break;
    case 3: {
      setColor(RIGHT_LED_RED_PIN, 0, RIGHT_LED_GREEN_PIN, 255, RIGHT_LED_BLUE_PIN, 0);
    }
    break;
    case 4: {
      setColor(RIGHT_LED_RED_PIN, 255, RIGHT_LED_GREEN_PIN, 255, RIGHT_LED_BLUE_PIN, 255);
    }
    break;
  }

  printWord();
  lcdButton.setCursor(0, 1);
  lcdButton.print("Serial: ");
  lcdButton.print(serialCode);
  
  Serial.println("Left led color: ");
  Serial.println(leftLedColor);
  Serial.println("Right led color: ");
  Serial.println(rightLedColor);
  Serial.println("Word: ");
  Serial.println(btnWordGen);
}


void buttonLoop()
{ 
  if(!buttonModuleDefused) 
  {
  int reading = digitalRead(BTN_PIN);
  
  if(reading != lastBtnState)
  lastDebounceTime = millis();
  
  switch(leftLedColor)
  {
    case 1: {// left led is blue
      if(btnWordGen == 3 && seconds > 0 && !moduleFinished){ //Abort written on the screen
        // press and imediately release
        if(millis() - lastDebounceTime > debounceDelayButton)
          {
            if(reading != btnState)
            {
              btnState = reading;
              if(btnState == HIGH)
              {
                moduleFinished = 1;
                digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                Serial.println("Finished: Left led is blue + Abort");
                defusedModuleBuzzer();
                printModuleDefused();
              }
            }
          }
      }
      else if (seconds > 0 && !moduleFinished) //hold button
        switch(rightLedColor)
        {
          case 1: 
          {// right led is blue
            // release when timer has a 4 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(4))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is blue + Right led is blue, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                  else if(btnState == LOW && !checkClock(4))
                  {
                    addStrike();
                  }
                }
            }
          }
          break;
          case 2: 
          {// right led is red
            // release when timer has a 3 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(3))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is blue + Right led is red, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(3))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 3: 
          {// Right led is green
            // release when timer has a 5 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(5))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is blue + Right led is green, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(5))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 4: 
          {// Right led is white
            // release when timer has a 1 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(1))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is blue + Right led is white, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(1))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
        }
    }
    break;
    case 2:
    {// Left led is red
      if(btnWordGen == 1 && seconds > 0 && !moduleFinished) // Hold is written on the screen
      {
        // press and imediately release
        if(millis() - lastDebounceTime > debounceDelayButton)
          {
            if(reading != btnState)
            {
              btnState = reading;
              if(btnState == HIGH)
              {
                moduleFinished = 1;
                digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                Serial.println("Finished: Left led is red + Hold");
                defusedModuleBuzzer();
                printModuleDefused();
              }
            }
          }
      }
      else if(seconds > 0 && !moduleFinished) //hold button
        switch(rightLedColor)
        {
          case 1: 
          {// Right led is blue
            // release when timer has a 4 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(4))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is red + Right led is blue, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(4))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 2: 
          {// Right led is red
            // release when timer has a 3 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(3))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is red + Right led is red, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(3))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 3: 
          {// Right led is green
            // release when timer has a 5 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(5))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is red + Right led is green, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(5))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 4: 
          {// Right Led is white
            //release when timer has a 1 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(1))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left Led is red + Right led is white, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(1))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
        }
    }
    break;
    case 3:
    {// Left led is green
      if(btnWordGen == 2 && seconds > 0 && !moduleFinished) // Detonate written on the screen
      {
        // press and imediately release
        if(millis() - lastDebounceTime > debounceDelayButton)
          {
            if(reading != btnState)
            {
              btnState = reading;
              if(btnState == HIGH)
              {
                moduleFinished = 1;
                digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                Serial.println("Finished: Left led is green + Detonate");
                defusedModuleBuzzer();
                printModuleDefused();
              }
            }
          }
      }
      else if(seconds > 0 && !moduleFinished) //hold button
        switch(rightLedColor)
        {
          case 1: 
          {// Right led is blue
            // release when timer has a 4 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(4))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is green + Right led is blue, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(4))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 2: 
          {// Right led is red
            // release when timer has a 3 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(3))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is green + Right led is red, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(3))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 3: 
          {// Right led is green
            // release when timer has a 5 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(5))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is green + Right led is green, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(5))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
          case 4: 
          {// Right led is white
            // release when timer has a 1 in any position
            if(millis() - lastDebounceTime > debounceDelayButton)
            {
                if(reading != btnState)
                {
                  btnState = reading;
                  if(btnState == LOW && checkClock(1))
                  {
                      moduleFinished = 1;
                      digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                      Serial.println("Finished: Left led is green + Right led is white, button has been held");
                      defusedModuleBuzzer();
                      printModuleDefused();
                  }
                    else if(btnState == LOW && !checkClock(1))
                    {
                      addStrike();
                    }
                }
            }
          }
          break;
        }
    }
    break;
    case 4:
    {// Left led is white
        if(seconds > 0 && !moduleFinished)
          switch(rightLedColor)
          {
            case 1: 
            {// Right led is blue
              // release when timer has a 4 in any position
              if(millis() - lastDebounceTime > debounceDelayButton)
              {
                  if(reading != btnState)
                  {
                    btnState = reading;
                    if(btnState == LOW && checkClock(4))
                    {
                        moduleFinished = 1;
                        digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                        Serial.println("Finished: Left led is white + Right led is blue, button has been held");
                        defusedModuleBuzzer();
                        printModuleDefused();
                    }
                    else if(btnState == LOW && !checkClock(4))
                    {
                      addStrike();
                    }
                  }
              }
            }
            break;
            case 2: 
            {// Right led is red
              // release when timer has a 3 in any position
              if(millis() - lastDebounceTime > debounceDelayButton)
              {
                  if(reading != btnState)
                  {
                    btnState = reading;
                    if(btnState == LOW && checkClock(3))
                    {
                        moduleFinished = 1;
                        digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                        Serial.println("Finished: Left led is white + Right led is red, button has been held");
                        defusedModuleBuzzer();
                        printModuleDefused();
                    }
                    else if(btnState == LOW && !checkClock(3))
                    {
                      addStrike();
                    }
                  }
              }
            }
            break;
            case 3: 
            {// Right led is green
              // release when timer has a 5 in any position
              if(millis() - lastDebounceTime > debounceDelayButton)
              {
                  if(reading != btnState)
                  {
                    btnState = reading;
                    if(btnState == LOW && checkClock(5))
                    {
                        moduleFinished = 1;
                        digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                        Serial.println("Finished: Left led is white + Right led is green, button has been held");
                        defusedModuleBuzzer();
                        printModuleDefused();
                    }
                    else if(btnState == LOW && !checkClock(5))
                    {
                      addStrike();
                    }
                  }
              }
            }
            break;
            case 4: 
            {// Right led is white
              // release when timer has a 1 in any position
              if(millis() - lastDebounceTime > debounceDelayButton)
              {
                  if(reading != btnState)
                  {
                    btnState = reading;
                    if(btnState == LOW && checkClock(1))
                    {
                        moduleFinished = 1;
                        digitalWrite(PIN_THE_BUTTON_LED_GREEN, HIGH);
                        Serial.println("Finished: Left led is white + Right led is white, button has been held");
                        defusedModuleBuzzer();
                        printModuleDefused();
                    }
                    else if(btnState == LOW && !checkClock(1))
                    {
                      addStrike();
                    }
                  }
              }
            }
            break;
          }
    }
    break;
  }
  
  lastBtnState = reading;
  }
}
/*char *correct_str;
char possible_letters[6][5];
char *possible_words[35] = {
  "ABOUT", "AFTER", "AGAIN", "BELOW", "COULD",
  "EVERY", "FIRST", "FOUND", "GREAT", "HOUSE",
  "LARGE", "LEARN", "NEVER", "OTHER", "PLACE",
  "PLANT", "POINT", "RIGHT", "SMALL", "SOUND",
  "SPELL", "STILL", "STUDY", "THEIR", "THERE",
  "THESE", "THING", "THINK", "THREE", "WATER",
  "WHERE", "WHICH", "WORLD", "WOULD", "WRITE"
};

void dispStr(char *str) {
  u8g2.firstPage();
  do {

  } while(u8g2.nextPage());
}

void generateGrid(char* word) {
  int position, temp, letter_in_word;
  int num_possible = 35;

  while(num_possible != 1){
    Serial.println("Attempting word generation.");
    for(int i = 0; i < 5; i++) {
      char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      position = random(6);
      alphabet[word[i] - 'A'] = 0;
      for(int j = 0; j < 6; j++) {
        if (position == j) {
          possible_letters[j][i] = word[i];
        } else {
          do {
            temp = random(26);
            possible_letters[j][i] = 'A' + temp;
          } while (alphabet[temp] == 0);
          alphabet[temp] = 0; 
        }
      }
    }

    // check for more than one possible word
    uint8_t word_checklist[35] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    num_possible = 35;
    for(int i = 0; i < 5; i++) {
      for(int word_idx = 0; word_idx < 35; word_idx++) {
        if(word_checklist[word_idx] == 1){
          letter_in_word = 0;
          for(int j = 0; j < 6; j++) {
            if(possible_letters[j][i] == possible_words[word_idx][i]){
              letter_in_word = 1;
            }
          }
          word_checklist[word_idx] = letter_in_word;
          num_possible -= letter_in_word;
        }
      }
      if(num_possible == 1) {
        break;
      }
    }
  }
}

void setuppass() {
  serial_port.begin(19200);
  Serial.begin(19200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_inb27_mf);

  // while(!module.getConfig()){
  //   module.interpretData();
  // }

  randomSeed(config_to_seed(module.getConfig()));
  int word_idx = random(35);
  correct_str = possible_words[word_idx];
  Serial.println(correct_str);
  generateGrid(correct_str);

  // module.sendReady();
}

void looppass() {
  // module.interpretData();
  delay(10);
  dispStr(correct_str);
  if(!module.is_solved){
    /*
    checkInputs();
    if(they_solved_it) {
      module.win();
    }
    if(they_messed_up) {
      module.strike();
    }
    updateOutputs();
    
  }
}*/

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));

  pinMode(PIN_STRIKE_1, OUTPUT);
  pinMode(PIN_STRIKE_2, OUTPUT);
  pinMode(PIN_STRIKE_3, OUTPUT);

  digitalWrite(PIN_STRIKE_1, LOW);
  digitalWrite(PIN_STRIKE_2, LOW);
  digitalWrite(PIN_STRIKE_3, LOW);
  
  generateSerialCode();
  timeSetup();
  whoSetup();
  memorySetup();
  buttonSetup();
// setuppass();
  defusedSimonSetup();
}
void bombExploded()
{
  exploded = true;
  whoModuleBoom();
 memoryModuleBoom();
  buttonModuleBoom();
//  looppass();
  boomBuzzer();
}
void addStrike() 
{
  nrStrikes++;
  strikeBuzzer();

  if (nrStrikes == 1) digitalWrite(PIN_STRIKE_1, HIGH);
  else if (nrStrikes == 2) digitalWrite(PIN_STRIKE_2, HIGH);
  else if (nrStrikes == 3) // the maximum number of strikes is reached
  {
    digitalWrite(PIN_STRIKE_3, HIGH);
    explodedFromStrikes = true;
    bombExploded();
  }
}
void loop() {

  timeLoop();
  
  if (!defused && !exploded)
  {
    whoLoop();
//    memoryLoop();
//    buttonLoop();
  }


}
