/*
 * Author: Deandre Baker
 * Description: Source code for the Bright Brain project
 */

// Includes the SPI, Adafruit GFX, and Adafruit RA8875 libraries
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

// Survey questions and facts
const char* depressionQ[] = {"How often do you have little interest or pleasure in doing things?", "How often do you feel down, depressed, or hopeless?", "How often do you have trouble with sleep?", "How often do you feel tired or have low energy?", "How often do you have a poor appetite or you overeat?", "How often do you feel bad about yourself or your failures?", "How often do you have slow speech or movements?", "How often do you have thoughts of self-harm or suicide?", "Over the last 2 weeks, how often have you been bothered by any of the previous problems?"};
const char* bipolarQ[] = {"Have you ever felt so much pride that others thought you were not your normal self or you got into trouble?", "Were so irritable that you shouted at people or started fights or arguments?", "Did you feel much more self-confident than usual?", "Did you get much less sleep than usual and found you did not really miss it?", "Were much more talkative or spoke much faster than usual?", "Did you have thoughts race through your head or you could not slow your mind down?", "Were so easily distracted by things around you that you had trouble concentrating?", "Did you have much more energy than usual?", "Were much more social or outgoing than usual and bothered others excessively?", "Have you done excessively foolish or risky things that were unusual for you to do?", "Did spending money get you or your family into trouble?", "Have several of the statements above ever happened during the same period of time?", "Did any of the statements above cause a lot of problems for you?"};
const char* anxietyQ[] = {"How often do you feel nervous, anxious, or on edge?", "How often are you able to fall asleep or control worrying", "How often do you worry too much about different things", "How often do you have trouble relaxing", "How often do you feel restless that it is hard to sit still", "How often do become easily annoyed or irritable", "How often do you feel afraid, as if something awful might happen"};
const char* facts[] = {"1 of 5 people in Canada will personally experience a mental health problem or illness.", "Suicide accounts for 24% of all deaths among 15-24 year olds.", "Suicide is one of the leading causes of death from adolescence to middle age.", "Almost 50% of those who have suffered from depression or anxiety have never gone to see a doctor.", "Stigma or discrimination prevent those who are suffering from getting treatment.", "It is estimated that 10-20% of Canadian youth are affected by a mental illness.", "The total number of 12-19 year olds in Canada at risk for developing depression is almost 3.2 million.", "Approximately 2.5% of Canadians suffer from bipolar I disorder and up to 5% suffer from bipolar II disorder."};

char* question = "";
int score;
String testType;
bool factsDisplayed = false;
unsigned long idleTime = 0;
int factNumber = 0;

// Pins
int redLed = 24;
int yellowLed = 23;
int blueLed = 22;
int greenLed = 25;
int pump = 26;

// Connect SCLK to UNO Digital #52 (Hardware SPI clock)
// Connect MISO to UNO Digital #50 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #51 (Hardware SPI MOSI)
#define RA8875_INT 44
#define RA8875_CS 2
#define RA8875_RESET 3

// Initializes the tft touchscreen display
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;
float xScale;
float yScale;

// The "Button" class represents a rectangular button object on the touchscreen
class Button
{
  public:
    int x;
    int y;
    int s;
    int h;
    int colour;

  // Button constructor
  Button(){}
  Button(int Colour, int xValue, int yValue, int sideLength)
  {
    x = xValue;
    y = yValue;
    s = sideLength;
    h = sideLength/2;
    colour = Colour;
    
    tft.graphicsMode();
    tft.fillRect(x-h, y-h, s, s, colour);
  }

  // the "pressed" function returns true if the button is being pressed and false if not
  bool pressed(uint16_t tx, uint16_t ty)
  {
    if(((x-h) <= (tx/xScale)) && ((x+h) >= (tx/xScale)) && ((y-h) <= (ty/yScale)) && ((y+h) >= (ty/yScale)))
      return true;
    else
      return false;
  }
};

void setup() 
{
  // Initializes pins 22 through 26 as output pins
  for(int i = 22; i <= 26; i++)
  {
    pinMode(i, OUTPUT);
  }
  
  Serial.begin(9600);
  Serial.println("RA8875 start");

  // Halts program if the RA8875 is not found
  if(!tft.begin(RA8875_800x480))
  {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  Serial.println("Found RA8875");

  // Sets the display scaling variables
  xScale = 1024.0F/tft.width();
  yScale = 1024.0F/tft.height();

  // Sets up the tft display
  tft.displayOn(true);
  tft.GPIOX(true);
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024);
  tft.PWM1out(255);
  
}

void loop() 
{
  startUp();
  modeSelect();
  testResults();
}

// Runs the start program
void startUp()
{
  setBackground(RA8875_CYAN);
    
  setText(RA8875_BLUE, 3, 220, 200);
  tft.textWrite("BRIGHTBRAIN");
  tft.textEnlarge(2);
  tft.textSetCursor(140, 275);
  tft.textWrite("Press anywhere to start");
  delay(500);
  
  tft.graphicsMode();
  tft.touchEnable(true);

  // Exits the while loop and exits the startUP function once the screen is touched
  tft.touchRead(&tx, &ty);
  while(!tft.touched()){}
  tft.touchRead(&tx, &ty);
  
  delay(100);
}

void modeSelect()
{
  modeSelectBegin:
  
  factsDisplayed = false;
  setBackground(RA8875_CYAN);

  // Displays 3 buttons that represent a different mental illness diagnosis test
  Button blueButton = Button(RA8875_BLUE, 175, 300, 200); 
  Button redButton = Button(RA8875_RED, 400, 300, 200); 
  Button yellowButton = Button(RA8875_YELLOW, 625, 300, 200); 
  
  setText(RA8875_BLUE, 2, 125, 120);
  tft.textWrite("Select a Diagnosis Test");
  setText(RA8875_WHITE, 1, 100, 280);
  tft.textWrite("Depression");
  tft.textSetCursor(345, 280);
  tft.textWrite("Anxiety");
  setText(RA8875_BLACK, 1, 570, 280);
  tft.textWrite("Bipolar");
  tft.textSetCursor(15, 420);
  tft.textWrite("*Diagnosis is not a professional opinion");
  delay(500);

  tft.graphicsMode();
  
  touchRead:
  
  tft.touchRead(&tx, &ty);

  // Displays interesting facts if the screen is left untouched for a length of time
  while(!tft.touched())
  {
    Serial.println(idleTime);
    if (idleTime >= (10 * 20))
    {
      displayFacts();
      factsDisplayed = true;
    }  
    delay(100);
    idleTime++;
  }
  idleTime = 0;

  if(factsDisplayed == true)
  {
    startUp();
    goto modeSelectBegin;
  }

  // Starts the depression survey
  tft.touchRead(&tx, &ty);
  if(blueButton.pressed(tx, ty))
  {  
    tft.fillScreen(RA8875_BLUE);
    setText(RA8875_WHITE, 2, 50, 150);
    tft.textWrite("You chose the depression test!");
    tft.textSetCursor(50, 250);
    largeText("Choose the answer that applies best to you", 2);
    tft.textSetCursor(50, 400);
    largeText("Questions from Mental Health America", 1);
    delay(5000);
    diagnosisTest(depressionQ, sizeof(depressionQ)/sizeof(int));
  }

  // Starts the anxiety survey
  else if(redButton.pressed(tx, ty))
  {
    tft.fillScreen(RA8875_RED);
    setText(RA8875_WHITE, 2, 50, 150);
    tft.textWrite("You chose the anxiety test!");
    tft.textSetCursor(50, 250);
    largeText("Choose the answer that applies best to you", 2);
    tft.textSetCursor(50, 400);
    largeText("Questions from Mental Health America", 1);
    delay(5000);
    diagnosisTest(anxietyQ, sizeof(anxietyQ)/sizeof(int)); 
  }

  // Starts the bipolar survey
  else if(yellowButton.pressed(tx, ty))
  {
    tft.fillScreen(RA8875_YELLOW);
    setText(RA8875_BLACK, 2, 50, 150);
    tft.textWrite("You chose the bipolar test!");
    tft.textSetCursor(50, 250);
    largeText("Choose the answer that applies best to you", 2);
    tft.textSetCursor(50, 400);
    largeText("Questions from Mental Health America", 1);
    delay(5000);
    diagnosisTest(bipolarQ, sizeof(bipolarQ)/sizeof(int));
  }

  else
    goto touchRead;  
}

// The "diagnosisTest" function runs the survey passed in through the parameters
void diagnosisTest(char* test[], int testSize)
{
  score = 0;

  // Creates 4 input button objects
  Button buttonOne = Button();
  Button buttonTwo = Button();
  Button buttonThree = Button();
  Button buttonFour = Button();
  
  for(int i = 0; i < (testSize); i++)
  {
    if(test == depressionQ || test == anxietyQ)
    {
      setBackground((test == depressionQ) ? RA8875_BLUE : RA8875_RED);
      testType = (test == depressionQ) ? "depression" : "anxiety";
      Button one = Button(RA8875_BLUE, 100, 300, 190);
      Button two = Button(RA8875_RED, 300, 300, 190);
      Button three = Button(RA8875_YELLOW, 500, 300,190);
      Button four = Button(RA8875_GREEN, 700, 300, 190);
      
      setText(RA8875_WHITE, 1, one.x-40, one.y-25);
      tft.textWrite("Never");
      setText(RA8875_WHITE, 1, two.x-75, two.y-25);
      tft.textWrite("Sometimes");
      setText(RA8875_BLACK, 1, three.x-40, three.y-25);
      tft.textWrite("Often");
      setText(RA8875_BLACK, 1, four.x-45, four.y-25);
      tft.textWrite("Always");
      
      buttonOne = one;
      buttonTwo = two;
      buttonThree = three;
      buttonFour = four;
    }
    
    else if(test == bipolarQ)
    {
      setBackground(RA8875_YELLOW);
      testType = "bipolar";
      Button two = Button(RA8875_GREEN, 250, 300, 190);
      Button one = Button(RA8875_RED, 550, 300, 190);
      setText(RA8875_BLACK, 1, two.x-20, two.y-25);
      tft.textWrite("Yes");
      setText(RA8875_WHITE, 1, one.x-15, one.y-25);
      tft.textWrite("No");
      buttonOne = one;
      buttonTwo = two;
    }

    // Displays the question on the screen
    setText(RA8875_BLACK, 1, 0, 100);
    question = test[i];
    largeText(question, 1);

    // Configures the screen to be able to read touch inputs
    tft.graphicsMode();
    delay(900);
    
    touchRead:

    // Adjusts score based on which button is pressed
    tft.touchRead(&tx, &ty);
    while(!tft.touched())
    {}
    tft.touchRead(&tx, &ty);
    
    if(buttonOne.pressed(tx, ty))
      score += 0;

    else if(buttonTwo.pressed(tx, ty))
      score += 1;
    
    else if(buttonThree.pressed(tx, ty))
      score += 2;
        
    else if(buttonFour.pressed(tx, ty))
      score += 3;

    else
      goto touchRead;
    
    delay(100);
  }
}

// The testResult function displays the diagnosis results to the screen
void testResults()
{
  String result = "";
  
  if(testType == "depression")
  {
    setBackground(RA8875_BLUE);
    if (score >= 1 && score <= 4) 
    {
      result = "You have Minimal Depression";
      digitalWrite(greenLed, HIGH);
    }
      
    else if (score >= 5 && score <= 9)
    {
      result = "You have Mild Depression";
      digitalWrite(greenLed, HIGH);
    }

    else if (score >= 10 && score <= 14)
    {
      result = "You have Moderate Depression";
      digitalWrite(blueLed, HIGH);
      digitalWrite(pump, HIGH);
    }

    else if (score >= 15)
    {
      result = "You have Severe Depression";
      digitalWrite(blueLed, HIGH);
      digitalWrite(pump, HIGH);
    }

    else
    {
      result = "You are healthy";
      digitalWrite(greenLed, HIGH);
    }
  }
  
  else if(testType == "bipolar")
  {
    setBackground(RA8875_YELLOW);
    if (score >= 9)
    {
      result = "You are Bipolar"; 
      digitalWrite(yellowLed, HIGH);
      digitalWrite(pump, HIGH); 
    }
      
    else
    {
      result = "You are healthy";
      digitalWrite(greenLed, HIGH);
    }
  }
  
  else if(testType == "anxiety")
  {
    setBackground(RA8875_RED); 
    if (score >= 1 && score <= 4) 
    {
      result = "You have Minimal Anxiety";
      digitalWrite(greenLed, HIGH);
    }
    else if (score >= 5 && score <= 9)
    {
      result = "You have Mild Anxiety";
      digitalWrite(greenLed, HIGH);
    }

    else if (score >= 10 && score <= 14)
    {
      result = "You have Moderate Anxiety";
      digitalWrite(redLed, HIGH);
      digitalWrite(pump, HIGH);
    }

    else if (score >= 15)
    {
      result = "You have Severe Anxiety";
      digitalWrite(redLed, HIGH);
      digitalWrite(pump, HIGH);
    }

    else
    {
      result = "You are healthy";
      digitalWrite(greenLed, HIGH);
    }
  }

  // Tells the user to look at the Brain model to see where the most active parts of the brain are for someone with that mental illness
  setText(RA8875_BLACK, 2, 0, 75);
  largeText(result, 2);
  tft.textSetCursor(0, 200);
  largeText("Look at the brain to see where the most active parts of your brain are", 2);
  setText(RA8875_BLACK, 1, 160, 350);
  tft.textWrite("Press anywhere to continue");
  tft.textSetCursor(15, 420);
  tft.textColor(RA8875_BLACK, RA8875_WHITE);
  tft.textWrite("*Diagnosis is not a professional opinion");
  delay(900);

  tft.graphicsMode();
  tft.touchRead(&tx, &ty);
  
  while(!tft.touched()){}
  for(int i = 22; i <= 26; i++)
  {
    digitalWrite(i, LOW);
  }
}

// Displays facts and statistics from the Canadian Mental Health Associations 
void displayFacts()
{
  if((idleTime = (10 * 5)))
  {
    setBackground(RA8875_CYAN);
    setText(RA8875_BLACK, 2, 0, 160);
    largeText(facts[factNumber], 2);
    setText(RA8875_BLACK, 0, 0, 430);
    largeText("From Canadian Mental Health Association (https://cmha.ca/about-cmha/fast-facts-about-mental-illness)", 0);
    setText(RA8875_BLACK, 1, 160, 350);
    tft.textWrite("Press anywhere to start survey");
    if (factNumber == (sizeof(facts)/sizeof(int)-1))
    {
      factNumber = 0;
      idleTime -= 50;
    }
    
    else
    {
      factNumber++;
      idleTime -= 50;
    }
  }
}

// The "largeText" function is used to display text that is passed through the parameters
void largeText(String text, int fontSize)
{
  tft.textMode();
  String textBuffer = "";
  int charLine = 100 / (fontSize + 1);
  int spaces = 0;
  int spaceBuffer = 0;
  int lineNumber = 1;
  
  for(int i = 0; i < text.length(); i ++)
  {
    if (i == text.lastIndexOf((' '), (charLine*lineNumber - spaces)))
    {
      for(int j = 0 ; j < ((charLine*lineNumber - spaces) - (i)); j ++)
      {        
        textBuffer = textBuffer + " ";
        spaceBuffer++;
      }

      // Makes a new line for the text
      lineNumber ++;
      spaces = spaceBuffer - lineNumber + 1;
    }
    else
      textBuffer = textBuffer + (String)text.charAt(i);
  }
  char charBuffer[textBuffer.length() + 1];
  textBuffer.toCharArray(charBuffer, textBuffer.length() + 1);
  tft.textEnlarge(fontSize);
  tft.textWrite(charBuffer);
}

// Configures the text colour, fontSize, and location
void setText(int colour, int fontSize, int xValue, int yValue)
{
  tft.textMode();
  tft.textTransparent(colour);
  tft.textEnlarge(fontSize);
  tft.textSetCursor(xValue, yValue);
}

// Sets the background colour of the screen
void setBackground(int colour)
{
  tft.graphicsMode();
  tft.fillScreen(RA8875_WHITE);
  tft.fillRect(0, 0, tft.width(), 75, colour);
  tft.fillRect(0, 405, tft.width(), 75, colour);
}
