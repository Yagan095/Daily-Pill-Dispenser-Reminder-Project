//libraries
#include <Wire.h>
#include <RTClib.h>
//REF: Tom Igoe wiring & syntax for liquid crystal display
#include <LiquidCrystal.h>

//REF: Nikodem Bartnik - nikodembartnik.pl syntax for the step motor
#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

//initizalizing arduino pins for LCD
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//REF: https://core-electronics.com.au/guides/accurate-clock-arduino-uno/ syntax for initilizing rtc 
RTC_DS1307 rtc;

//declaring time 
int hours = 1;
int minutes = 0;
int seconds = 0;

//declaring alarm time
int alarmHours = 9;
int alarmMinutes = 0;

//declaring buzzer digital pin
const int buzzer = 8;
//declare temp values
int step_number = 0;

//makes sure the boolean triggers only once a day
bool alarmToday = false;

//button HIGH or LOW variables
int button1 = 0, button2 = 0, button3 = 0;

void setup() {
  //declaring button pin modes as buttons REF: https://deepbluembedded.com/arduino-pinmode-function-input-pullup-pulldown/ for pullup
  pinMode(A1, INPUT_PULLUP); // button 1
  pinMode(A2, INPUT_PULLUP); // button 2
  pinMode(A3, INPUT_PULLUP); // button 3

  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  //set pins for stepper motor output
  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);
  //for buzzer set up
  pinMode(buzzer, OUTPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //if the rtc time isn't set, set the time
  if (! rtc.isrunning()) {
    changeTime();
  }

}

void loop() {
  //Reading buttons for input except set button 3 as low
  button1 = digitalRead(A1);
  button2 = digitalRead(A2);
  button3 = HIGH;
  //REF: https://www.instructables.com/Arduino-Alarm-Clock-3/ displaying rtc time on 1st row
  DateTime now = rtc.now();
  displayTime();
  //display set alarm time on 2nd row LCD
  lcd.setCursor(0, 1);
  lcd.print(String(alarmHours) + ":" + String(alarmMinutes));
  //makes sure the boolean triggers only once a day
  //triggers alarm and dispenser when alarm time hits
  if (now.hour() == alarmHours && now.minute() == alarmMinutes && now.second() == 0 && !alarmToday) {
    alarm();
    alarmToday = true;
  } 
  //resets alarm for today after a second
  alarmToday = alarmToday && now.hour() == alarmHours && now.minute() == alarmMinutes && now.second() > 1;
  //button 1 changes the time, button 2 changes the alarm time
  if (button1 == LOW){
    changeTime();
  } else if (button2 == LOW){
    changeAlarm();
  } 
}

//REF: https://www.instructables.com/Arduino-Alarm-Clock-3/ displaying rtc time
void displayTime(){
  // Read the current time from RTC
  DateTime now = rtc.now();
  // Display the current time on the LCD
  lcd.setCursor(0,0);
  //display the hour
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print("0");
  }
  //dispay the minutes
  lcd.print(now.minute());
  lcd.print(":");
  //display the seconds
  if (now.second() < 10) {
    lcd.print("0");
  }
  lcd.print(now.second());
  lcd.print("   ");
}

void changeTime(){
  delay(250);
  //set the current time to now
  DateTime now = rtc.now();
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();
  //repeat until user exits via pressing button 3
  while(button3 == HIGH){
    button3 = digitalRead(A3);
    lcd.setCursor(0,0);
    //display time currently set
    lcd.print(String(hours) + ":" + String(minutes) + ":00*  ");
    //when button 1 is pressed, increase set time by 1 hour
    button1 = digitalRead(A1);
    button2 = digitalRead(A2);
    if (button1 == LOW){
      hours += 1;
      if(hours > 24){
        hours = 1;
      }
      delay(250);
    //when button 12 is pressed, increase set time by 1 minute
    } else if (button2 == LOW){
      minutes += 1;
      if (minutes > 59){
        minutes = 0;
      }
      delay(250);
    }
  }
  //set the RTC clock as the set time
  rtc.adjust(DateTime(2000, 1, 1, hours, minutes, 0));
}

//procedure to change the alarm time
void changeAlarm(){
  delay(250);
  //repeat until button 3 is pressed
  while(button3 == HIGH){
    displayTime();
    lcd.setCursor(0,1);
    //display the current set alarm time
    lcd.print(String(alarmHours) + ":" + String(alarmMinutes) + "*  ");
    //when button 1 is pressed, increase the alarm time by 1 hour
    button1 = digitalRead(A1); //button1
    button2 = digitalRead(A2); //button2
    button3 = digitalRead(A3); //button3
    if (button1 == LOW){
      alarmHours += 1;
      if(alarmHours > 24){
        alarmHours = 1;
      }
      delay(250);
    //if button 2 is pressed, increase the alarm time by 1 minute
    } else if (button2 == LOW){
      alarmMinutes += 1;
      if (alarmMinutes > 59){
        alarmMinutes = 0;
      }
      delay(250);
    }
  }
  lcd.setCursor(0,1);
  //display the final alarm time
  lcd.print(String(alarmHours) + ":" + String(alarmMinutes) + "   ");
}

//procedure of when the time hits the alarm time
void alarm(){
  //if statement ensures the motor triggers no more
  if(button3 == HIGH){
    //dispensing medication procedure via 360 rotation
    for(int steps = 0; steps < 2048; steps++){
      OneStep();
      //delay per step
      delay(3);
    }
  }
  //buzzer loops until button 3 is held
  while(button3 == HIGH){
    //REF: www.ardumotive.com/how-to-use-a-buzzer-en.html#:~:text=It's%20simple%2C%20tone(buzzer%2C,making%20a%20short%20beeping%20sound. syntax for buzzer
    //buzz at a tone of 1000 for 1 second, then stop, then buzz again after anotehr second
    tone(buzzer,1000);
    delay(1000);
    noTone(buzzer);
    delay(1000);
    button3 = digitalRead(A3);
  }
}

//REF Nikodem Bartnik - nikodembartnik.pl modified procedure for the step motor counterclockwise
void OneStep(){
  switch(step_number){
    case 0:
    digitalWrite(STEPPER_PIN_1, LOW);
    digitalWrite(STEPPER_PIN_2, LOW);
    digitalWrite(STEPPER_PIN_3, LOW);
    digitalWrite(STEPPER_PIN_4, HIGH);
    break;
    case 1:
    digitalWrite(STEPPER_PIN_1, LOW);
    digitalWrite(STEPPER_PIN_2, LOW);
    digitalWrite(STEPPER_PIN_3, HIGH);
    digitalWrite(STEPPER_PIN_4, LOW);
    break;
    case 2:
    digitalWrite(STEPPER_PIN_1, LOW);
    digitalWrite(STEPPER_PIN_2, HIGH);
    digitalWrite(STEPPER_PIN_3, LOW);
    digitalWrite(STEPPER_PIN_4, LOW);
    break;
    case 3:
    digitalWrite(STEPPER_PIN_1, HIGH);
    digitalWrite(STEPPER_PIN_2, LOW);
    digitalWrite(STEPPER_PIN_3, LOW);
    digitalWrite(STEPPER_PIN_4, LOW);
  } 
  step_number++;
  if(step_number > 3){
    step_number = 0;
  }
}
