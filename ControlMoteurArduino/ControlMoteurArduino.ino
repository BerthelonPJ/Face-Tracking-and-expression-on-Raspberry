
#include <Servo.h>

Servo servoH;
Servo servoV;

int posH = 90, posV = 90;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  servoH.attach(8);
  servoV.attach(9);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
}

void serialEvent() {
  while (Serial.available()) {
    byte receivedCommand = Serial.read();

    switch(receivedCommand){
      case 'l':
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        Serial.println("Ol");
        break;
      case 'L':
        digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
        Serial.println("OL");
        break;
      case 'v':
        servoV.write(posV - 2);
        posV = posV - 2;
        delay(15);
        break;
      case 'V':
        servoV.write(posV + 2);
        posV = posV + 2;
        delay(15);
        break;
      case 'h':
        servoH.write(posH - 2);
        posH = posH - 2;
        delay(15);
        break;
      case 'H': 
        servoH.write(posH + 2);
        posH = posH + 2;
        delay(15);
        break; 
      case 'r': 
        servoH.write(90);
        posH = 90;
        delay(15);
        servoV.write(90);
        posV = 90;
        delay(15);
        break; 
      case '\n':
       default:
        Serial.println("Err");
        break;
      }
    }
  }
