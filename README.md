# Nodemcu MQTT Simple device alarm

This is a simple Nodemcu project for write an alarm message into a MQTT broker, the example is built for a pool pump device.
It have also a MQTT listener for reset the alarm status.

This project has been developed with [PlatformIo framework](https://platformio.org/)


JSON object published by the producer:
```json
{ 
    "device": "Pool pump",
    "alarm": "High Amperage", 
    "status":"0"
}
```

JSON object for reset the device:
```json
{
    "device":"Pool pump",
    "reset":true
}
```

When an alarm is fired a relay switch on, use the normally closed contact in your device power supply or dry contact.
