# Intel.Letto
An intelligent alarm clock connected to your bed. This is an alarm developed for a Belgian tv show, [Team Scheire](http://www.canvas.be/team-scheire), in order to help somebody who does not wake up from normal alarms, due to a disturbed sleeping cycle. 
Before using this solution, contact a doctor to determine your sleeping disorder! Intel.Letto was used to help with Delayed Sleep Phase Syndrome (DSPS), but could be useful in other cases. Nevertheless, a technical solution is **never** the complete answer, a doktor should be your first stop!

Intel.Letto works by working on all stimuli:

* sound: a normal buzzer
* sound: voices, animals, reminders. Interaction that requires the brain to listen
* touch: wind from a fan blowing irregularly
* touch: massage movement in an irregular unpredictable way
* light: waking up requires high lumen light to be switched on
* light: in the pre-alarm phase we add sunrise to help becoming aware of the need to get up.
* brain activity: deep sleepers can switch of alarms, then continue sleeping. To avoid this, a bed detector is present. You can only switch of the alarm when you are *not* in the bed. Also, a specific action must be done.

All the above is done to gradually make the sleeper aware of the fact they need to get up. On the alarm time, the cycle becomes stronger. In a rough drawing:
![wake up sequence](doc/intelletto_sequentie.png)

## Hardware


## Arduino Test Sketches

1. [MPR121 Capacitive touch testcode](/alarmblanket/ArduinoTestSketches/MPR121_capacitive_touch_testcode/MPR121_capacitive_touch_testcode.ino): connect the MPR121 sensor to a NodeMCU for tracking and an OLED 128x32 for display of result
2. [Neopixel sunrise](/alarmblanket/ArduinoTestSketches/Neopixel_sunrise/Neopixel_sunrise.ino): Connect a neopixel ledstrip to Arduino or NodeMCU, and simulate the sunrise
3. [Time clock with NodeMCU](/alarmblanket/ArduinoTestSketches/Time_clock_testcode/Time_clock_testcode.ino): Show the time and date on an OLED 128x32 display using NodeMCU and an NTP server. At least once wifi connection must be made.
4. [Alarm clock with sunrise](/alarmblanket/ArduinoTestSketches/Alarm_clock_sunrise/Alarm_clock_sunrise.ino): Show time and date on an OLED 128x32 display using NodeMCU and use neopixel strip to show sunrise as alarm time is approached
