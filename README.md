# KeySafe
This is an Arduino based project, using a BNO055 sensor.  It allows a battery powered "keysafe" to only open when it is turned through the right combination of right and left turns.


The safe is turned on using a tilt switch.  It then runs for approximately 60 seconds (using a simple MOSEFT monostable as a timer).

During operation the safe is waiting for input in the form of rotations.  The rotations can be left or right (anti-clockwise or clockwise) and a sequence of these form a unique combination.  If the combination "entered" in this way matches the secret key then the arduino activates a solenoid and releases the lid.

