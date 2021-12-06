#include "twin_feedback_rotation_servo.h"

#define SERIAL_SERVO_BYTE_TYPE 0x00
#define SERIAL_SERVO_FLOAT_TYPE 0x01
#define SERIAL_SERVO_BAUD_RATE 115200
#define SERIAL_SERVO_TOTAL_COMMUNICATE_BYTES 14
#define SERIAL_SERVO_MAX_FALSE_COUNT 255
#define SERIAL_SERVO_TIME_OUT 500 //0.5sec

static const byte header[]
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
static const byte writing_data_description[] 
{
    0x08, 0x04, 0x02, SERIAL_SERVO_FLOAT_TYPE
};

ServoSet servo_set;
WritingData writing_data;
Commands commands;
unsigned long time_last_cmd;

extern HardwareSerial Serial;

void readCommands()
{
    byte header_count(0);
    byte false_count(0);
    while(header_count < 8)
    {
        if(Serial.read() == 0xff)
        {
            header_count++;
        }
        else
        {
            header_count = 0;
            false_count++;
            if(false_count >= SERIAL_SERVO_MAX_FALSE_COUNT)
            {
                //return nothing ??
                return;
            }
        }
    }

    if(
        Serial.read() == 0x02 && Serial.read() == 0x01 &&
        Serial.read() == 0x02 && Serial.read() == SERIAL_SERVO_BYTE_TYPE 
    )
    {
        byte cmd_upper(Serial.read());
        byte cmd_lower(Serial.read());
        if(
            cmd_upper >= ROTATION_SERVO_CCW_MAX && 
            cmd_upper <= ROTATION_SERVO_CW_MAX
        )
        {
            commands.command_upper = cmd_upper;
        }
        if(
            cmd_lower >= ROTATION_SERVO_CCW_MAX &&
            cmd_lower <= ROTATION_SERVO_CW_MAX
        )
        {
            commands.command_lower = cmd_lower;    
        }
    }
}

void setup()
{
    servo_set.upper_servo_ptr = new Servo();
    servo_set.lower_servo_ptr = new Servo();
    servoInit(&servo_set);

    commands.command_upper = ROTATION_SERVO_STOPPING;
    commands.command_lower = ROTATION_SERVO_STOPPING;
    time_last_cmd = 0;

    Serial.begin(SERIAL_SERVO_BAUD_RATE);

    while (!Serial)
    {
        delay(10);
    }
}

void loop()
{
    if(Serial)
    {
        Serial.flush();

        unsigned long time_now(millis());
        // just in case of overflow
        if(time_now < time_last_cmd)
        {
            time_last_cmd = 0;
        }
  
        if(Serial.available() >= SERIAL_SERVO_TOTAL_COMMUNICATE_BYTES)
        {
            readCommands();
            servo_set.upper_servo_ptr->write(commands.command_upper);
            servo_set.lower_servo_ptr->write(commands.command_lower);
            time_last_cmd = time_now;

            //60fps loop
            delay(15);
        }

        if(time_now - time_last_cmd > SERIAL_SERVO_TIME_OUT)
        {
            prepareExit(&servo_set);
        }

        readFeedBack(&writing_data);
        
        Serial.write(
            header,
            8
        );
        Serial.write(
            writing_data_description,
            4
        );
        // for test
        writing_data.feedbacks.upper_feedback = (float)commands.command_upper;
        writing_data.feedbacks.lower_feedback = (float)commands.command_lower;
        Serial.write(
            writing_data.bin,
            8
        );
    }
    else
    {
        prepareExit(&servo_set);
    }
}
