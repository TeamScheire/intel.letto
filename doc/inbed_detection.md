# In Bed Detection Module

For the inbed detection, we use touch module [MPR121](https://learn.sparkfun.com/tutorials/mpr121-hookup-guide).

We take a bed sheet that can be stretched over the bed, and sew conductive carbon fiber yarn to it. Next we take a second bed sheet, and sew it on top if. Like this the carbon fiber is well protected, and washable. 
We do this in 6 horizontal parts where the person who operates the alarm clock sleeps. 

The result:

![bed sheet](inbed_detection02.png)

This is put in the bed, and over it a normal blanket is placed, so as to hide the entire system:

![bed sheet](inbed_detection01.png)

The ends of the carbon fiber is attached to  normal coper wire, which we connect to a 6pin connector. The 6 pin connectors then connects to the In bed detection module, which consists of an MPR121 with is connected over I2C,to a NodeMCU module. 

![bed sheet](inbed_detection03.png)

An OLED screen is attached to the module, indicating on which of the 6 zones a person is detected.

# MPR121

## IMPORTANT REMARK

It is important to note that the MPR121 does a calibration at start up. It is therefore required that **no person is in the bed when the system is switched on**!

To read up on how to change the sensitivity, see [adafruit forum](https://forums.adafruit.com/viewtopic.php?f=19&t=72025).

## Install Library
To work with the code, the MPR121 adafruit library must be installed in the Arduino IDE.

![bed sheet](inbed_detection04.png)

# MQTT
## Configuration
The system connects via MQTT to the Rasp Pi. In the code, set the correct WiFi credentials and give the IP of the MQTT server:

    // write here your wifi credentials 
    const char* password = "********"; // and password
    const char* ssid = "intelletto";   // insert your own ssid 

    //mqtt server/broker 
    const char* mqtt_server = "192.168.0.212";  //eth0 address of the raspberry pi
    uint8_t mqtt_server_IP[4] = {192, 168, 0, 212};

Then use the Arduino IDE to flash the NodeMCU.

## Messages
This module sends and reacts to the following MQTT messages:

1. **intellettoBedSensorInBed**
Publish on this topic to force a publish on the intellettoBedSensor. No payload

2. **intellettoBedSensor**
The bed sensor publishes on this topic every 5 seconds or after receiving an *intellettoBedSensorInBed*. The payload is a single char, 0 for nobody in the bed, 1 for somebody in the bed.


# Resources

* [The Arduino Code](../alarmblanket/intelletto_inbed_detector/inbed_detector/)
