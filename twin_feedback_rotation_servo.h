#ifndef FEEDBACK_ROTATION_SERVO_H_
#define FEEDBACK_ROTATION_SERVO_H_

#include "Arduino.h"
#include "Servo.h"

#define SERVO_SET_UPPER_PIN 10
#define SERVO_SET_LOWER_PIN 9

#define SERVO_PULSE_MAX_UPPER 2000
#define SERVO_PULSE_MIN_UPPER 1000
#define SERVO_PULSE_MAX_LOWER 2000
#define SERVO_PULSE_MIN_LOWER 1000

#define ROTATION_SERVO_STOPPING 90
#define ROTATION_SERVO_CW_MAX 180
#define ROTATION_SERVO_CCW_MAX 0

#define RAW_2_VOL 0.0047483381f //5.0v / 1053
#define VOL_2_ANG 109.0909090909f //360deg / 3.3v
#define ONE_ROUND_DEG 360.0f

struct ServoSet
{
    Servo* upper_servo_ptr;
    Servo* lower_servo_ptr;
};

struct Feedbacks
{
    float upper_feedback;
    float lower_feedback;
};

struct Commands
{
    uint8_t command_upper;
    uint8_t command_lower;
};

union WritingData
{
    struct Feedbacks feedbacks;
    uint8_t bin[8];
};

void prepareExit(const ServoSet* servo_set_ptr)
{
    servo_set_ptr->upper_servo_ptr->write(ROTATION_SERVO_STOPPING);
    servo_set_ptr->lower_servo_ptr->write(ROTATION_SERVO_STOPPING);
}

void servoInit(const ServoSet* servo_set_ptr)
{
    servo_set_ptr->lower_servo_ptr->attach(
        SERVO_SET_LOWER_PIN,
        SERVO_PULSE_MAX_LOWER,
        SERVO_PULSE_MIN_LOWER
    );
    servo_set_ptr->upper_servo_ptr->attach(
        SERVO_SET_UPPER_PIN,
        SERVO_PULSE_MAX_UPPER, 
        SERVO_PULSE_MIN_UPPER
    );
    servo_set_ptr->lower_servo_ptr->write(ROTATION_SERVO_STOPPING);
    servo_set_ptr->upper_servo_ptr->write(ROTATION_SERVO_STOPPING);
}

void readFeedBack(WritingData* writing_data)
{
    int raw_lower(analogRead(PIN_A0));
    int raw_upper(analogRead(PIN_A1));
    float volt_lower(raw_lower * RAW_2_VOL);
    float volt_upper(raw_upper * RAW_2_VOL); 
    float angle_lower(volt_lower * VOL_2_ANG);
    float angle_upper(volt_upper * VOL_2_ANG);
    
    if(angle_lower > ONE_ROUND_DEG)
    {
        angle_lower -= ONE_ROUND_DEG;
    }
    if(angle_upper > ONE_ROUND_DEG)
    {
        angle_upper -= ONE_ROUND_DEG;
    }

    writing_data->feedbacks.lower_feedback = angle_lower;
    writing_data->feedbacks.upper_feedback = angle_upper;
}

#endif
