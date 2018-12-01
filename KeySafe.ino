#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* Stephen Devlin 2018 - IDE 1 Gizmo project
 *  
    This code provides the logic to operate the Key Safe.
    It retrieves yaw angles from a BNO055 accelerometer sensor,
    determines whether they match a predetermined "combination",
    and if so, it sets an I/O pin to high in order to fire a solenoid
    and release a latch.

    I have re-used some code from the "Rawdata" example sketch that comes with the BNO055 sensor
*/

#define BNO055_SAMPLERATE_DELAY_MS (10)  // sets the polling frequency for position data at 100Hz 

Adafruit_BNO055 bno = Adafruit_BNO055();

float new_angle, previous_angle, last_stopped_angle;
String solution_key = "rlrr";       // for the gizmo project this was just hardcoded.  I intend in the next version to allow the user to set any code
String solution_string = "xxxx";    // I use a string with "r"'s and "l"'s to represent the code.  This allowed easy debugging on the serial monitor.
boolean last_turn_added = false;
int stopped_time = 0, turn_number = 0;
const int solenoid_pin = 5;         // Digital Pin 5 for solenoid release


/**************************************************************************/
/*
    Setup code starts the BNO055 sensor and initialises some IO pins and variables
*/
/**************************************************************************/
void setup(void)
{
  if (!bno.begin())              // boot up BNO055 and check that it is working OK
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);                   // allow some delay for BNO055 to stabilise and calibrate

  pinMode(13, OUTPUT);                                                 // Set output 13 (LED) as output - I use this LED for debugging as sometimes the solenoid sticks
                                                                       // So I set the LED on when the code is correct so that I can tell it is a solenoid rather than a 
                                                                       // logic problem if the top does not pop open
  pinMode(solenoid_pin, OUTPUT);                                       // Set Solenoid pin for output
  digitalWrite(solenoid_pin, LOW);                                     // Make sure solenoid is off

  bno.setExtCrystalUse(true);                                          // this is recommended by the BNO055 datasheet to improve accuracy

}

/**************************************************************************/
/*  The loop waits for the BNO055 yaw angle to start changing.  It ignores small changes (to ignore drift) - but if the changes are above a certain rate
     then it determines that it is being turned.  Once a turn has stopped for more than .2 of a second, I calculate which direction I have turned.
     This turn is then added to the combination in sequence.  When the turning stops for more than 2 seconds then the safe assumes that the user has
     completed their code and compares it to the key.  If the code is correct then the solenoid PIN is set to high and the safe will open.  If it is
     incorrect then the combination is reset to x's and the user can try again.  In future versions I may put a timer on this so that only one try per
     minute is permitted.


*/
/**************************************************************************/
void loop(void)
{
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  new_angle= (euler.x());       //BNO055 returns the angle of the Key safe

  if (abs(previous_angle - new_angle) > .45) {       //ignore small drift and pertubations in gyro angle I assume when turning
                                                     //that the rate will be more than 45 degrees per second (remember we are working at 100Hz)
                                                     
    last_turn_added = false;    // this means we are turning - so zero stopped time and reset the boolean that tells us that there is a new turn to be added
    stopped_time = 0;
  }
  else
  {
    stopped_time += 1;          //  if not turning - increment the counter so that we can tell how long the safe is stopped
  }



  if (stopped_time > 20 && !last_turn_added) {

    if (abs(last_stopped_angle - new_angle) < 180)   {    // this "180" logic is needed since the angle given by the BNO055 is a compass bearing
                                                          // hence if you turn from angle 35 left by 50 degrees the new angle is 345
                                                          // just subtracting the two angles will not tell you if it has gone left or right !
                                                          // so I check whether absolute difference is more than 180 and if it is, then I use reverse logic
      if (last_stopped_angle < new_angle) {
        turning[0] = 'r';
      }
      else
      {
        turning[0] = 'l';
      }
    }
    else
    {
      if (last_stopped_angle > new_angle)
      {
        turning[0] = 'r';
      }
      else
      {
        turning[0] = 'l';
      }
    }

    last_turn_added = true;                             // this line ensures that we only add the turn once.  Otherwise if you stopped for, say 1 whole second between turns 
                                                        // you would add the turn multiple times
                                                        
    solution_string.setCharAt(turn_number, turning[0]); // overwite the combination string one character at a time
    last_stopped_angle = new_angle;                     // reset the last_stopped_angle for the next time so we can always determine what direction we are going in
    turn_number += 1;
    stopped_time = 0;
  }

  if (stopped_time > 300) {                              // if safe is stopped for 3 seconds then it compares the combination string with the key
    if (solution_string == solution_key) {                
      digitalWrite(13, HIGH);                                               // switch on LED (debug only)
      digitalWrite(solenoid_pin, HIGH);                                     //Fire solenoid
      delay (100);                                                         //Wait .1s before closing solenoid
      digitalWrite(solenoid_pin, LOW);                                     //Switch solenoid off
    }
    
    stopped_time = 0;                                    // this is only relevant when code is incorrect
    turn_number = 0;                                     // these variable resets allow the user to start again
    solution_string = "xxxx";
  }

  previous_angle = new_angle;   // I use new_angle_previous to hold the last value of the yaw angle so that I can tell whether it is 
                               // turning at the start of the loop

  delay(BNO055_SAMPLERATE_DELAY_MS);    // this delay just determines the frequency with which we poll the sensor
}
