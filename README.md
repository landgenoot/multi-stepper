multi-stepper
=============
Usage:
```
#include <MultiStepper.h>

MultiStepper multiStepper(2);

void setup() ​{
  multiStepper.addMotor(64, 8, 9, 10, 11);
  multiStepper.addMotor(64, 4, 5, 6, 7);
  multiStepper.setSpeed(100, 0);
  multiStepper.setSpeed(100, 1);
}

void loop() {
  int steps[] = {10, 10};
  multiStepper.step(steps);
}
```
