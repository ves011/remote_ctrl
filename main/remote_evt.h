/*
 * joystick.h
 *
 *  Created on: Jan 2, 2025
 *      Author: viorel_serbu
 */

#ifndef MAIN_JOYSTICK_H_
#define MAIN_JOYSTICK_H_


#define JS_RIGHT_CHANNEL 	0			// 	joystick right channel ADC
#define JS_LEFT_CHANNEL 	1			// 	joystick left channel ADC 

#define JS_POLL_INT			20000		//	poll interval usec
#define JS_ADC_SAMPLES		7			// 	no of adc values to be taken on each poll

#define JS_THERESHOLD		0.975

#define JSTIMER_MSG			1

void init_remote_event();

typedef struct
	{
	int msg_type;
	int val;
	}evt_message_t;
	
#endif /* MAIN_JOYSTICK_H_ */
