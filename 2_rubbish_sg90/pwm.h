#ifndef _PWM_H_
#define _PWM_H_

#define PWM_OTHER_RUBBISH       7
#define PWM_RECOVERABLE_RUBBISH 5

void pwm_write(int pwm_pin);

void pwm_stop(int pwm_pin);

#endif