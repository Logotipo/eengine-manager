#include <math.h>
#include <stdint.h>
#include "calc_funcs.h"
#include "settings.h"

void winding_to_2p(WINDING_VECTORS *wv, int16_t *alpha, int16_t *beta){
	*beta  = (wv->B - wv->C) * (1/sqrt(3));
	*alpha = wv->A;
}

void stator_to_rotor(double alpha, double beta, double *d, double *q, double current_motor_angle){
	//cos and sin
	double sin_val = sin(current_motor_angle);
	double cos_val = cos(current_motor_angle);
	/////////////
	//calculate values
	*d = (sin_val * alpha) - (cos_val * beta);
	*q = (sin_val * beta) + (cos_val * alpha);
	//////////////////
}

void rotor_to_stator(double d, double q, double *alpha, double *beta, double current_motor_angle){
	//cos and sin
	double sin_val = sin(current_motor_angle);
	double cos_val = cos(current_motor_angle);
	/////////////
	//calculate values
	*alpha = (sin_val * d) + (cos_val * q);
	*beta  = (sin_val * q) - (cos_val * d);
	//////////////////
}

void two_p_to_winding(double *A, double *B, double *C, double alpha, double beta){
	*A = alpha;
	*B = ((beta * sqrt(3)) - alpha) / 2;
	*C = (-1 *((beta * sqrt(3))) - alpha) / 2;
}

double PI_reg(double input, double k1, double k2, double *integr_value){
	*integr_value += input * TIME_CALC;//integration
	return (input * k1 + k2 * (*integr_value));
}

void compensation_of_crossconnection(double *d, double *q, double current_motor_speed){
	double save_d = *d;
	double save_q = *q;
	*d = (1/K_CF)*(save_d - current_motor_speed * 16 * save_q * CROSS_K);
	*q = (1/K_CF)*(save_q + current_motor_speed * 16 * save_q * CROSS_K);
}

//already shifted!!!
uint16_t GetKeysCode(double a, double b, double c, int8_t clock_signal){
	uint16_t result = 0;
	a -= clock_signal;
	b -= clock_signal;
	c -= clock_signal;
	//////////////////
	if(a > 0)
		result |= 0x01;
	else
		result |= 0x02;
	if(b > 0)
		result |= 0x04;
	else
		result |= 0x08;
	if(c > 0)
		result |= 0x10;
	else
		result |= 0x20;
	
	return result;
}
#if SVPWM
void getResVector(double a, double b, double c, double *z, double *angle){
	double BC = sqrt(b * b + c * c - 2 * b * c * cos(M_PI - 2.0944));
	double BC_angle = asin(c * sin(M_PI - 2.0944) / BC);
	*z = sqrt(a * a + BC * BC - 2 * a * BC * cos(M_PI - 2.0944 + BC_angle));
	*angle = asin(BC * sin(M_PI - 2.0944 + BC_angle) / (*z));
}
uint8_t getSector(double angle){
	if(angle >= 0 && angle <= M_PI/3)
		return 1;
	else if(angle > M_PI/3 && angle <= 2/3*M_PI)
		return 2;
	else if(angle > 2/3*M_PI && angle <= M_PI)
		return 3;
	else if(angle > M_PI && angle <= 4/3*M_PI)
		return 4;
	else if(angle > 4/3*M_PI && angle <= 5/3*M_PI)
		return 5;
	else if(angle > 5/3*M_PI && angle <= D_PI)
		return 6;
	else
		return 0;
}
/*angles of generating vectors*/
const double vect_angles[6][2] = {
	0,			M_PI/3,
	M_PI/3,		2/3*M_PI,
	2/3*M_PI,	M_PI,
	M_PI,		4/3*M_PI,
	4/3*M_PI,	5/3*M_PI,
	5/3*M_PI,	2*M_PI
};
void getCCR(double z, double z_angle, uint8_t sector, uint16_t *ccrs){
	double tau[3] = {0, 0, 0};
	double tau_f[3];
	tau[0] = sin(vect_angles[sector][1] - z_angle)/0.8660025;
	tau[1] = sin(z_angle - vect_angles[sector][0])/0.8660025;
	tau[2] = 1 - (tau[0] + tau[1]);
	switch(sector){
		case 1:
			tau_f[0] = tau[0] + tau[1] + tau[2]/2;
			tau_f[1] = tau[1] + tau[2]/2;
			tau_f[2] = tau[2]/2;
			break;
		case 2:
			tau_f[0] = tau[0] + tau[2]/2;
			tau_f[1] = tau[1] + tau[0] + tau[2]/2;
			tau_f[2] = tau[2]/2;
			break;
		case 3:
			tau_f[0] = tau[2]/2;
			tau_f[1] = tau[0] + tau[1] + tau[2]/2;
			tau_f[2] = tau[1] + tau[2]/2;
			break;
		case 4:
			tau_f[0] = tau[2]/2;
			tau_f[1] = tau[0] + tau[2]/2;
			tau_f[2] = tau[0] + tau[1] + tau[2]/2;
			break;
		case 5:
			tau_f[0] = tau[1] + tau[2]/2;
			tau_f[1] = tau[2]/2;
			tau_f[2] = tau[0] + tau[1] + tau[2]/2;
			break;
		case 6:
			tau_f[0] = tau[0] + tau[1] + tau[2]/2;
			tau_f[1] = tau[2]/2;
			tau_f[2] = tau[1] + tau[2]/2;
			break;
		default:
			tau_f[0] = 0;
			tau_f[1] = 0;
			tau_f[2] = 0;
			break;
	}
	ccrs[0] = tau_f[0] * 0xFFFF;
	ccrs[1] = tau_f[1] * 0xFFFF;
	ccrs[2] = tau_f[2] * 0xFFFF;
}
#endif
