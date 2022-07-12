#ifndef SETTINGS_H
#define SETTINGS_H

#define M_PI					3.14159265
#define D_PI 					6.28318531
#define MAX_SPEED				12.56 // rad/sec (2 rev/sec)
#define ENCODER_STEPS_COUNT		127
#define HOLD_RESET_OVR_TACTS	10
//PI-reg speed
#define K_11					178.209
#define K_12					509.725
//PI-reg amp
#define K_21					26.537
#define K_22					231.562
//P-reg angle
#define K_FI					6
//compensation of crossconnection
#define CROSS_K					0.00417
#define K_CF					1
//calculate period (sec)
#define TIME_CALC				0.001

//space vector PWM
#define SVPWM					10
//STAND
#define STAND					10

#endif
