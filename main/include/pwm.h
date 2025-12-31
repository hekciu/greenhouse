#ifndef PWM_H
#define PWM_H


esp_err_t initialize_pwm();
void * use_pwm_task(void * data);
esp_err_t set_pwm(uint8_t percent);


#endif
