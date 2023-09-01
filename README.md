# PulseSensor Heart Rate Driver

## Getting Started
Create a `pulse_sensor_t` instance in main and give it a `thresh_setting` (this may change per user). Call `heart_rate_init()`. The PulseSensor outputs an analog pulse signal, so you'll need an ADC to convert the output voltage from the sensor to a digital value the micro can use. 

Put this in your super loop in main (psuedocode):
```
if(adc_data_ready) {
    pulse_sensor.signal = adc_data; // pulse_sensor is the pulse_sensor_t instance
    pulse_sensor_process_sample(&pulse_sensor, time);
    adc_data_ready = false;
}
```

`time` is the time interval between process sample function calls, this is used to track the amount of time that has passed and calculating beats per minute, to ensure accuracy I suggest using a capture compare timer task.

## Debug Output
A precompiler directive is used to turn debug output on and off. Currently the `LOG()` macro is defined to use `NRF_LOG_INFO` which is a Nordic nRF5 SDK specific function, modify this and the includes to fit your micro and environment. 