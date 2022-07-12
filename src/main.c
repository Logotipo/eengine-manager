#include <stdint.h>
#include <math.h>
#include <stm32f10x_gpio.h>
#include "rcc.h"
#include "usart.h"
#include "current_speed.h"
#include "calc_funcs.h"
#include "dio.h"
#include "settings.h"


double set_motor_speed = 0;//rad/sec
double set_motor_angle = 0;//rad
double current_motor_speed = 0;//rad/sec
double current_motor_angle = 0;//rad
double motor_angle_offset = 0;//rad

//////////////////////////////////
double integr_value_omega = 0;
double integr_value_current_d = 0;
double integr_value_current_q = 0;
//////////////////////////////////
double D_save = 0;
double Q_save = 0;
//////////////////////////////////
uint8_t clock_state = 0;

uint8_t hold_reset_overloaded_channel = 0;
uint8_t manage_mode = 0;//0 - angle, 1 - speed

uint8_t mainTimFlag = 0;

uint16_t initAngleOffsetStep = 0;


int main(void){
	//init system
	SetSysClock72();
	UART_Init();
	InitCheckSpeed();
#if SVPWM
	InitPWMTIMs();
#endif
	InitGPIO();
	/////////////
	//init angle offset
	initAngleOffsetStep = 0;
	TIM2->CCR1 = 0xFFFF; // 100% set orientation rotor to phase +A
	TIM2->CCR2 = 0;
	TIM2->CCR3 = 0;
	TIM2->CCR4 = 0;
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	while(initAngleOffsetStep < 2000){ // wait 2 seconds
		if(mainTimFlag){
			mainTimFlag = 0;
			initAngleOffsetStep++;
		}
	}
	UpdateSpeedAndAngle();
	motor_angle_offset = current_motor_angle;
	/////////////
	//main loop
	while(1){
		if(hold_reset_overloaded_channel){
			hold_reset_overloaded_channel--;
			if(!hold_reset_overloaded_channel)
				GPIO_WriteBit(GPIOA, GPIO_Pin_14, Bit_RESET);//remove EN-signal
		}
		if(mainTimFlag){
			mainTimFlag = 0;//reset flag
#if !SVPWM
			clock_state ^= 1;//switch 1-0
#endif
			
			UpdateSpeedAndAngle();//get current angle and speed
			double omega_u = 0;
			if(0 == manage_mode){
				float angle_u = set_motor_angle - current_motor_angle;//signal of manage
				angle_u *= K_FI;//P-regulate of angle
				omega_u = angle_u - current_motor_speed;
			}
			else omega_u = set_motor_speed - current_motor_speed;//signal of manage
			omega_u = PI_reg(omega_u, K_11, K_12, &integr_value_omega);
			///D, Q regulation
			double D = PI_reg(0 - D_save, K_21, K_22, &integr_value_current_d);
			double Q = PI_reg(omega_u - D_save, K_21, K_22, &integr_value_current_q);
			//////////////////
			///compesation of crossconnection
			//compensation_of_crossconnection(&D, &Q, current_motor_speed);
			/////////////////////////////////
			///rotor to stator angles
			double alpha, beta;//stator angles
			rotor_to_stator(D, Q, &alpha, &beta, current_motor_angle);
			/////////////////////////
			//2p_to_3p
			double A, B, C;
			two_p_to_winding(&A, &B, &C, alpha, beta);
#if SVPWM
			/* space vector PWM */
			double Z, Z_angle;
			uint16_t ccrs[3];
			getResVector(A, B, C, &Z, &Z_angle);
			uint8_t sector = getSector(Z_angle);
			getCCR(Z, Z_angle, sector, &ccrs[0]);
			///set CCR for PWM
			TIM2->CCR1 = ccrs[0] - 0xCCD;// -5% protection for keys
			TIM2->CCR2 = ccrs[0] + 0xCCD;// +5% protection for keys
			TIM2->CCR3 = ccrs[1] - 0xCCD;// -5% protection for keys
			TIM2->CCR4 = ccrs[1] + 0xCCD;// +5% protection for keys
			TIM3->CCR1 = ccrs[2] - 0xCCD;// -5% protection for keys
			TIM3->CCR2 = ccrs[2] + 0xCCD;// +5% protection for keys
			/////////////////////////
#else
			//////////
			///write signal of manage on motor driver
			GPIO_Write(GPIOA, GetKeysCode(A, B, C, clock_state));
			/////////////////////////////////////////
#endif
			///Save D and Q
			D_save = D;
			Q_save = Q;
		}
	}
	///////////
}
