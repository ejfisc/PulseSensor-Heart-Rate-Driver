/* ****************************************************************************/
/** Heart Rate Sensor Driver

  @File Name
    HeartRate.h

  @Summary
    Calculates BPM based on analog signal

  @Description
    Defines functions that allow the user to interact with heart rate sensor
******************************************************************************/

#ifndef HEART_RATE_H
#define HEART_RATE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


//#define DEBUG_OUTPUT // Uncomment if you want to use (depends on printf in stdio.h)
#ifdef DEBUG_OUTPUT
#include "nrf_log.h"
#define LOG(...) NRF_LOG_INFO(__VA_ARGS__) // change to printf when going back to nordic
#endif

typedef struct {
    // pulse detection output variables
    float signal; // latest voltage signal from ADC, update every time a new sample is ready
    uint8_t BPM; // beats per minute, updated every sample
    uint32_t IBI; // inter beat interval, time interval (ms) between beats
    bool pulse; // True when heartbeat detected
    bool start_of_beat; // True when start of heart beat is detected
    float thresh_setting; // used to seed and reset thresh variable !* must be initialized *!
    float amplitude; // amplitude of pulse waveform
    uint64_t last_beat_time;

    // pulse detection internal variables
    uint32_t rate[10]; // array to hold last 10 IBI values (ms)
    uint64_t sample_counter; // determines pulse timing, ms since start
    uint64_t N; // used to monitor duration between beats
    float peak; // peak in pulse wave, (sample value)
    float trough; // trough in pulse wave, sample value
    float thresh; // instant moment of heart beat, sample value
    bool first_beat; // used to seed rate array so we start with reasonable BPM
    bool second_beat;
}pulse_sensor_t;

/*
    @brief heart rate sensor initialization
    @note sets default variables
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void heart_rate_init(pulse_sensor_t * Pulse_Sensor);

/*
    @brief reset variables to default
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void reset_variables(pulse_sensor_t * Pulse_Sensor);

/*
    @brief update threshold variable
    @note this value will be used to seed the thresh variable
    @param Pulse Sensor Pointer to pulse sensor handler
    @param threshold sample value to indicate heart beat
    @retval None
*/
void set_threshold(pulse_sensor_t * Pulse_Sensor, float threshold);

/*
    @brief get the latest pulse sensor sample
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval signal value
*/
float get_latest_sample(pulse_sensor_t * Pulse_Sensor);

/*
    @brief get the current bpm measurement
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval BPM value
*/
uint8_t get_beats_per_minute(pulse_sensor_t * Pulse_Sensor);

/*
    @brief get the current inter-beat interval
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval IBI value
*/
uint32_t get_inter_beat_interval(pulse_sensor_t * Pulse_Sensor);

/*
    @brief reads and clears saw start of beat flag
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval start_of_beat value
*/
bool saw_start_of_beat(pulse_sensor_t * Pulse_Sensor);

/*
    @brief returns true if the pulse sensor is inside of a heart beat
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval pulse value
*/
bool is_inside_beat(pulse_sensor_t * Pulse_Sensor);

/*
    @brief get latest amplitude value
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval amplitude value
*/
float get_pulse_amplitude(pulse_sensor_t * Pulse_Sensor);

/*
    @brief returns sample number of most recently detected pulse
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval last_beat_time value
*/
uint32_t get_last_beat_time(pulse_sensor_t * Pulse_Sensor);

/*
    @brief processes the latest sample value
    @note calculates BPM, IBI, etc.
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void pulse_sensor_process_sample(pulse_sensor_t * Pulse_Sensor, uint32_t ms);

#endif // HEART_RATE_H