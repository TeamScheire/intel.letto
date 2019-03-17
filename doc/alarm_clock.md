# Alarm Clock Base Station

## Introduction
the base station which shows the clock and allows to program the alarm. 
It also contains a buzzer and a LED strip can be connected to it.


## Messages
This module sends and reacts to the following MQTT messages:

1. **intellettoStatus**
The base station publishes on this channel. Subscribe to this topic to see status messages

2. **intelletto**
Test topic to see if alarm connected correctly to the broker and can execute commands. Payload `c1` will switch on builtin LED of the NodeMCU, `c0` will switch it off.

3. **intellettoBedSensor**
The base station listens to these messages to determine if somebody in bed, see in bed module

4. **intellettoMassage**
The station sends messages to control the massage. See massage module

5. **cmnd/sonoff_ventilator/power**
The stations sends messages to set ventilator on or off

6. **cmnd/sonoff_light/power**
The stations sends messages to set light on or off

7. **intellettoLoudSp**
The stations sends messages to have messages played. See message module.


# Resources

* [The NodeMCU code](../alarmblanket/intelletto/intelletto.ino/)
* The lasercut files

![The lasercut box](../lasercut/wekkerbasestation.svg)
