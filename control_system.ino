#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "PCF8575.h"
#include "Wire.h"

#include <LiquidCrystal_I2C.h>

#include <Keypad.h>

#include <EEPROM.h>

PCF8575 ioport(0x20);

/**
 * @name setup()
 * initialize the program
 */
int enable = 13; // activates the motor and controls the speed
int startSequence = 0; // Starts the process of the control system by having the user enter their passcode
int motor_logic_1 = 11; // positive motor pin
int motor_logic_2 = 12; // negative motor pin
int flag = 0; // responsible for activating the locking or unlocking process
int coin_sig = 2; // coin interrupt pin
boolean insert = false; // Checks if a coin is inserted
int pulse = 0; // Records the pulse of the coins inserted

int balance; // Records the number of quarter coins inserted
int hours = 0; // the number of user inputted hours

/*
  Pwd variables record the initial user passcod
*/
int pwdCounter = 0;
int pwdDigit1 = 0; 
int pwdDigit2 = 0;
int pwdDigit3 = 0;
int pwdDigit4 = 0; 
int pwdCorrectFlag = 0; // checks if the passcode is correct when user types it in the end

/*
  cPwd variables record if the passcode the user inserted during the locking process is correct
*/
int cPwdDigit1 = 0;
int cPwdDigit2 = 0; 
int cPwdDigit3 = 0;
int cPwdDigit4 = 0;
int pwdAttemptsCounter = 10; // number of attempts in unlocking the bike
int intruderFlag = 0; // checks for anyone trying to attempt to unlock a bike with all attempts wrong

int devMode = 0; // for testing purposes in unlocking the bike after a sample time

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {

  {'1','2','3','A'},

  {'4','5','6','B'},

  {'7','8','9','C'},

  {'*','0','#','D'}

}; // The characters of the 4x4 keypad

int counter =0; // Program counter

byte rowPins[ROWS] = {10, 9, 8, 7}; //keypad rows

byte colPins[COLS] = {6,A0,A1,A2}; //keypad columns

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // initiates the use of the keypad 

LiquidCrystal_I2C lcd(0x27, 16, 2); // records keypad inputs and displays outputs based on keypad and coin slot

void setup()
{
	
  Serial.begin(9600); 
  lcd.begin(); // activates the lcd
  lcd.backlight();
	ioport.begin();
  /*
    initializes inputs and outputs for all the components
  */
  pinMode(enable, OUTPUT); 
  pinMode(motor_logic_1, OUTPUT);
  pinMode(motor_logic_2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(coin_sig),coinSignal, FALLING);

  /*
    sets up the lcd display
  */
  lcd.clear();
  lcd.print("Setting up...");
  /*
    Sets up the motor but doesn't start as the locking process hasn't started
  */
  digitalWrite(enable, LOW);
  digitalWrite(motor_logic_1, LOW);
  digitalWrite(motor_logic_2, LOW);

  digitalWrite(motor_logic_1, HIGH);
  digitalWrite(motor_logic_2, LOW);
  delay(200000);
  lcd.clear();
  /*
    initializes all the counters and flags
  */
  balance = 0;
  counter = 0; 
  flag = 0;
  startSequence = 0;
  insert = false;
  pwdDigit1 = 0; 
  pwdDigit2 = 0;
  pwdDigit3 = 0;
  pwdDigit4 = 0; 
  pwdCounter = 0;

  cPwdDigit1 = 0;
  cPwdDigit2 = 0; 
  cPwdDigit3 = 0;
  cPwdDigit4 = 0;
  pwdAttemptsCounter = 10; 
  intruderFlag = 0;
  pwdCorrectFlag = 0;
  
}

/**
 * @name loop()
 * main loop of program and runs endlessly
 */
void loop()
{

  //lcd.clear();
  devMode = 1; //turn devMode off for actual usage! 

  if (intruderFlag == 1) {
    //do nothing
    //keep system from unlocking
    lcd.println("INTRUDER DETECTED");    
  }

  if (startSequence == 0) {
    lcd.print("Welcome user!");

    delay(5000);

    lcd.clear();

    lcd.print("Enter 4 pin passcode");

    delay(5000);

    lcd.clear();

    startSequence = 1; 
  }

  if (insert) {

    lcd.clear();

    insert = false;

    balance += 1;

    lcd.print("balance: ");

    lcd.print(balance);

    delay(3000);

    lcd.clear();

  }

  if (balance == hours && balance > 0 && hours > 0){
      
    lockBike();

  }

  char key = keypad.getKey();

  if (key){

    key = key - '0';
    
    Serial.println(key);

    if (counter < 5) { //means user is in passcode-entering stage
      
      lcd.print('*');

      //this code saves 4 digits of 4-pin passcode in seperate variables
      if (counter == 1) {
        pwdDigit1 = key; 
      } else if (counter == 2) {
        pwdDigit2 = key;
      } else if (counter == 3) {
        pwdDigit3 = key;
      } else if (counter == 4) {
        pwdDigit4 = key; 
      }

    } else if (counter == 5) { //means user is in the hours-entering stage

    hours = key;

    lcd.print(hours);

    lcd.print(" hours");

    delay(1000);

    lcd.clear();

    lcd.print("Cost is ");
    lcd.print(hours);
    delay(5000);
    int cost = 0;
    cost = key;
    Serial.println(cost);
    delay(1000);
    lcd.clear();

    }

    counter = counter + 1;

  }

 

  if (counter == 4) {

    delay(300);

    lcd.clear();

    delay(300);

    lcd.print("Passcode set");

    delay(5000);

    lcd.clear();

    lcd.print("How many hours?");

    delay(5000);

    lcd.clear();

    counter = counter + 1;

  }

  if (flag == 1) { //Locking complete
    lcd.clear(); 
    
    // user can release their bike anytime by entering the passcode on the keypad
    lcd.println("Enter passcode anytime on keypad to unlock");
    delay(8000);
    lcd.clear();

    lcd.print("Wait ");
    lcd.print(hours); 
    lcd.print(" hours");

    
    if (key) {

      lcd.clear();

      key = key - '0';

      lcd.print("*");

      if (pwdCounter == 0) {
        cPwdDigit1 = key; 
      } else if (pwdCounter == 1) {
        cPwdDigit2 = key;
      } else if (pwdCounter == 2) {
        cPwdDigit3 = key;
      } else if (pwdCounter == 3) {
        cPwdDigit4 = key; 
      }

      pwdCounter = pwdCounter + 1;

    }

    if (cPwdDigit1 == pwdDigit1 && cPwdDigit2 == pwdDigit2 && cPwdDigit3 == pwdDigit3 && cPwdDigit4 == pwdDigit4){
        pwdCorrectFlag = 1; 
    } else {

      if (pwdAttemptsCounter == 0) {
        intruderFlag = 1; 
        lcd.clear();
        lcd.println("LOCKED");
        delay(5000);
        lcd.clear();
        lcd.println("INTRUSION DETECTED");
      }

      
      lcd.clear();
      lcd.print("Password incorrect.");
      delay(5000);
      lcd.clear();
      lcd.print("Try again"); 
      delay(5000);
      lcd.clear();

      lcd.print(pwdAttemptsCounter);

      lcd.print(" attempts left");
      delay(8000);

      pwdAttemptsCounter = pwdAttemptsCounter - 1; 

    }

    if (devMode == 1) {
      delay(15000);
    } else {
      delay(hours * 60 * 60 * 1000);
    }

    if (intruderFlag == 0) {
      
      unlockBike();

    }

    //reset variables for next user
    counter = 0; 
    flag = 0;   
    pwdDigit1 = 0; 
    pwdDigit2 = 0;
    pwdDigit3 = 0;
    pwdDigit4 = 0; 
    cPwdDigit1 = 0; 
    cPwdDigit2 = 0;
    cPwdDigit3 = 0;
    cPwdDigit4 = 0; 
    startSequence = 0;  
    balance = 0;
    pwdCounter = 0;
    hours = 0; 
    pwdAttemptsCounter = 10; 
    pwdCorrectFlag = 0;

  }

}

void unlockBike() {
    digitalWrite(enable, HIGH);
    digitalWrite(motor_logic_1, HIGH);
    digitalWrite(motor_logic_2, LOW);
    delay(200000);
    digitalWrite(enable, LOW);


    lcd.clear();
    lcd.print("Unlocking");
    delay(5000);
    lcd.clear();
    lcd.println("Thank you");
    delay(5000);
    lcd.clear();
    lcd.println("Come Again");
    delay(5000);
    lcd.clear();
}

void lockBike() {
    lcd.print("Confirmed");

    delay(5000);

    lcd.clear();

    lcd.print("Engaging lock");

    delay(5000);

    lcd.clear();

    lcd.print("Stand back");

    delay(5000);

    lcd.clear();
    digitalWrite(enable, HIGH);
    digitalWrite(motor_logic_1, LOW);
    digitalWrite(motor_logic_2, HIGH);
    lcd.print("Locking");
    delay(200000);
    digitalWrite(enable, LOW);

    lcd.clear();
    lcd.print("Lock complete");
    delay(5000);

    flag = 1;
}

void coinSignal() {

  pulse++;

  insert = true;

  delay(1000);

}

