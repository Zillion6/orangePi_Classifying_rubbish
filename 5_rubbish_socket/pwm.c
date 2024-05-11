#include <wiringPi.h>
#include <softPwm.h>

void pwm_write(int pwm_pin)
{
    pinMode(pwm_pin, OUTPUT);
    softPwmCreate(pwm_pin, 0, 200);
    softPwmWrite(pwm_pin, 25);
    delay(1000);
    softPwmStop(pwm_pin);
}

void pwm_stop(int pwm_pin)
{
    pinMode(pwm_pin, OUTPUT);
    softPwmCreate(pwm_pin, 0, 200);
    softPwmWrite(pwm_pin, 5);
    delay(1000);
    softPwmStop(pwm_pin);
}