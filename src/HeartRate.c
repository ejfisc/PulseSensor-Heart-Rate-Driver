/* ****************************************************************************/
/** Heart Rate Sensor Driver

  @File Name
    HeartRate.c

  @Summary
    Calculates BPM based on analog signal

  @Description
    Implements functions that allow the user to interact with heart rate sensor
******************************************************************************/

#include "HeartRate.h"
#include <stdlib.h>
#include <string.h>

/*
    @brief heart rate sensor initialization
    @note sets default variables
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void heart_rate_init(pulse_sensor_t * PS) {
    reset_variables(PS);
}

/*
    @brief reset variables to default
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void reset_variables(pulse_sensor_t * PS) {
    memset(PS->rate, 0, 10);
    PS->start_of_beat = false;
    PS->BPM = 0;
    PS->IBI = 750; // 750 ms per beat = 80 bpm
    PS->pulse = false;
    PS->sample_counter = 0;
    PS->last_beat_time = 0;
    PS->peak = 0.6; // peak at 1/2 input range 0-1.2V
    PS->trough = 0.6; // trough at 1/2 input range 0-1.2V
    PS->thresh = PS->thresh_setting;
    PS->amplitude = 0.12; // amp at 1/10 of input range 
    PS->first_beat = true; // looking for first beat
    PS->second_beat = false;
}

/*
    @brief update threshold variable
    @note this value will be used to seed the thresh variable
    @param Pulse Sensor Pointer to pulse sensor handler
    @param threshold sample value to indicate heart beat
    @retval None
*/
void set_threshold(pulse_sensor_t * PS, float threshold) {
    PS->thresh_setting = threshold;
    PS->thresh = threshold;
}

/*
    @brief get the latest pulse sensor sample
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval signal value
*/
float get_latest_sample(pulse_sensor_t * PS) {
    return PS->signal;
}

/*
    @brief get the current bpm measurement
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval BPM value
*/
uint8_t get_beats_per_minute(pulse_sensor_t * PS) {
    return PS->BPM;
}

/*
    @brief get the current inter-beat interval
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval IBI value
*/
uint32_t get_inter_beat_interval(pulse_sensor_t * PS) {
    return PS->IBI;
}

/*
    @brief reads and clears saw start of beat flag
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval start_of_beat value
*/
bool saw_start_of_beat(pulse_sensor_t * PS) {
    return PS->start_of_beat;
}

/*
    @brief returns true if the pulse sensor is inside of a heart beat
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval pulse value
*/
bool is_inside_beat(pulse_sensor_t * PS) {
    return PS->pulse;
}

/*
    @brief get latest amplitude value
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval amplitude value
*/
float get_pulse_amplitude(pulse_sensor_t * PS) {
    return PS->amplitude;
}

/*
    @brief returns sample number of most recently detected pulse
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval last_beat_time value
*/
uint32_t get_last_beat_time(pulse_sensor_t * PS) {
    return PS->last_beat_time;
}

/*
    @brief processes the latest sample value
    @note calculates BPM, IBI, etc.
    @param Pulse Sensor Pointer to pulse sensor handler
    @retval None
*/
void pulse_sensor_process_sample(pulse_sensor_t * PS, uint32_t ms) {
#ifdef DEBUG_OUTPUT
    LOG("sample: %.6f\n", PS->signal);
#endif
    PS->sample_counter += ms; // keep track of total time in ms
    PS->N = PS->sample_counter - PS->last_beat_time; // monitor time since last beat to avoid noise
#ifdef DEBUG_OUTPUT
    LOG("\tsample_counter (%d), last_beat_time (%d)\n", PS->sample_counter, PS->last_beat_time);
#endif

    // find the peak and trough of the pulse wave
    if(PS->signal < PS->thresh && PS->N > (PS->IBI/5)*3) { // avoid dichrotic noise by waiting 3/5 of last IBI
        if(PS->signal < PS->trough) {
            PS->trough = PS->signal; // keep track of lowest point in pulse wave
#ifdef DEBUG_OUTPUT
            LOG("\t\tTrough found: %.6f\n", PS->trough);
#endif
        }
    }

    if(PS->signal > PS->thresh && PS->signal > PS->peak) { // thresh condition helps avoid noise
        PS->peak = PS->signal; // keep track of highest point in pulse wave
#ifdef DEBUG_OUTPUT
        LOG("\t\tPeak found: %.6f\n", PS->peak);
#endif
    }

    // now look for heart beat, signal surges up everytime there is a pulse
    if(PS->N > 250) { // avoid high frequency noise
        if((PS->signal > PS->thresh) && (!PS->pulse) && (PS->N > (PS->IBI/5)*3)) {
            PS->pulse = true; // set the pulse flag when we think there is a pulse
            PS->IBI = PS->sample_counter - PS->last_beat_time; // measure time in between beats in ms
            PS->last_beat_time = PS->sample_counter; // update last beat time
#ifdef DEBUG_OUTPUT
            LOG("\t\tBeat found, updated IBI is %d, updated last_beat_time is %d\n", PS->IBI, PS->last_beat_time);
#endif

            if(PS->second_beat) {
                PS->second_beat = false;
                for(uint8_t i = 0; i < 10; i++) { // seed the running total to get a realistic BPM at startup
                    PS->rate[i] = PS->IBI;
                }
            }

            if(PS->first_beat) {
                PS->first_beat = false;
                PS->second_beat = true;
                return; // IBI value is unreliable so discard it
            }

            // keep a running total of the last 10 IBI values
            uint32_t running_total = 0;

            for(uint8_t i = 0; i < 9; i++) { // shift data into the rate array
                PS->rate[i] = PS->rate[i+1]; // drop the oldest IBI value
                running_total += PS->rate[i]; // sum the 9 oldest IBI values
            }

            PS->rate[9] = PS->IBI; // add latest IBI to rate array and take running average
            running_total += PS->IBI;
            running_total /= 10;
            PS->BPM = 60000 / running_total; // how many beats can fit into a minute?
            PS->start_of_beat = true; // we detected a beat, set start_of_beat flag
        }
    }

    // when the values are going down, the beat is over
    if(PS->signal < PS->thresh && PS->pulse) {
#ifdef DEBUG_OUTPUT
        LOG("\tBeat is over\n");
#endif
        PS->pulse = false;
        PS->amplitude = PS->peak - PS->trough; // get amplitude of pulse wave
        PS->thresh = PS->amplitude / 2 + PS->trough; // set threshold to 50% of amplitude
        PS->peak = PS->thresh; // reset these for next time
        PS->trough = PS->thresh;
    }

    // if 2.5 seconds go by without a beat
    if(PS->N > 2500) {
#ifdef DEBUG_OUTPUT
    LOG("\tTime since last beat (N = %d) is greater than 2.5 seconds, so reset variables\n", PS->N);
#endif
        PS->thresh = PS->thresh_setting;
        PS->peak = 0.6;
        PS->trough = 0.6;
        PS->last_beat_time = PS->sample_counter; // bring last beat time up to date
        PS->first_beat = true;
        PS->second_beat = false;
        PS->start_of_beat = false;
        PS->BPM = 0;
        PS->IBI = 600; // 600ms per beat = 100 bpm
        PS->pulse = false;
        PS->amplitude = 0.12;
    }
}
