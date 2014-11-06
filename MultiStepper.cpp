/*
  Stepper.cpp - - Stepper library for Wiring/Arduino - Version 0.4
  
  Original library     (0.1) by Tom Igoe.
  Two-wire modifications   (0.2) by Sebastian Gassner
  Combination version   (0.3) by Tom Igoe and David Mellis
  Bug fix for four-wire   (0.4) by Tom Igoe, bug fix from Noah Shibley  
  Enable support for multiple motors   (0.5) by Daan Middendorp

  Drives a unipolar or bipolar stepper motor using  2 wires or 4 wires

  When wiring multiple stepper motors to a microcontroller,
  you quickly run out of output pins, with each motor requiring 4 connections. 

  By making use of the fact that at any time two of the four motor
  coils are the inverse  of the other two, the number of
  control connections can be reduced from 4 to 2. 

  A slightly modified circuit around a Darlington transistor array or an L293 H-bridge
  connects to only 2 microcontroler pins, inverts the signals received,
  and delivers the 4 (2 plus 2 inverted ones) output signals required
  for driving a stepper motor.

  The sequence of control signals for 4 control wires is as follows:

  Step C0 C1 C2 C3
     1  1  0  1  0
     2  0  1  1  0
     3  0  1  0  1
     4  1  0  0  1

  The sequence of controls signals for 2 control wires is as follows
  (columns C1 and C2 from above):

  Step C0 C1
     1  0  1
     2  1  1
     3  1  0
     4  0  0

  The circuits can be found at 
 
http://www.arduino.cc/en/Tutorial/Stepper
 
 
 */


#include "Arduino.h"
#include "Stepper.h"


/*
 * Creates arrays with length equal to the amount of motors.
 */
MultiStepper::MultiStepper(int number_of_motors)
{
  this->step_number = new int[number_of_motors];      // which step the motor is on
  this->speed = new int[number_of_motors];        // the motor speed, in revolutions per minute
  this->direction = new int[number_of_motors];      // motor direction
  this->last_step_time = new int[number_of_motors];    // time stamp in ms of the last step taken
  this->number_of_steps = new int[number_of_motors];    // total number of steps for this motor
  this->step_delay = new int[number_of_motors];
  
  this->motor_pin_1 = new int[number_of_motors];
  this->motor_pin_2 = new int[number_of_motors];
  this->motor_pin_3 = new int[number_of_motors];
  this->motor_pin_4 = new int[number_of_motors];
  
  this->pin_count = new int[number_of_motors];
  this->motor_count = 0;
}

/*
 * two-wire constructor.
 * Sets which wires should control the motor.
 */
MultiStepper::addMotor(int number_of_steps, int motor_pin_1, int motor_pin_2)
{
  int i = this->motor_count;
  this->step_number[i] = 0;      // which step the motor is on
  this->speed[i] = 0;        // the motor speed, in revolutions per minute
  this->direction[i] = 0;      // motor direction
  this->last_step_time[i] = 0;    // time stamp in ms of the last step taken
  this->number_of_steps[i] = number_of_steps;    // total number of steps for this motor
  
  // Arduino pins for the motor control connection:
  this->motor_pin_1[i] = motor_pin_1;
  this->motor_pin_2[i] = motor_pin_2;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1[i], OUTPUT);
  pinMode(this->motor_pin_2[i], OUTPUT);
  
  // When there are only 2 pins, set the other two to 0:
  this->motor_pin_3[i] = 0;
  this->motor_pin_4[i] = 0;
  
  // pin_count is used by the stepMotor() method:
  this->pin_count[i] = 2;
  
  this->motor_count++;
}

/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */
MultiStepper::addMotor(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4)
{
  int i = this->motor_count;
  this->step_number[i] = 0;      // which step the motor is on
  this->speed[i] = 0;        // the motor speed, in revolutions per minute
  this->direction[i] = 0;      // motor direction
  this->last_step_time[i] = 0;    // time stamp in ms of the last step taken
  this->number_of_steps[i] = number_of_steps;    // total number of steps for this motor
  
  // Arduino pins for the motor control connection:
  this->motor_pin_1[i] = motor_pin_1;
  this->motor_pin_2[i] = motor_pin_2;
  this->motor_pin_3[i] = motor_pin_3;
  this->motor_pin_4[i] = motor_pin_4;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1[i], OUTPUT);
  pinMode(this->motor_pin_2[i], OUTPUT);
  pinMode(this->motor_pin_3[i], OUTPUT);
  pinMode(this->motor_pin_4[i], OUTPUT);

  // pin_count is used by the stepMotor() method:  
  this->pin_count[i] = 4;  
  
  this->motor_count++;
}

/*
  Sets the speed in revs per minute

*/
void MultiStepper::setSpeed(long whatSpeed, int motor)
{
  this->step_delay[i] = 60L * 1000L / this->number_of_steps / whatSpeed;
}


int MultiStepper::stepsLeft(int steps_left[])
{
  int max_steps_left = 0;
  for (int steps: steps_left) {
    if (steps > max_steps_left) { max_steps_left = steps; }
  }
  return max_steps_left;
}

/*
  Moves the motors steps_to_move steps.  If the number is negative, 
  the motor moves in the reverse direction.
 */
void MultiStepper::step(int steps_to_move[])
{  
  int steps_left = new int[this->motor_count];
  int i = 0;
  int max_steps_left = 0; // Used to determine if we have to keep looping
  
  for (int steps: steps_to_move) {
	steps_left[i] = abs(steps); // how many steps to take
	
    // determine direction based on whether steps_to_mode is + or -:
    if (steps > 0) {this->direction[i] = 1;}
    if (steps < 0) {this->direction[i] = 0;}
	
	if (steps > max_steps_left) {max_steps_left = steps;}
	i++;
  }
  
  // decrement the number of max steps, moving one step each time:
  while(stepsLeft(steps_left) > 0) {
	for (int motor : steps_to_move) {
	  if (millis() - this->last_step_time[motor] >= this->step_delay[motor]){
	    this->last_step_time[motor] = millis();
	  
	    if (this->direction == 1) {
	      this->step_number[motor]++;
		  if (this->step_number == this->number_of_steps[motor]) {
		    this->step_number[motor} = 0;
		  }
	    } else {
	      if (this->step_number[motor] == 0) {
		    this->step_number[motor] = this->number_of_steps[motor];
		  }
		  this->step_number--;
	    }
        stepMotor(this->step_number[motor] % 4, motor);
      }
	}
  }
}

/*
 * Moves the motor forward or backwards.
 */
void MultiStepper::stepMotor(int thisStep, int motor)
{
  if (this->pin_count[motor] == 2) {
    switch (thisStep) {
      case 0: /* 01 */
      digitalWrite(motor_pin_1[motor], LOW);
      digitalWrite(motor_pin_2[motor], HIGH);
      break;
      case 1: /* 11 */
      digitalWrite(motor_pin_1[motor], HIGH);
      digitalWrite(motor_pin_2[motor], HIGH);
      break;
      case 2: /* 10 */
      digitalWrite(motor_pin_1[motor], HIGH);
      digitalWrite(motor_pin_2[motor], LOW);
      break;
      case 3: /* 00 */
      digitalWrite(motor_pin_1[motor], LOW);
      digitalWrite(motor_pin_2[motor], LOW);
      break;
    } 
  }
  if (this->pin_count[motor] == 4) {
    switch (thisStep) {
      case 0:    // 1010
      digitalWrite(motor_pin_1[motor], HIGH);
      digitalWrite(motor_pin_2[motor], LOW);
      digitalWrite(motor_pin_3[motor], HIGH);
      digitalWrite(motor_pin_4[motor], LOW);
      break;
      case 1:    // 0110
      digitalWrite(motor_pin_1[motor], LOW);
      digitalWrite(motor_pin_2[motor], HIGH);
      digitalWrite(motor_pin_3[motor], HIGH);
      digitalWrite(motor_pin_4[motor], LOW);
      break;
      case 2:    //0101
      digitalWrite(motor_pin_1[motor], LOW);
      digitalWrite(motor_pin_2[motor], HIGH);
      digitalWrite(motor_pin_3[motor], LOW);
      digitalWrite(motor_pin_4[motor], HIGH);
      break;
      case 3:    //1001
      digitalWrite(motor_pin_1[motor], HIGH);
      digitalWrite(motor_pin_2[motor], LOW);
      digitalWrite(motor_pin_3[motor], LOW);
      digitalWrite(motor_pin_4[motor], HIGH);
      break;
    } 
  }
}

/*
  version() returns the version of the library:
*/
int MultiStepper::version(void)
{
  return 5;
}
