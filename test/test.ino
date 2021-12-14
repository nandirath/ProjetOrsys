
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(D2);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  myservo.write(180);              // tell servo to go to position in variable 'pos'
    delay(1000); 
    myservo.write(0);              // tell servo to go to position in variable 'pos'
    delay(1000); 
}
