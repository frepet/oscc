# The Open Slot Car Controller Project
OSCC has the goal to provide a cheap option to hobby-grade slot car controllers.

## Description
The heart of OSCC is a ATTiny84 microcontroller running Arduino.
The Arduino reads analog input from the throttle hall effect sensor
and the settings potentiometers. The inputs are converted into two
PWM outputs. One PWM drives the P-Channel MOSFET that enables the
car to move forward. The other PWM shorts the motor circuit to enable breaking.

## Circuit
The circuit is built for a track running at 10-12 Volts with the shared braid beeing the
negative(ground). The controller should be hooked up to:
- Red: Track ground (minus from power supply and left braid).
- Black: Right braid.
- Yellow or White: Track +10-12 Volts.

The complete circuit:

![OSCC Circuit Diagram](docs/circuit.png)
