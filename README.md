# HeaterControl
This is the directory for an Arduino heater controller.
User should have setup the ARduino development environment prior to using.
Heater control is initially set for 100C.  HeaterControl.ino must be edited, 
compiled and uploaded to change from default setpoint.

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
http://creativecommons.org/licenses/by-sa/4.0/
     
Partially derived from public domain tutorial(s) code found at arduino website: 
http://arduino.cc/en/Tutorial/AnalogInput

WARNING: No warranty is implied or intended.  If used unattended, failsafe 
circuitry should be added to guard against relay failure in a closed state.

Files include:
HeaterControl.ino - Text file source code for Arduino

Authors note: This works well with a Raspberry Pi to capture the text output
via a terminal piped to a file.  I've used it for testing parts cheaply.  The 
heaters get very hot and WILL cause an issue WHEN the relay fails. 
