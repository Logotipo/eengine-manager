#ifndef CALC_FUNCS_H
#define CALC_FUNCS_H
#include <stdint.h>

typedef __packed struct{
	uint16_t A;
	uint16_t B;
	uint16_t C;
}WINDING_VECTORS;

void winding_to_2p(WINDING_VECTORS *wv, int16_t *alpha, int16_t *beta);
void stator_to_rotor(double alpha, double beta, double *d, double *q, double current_motor_angle);
void rotor_to_stator(double d, double q, double *alpha, double *beta, double current_motor_angle);
void two_p_to_winding(double *A, double *B, double *C, double alpha, double beta);
double PI_reg(double input, double k1, double k2, double *integr_value);
void compensation_of_crossconnection(double *d, double *q, double current_motor_speed);
uint16_t GetKeysCode(double a, double b, double c, int8_t clock_signal);


void getResVector(double a, double b, double c, double *z, double *angle);
uint8_t getSector(double angle);
void getCCR(double z, double z_angle, uint8_t sector, uint16_t *ccrs);


#endif
