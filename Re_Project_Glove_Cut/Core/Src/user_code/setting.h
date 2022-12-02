#ifndef _SETTING_H_
#define _SETTING_H_

#define LCD_dly 300
#define LCD_dly_short 150

#define BTN_Start_Pin X3_Pin
#define BTN_Start_GPIO_Port X3_GPIO_Port
#define BTN_Stop_Pin X4_Pin
#define BTN_Stop_GPIO_Port X4_GPIO_Port
#define BTN_Mode_Pin X5_Pin
#define BTN_Mode_GPIO_Port X5_GPIO_Port

#define LED_Panel_Pin Q0_Pin
#define LED_Panel_GPIO_Port Q0_GPIO_Port

#define Q0_Pin Y6_Pin
#define Q0_GPIO_Port Y6_GPIO_Port
#define Q1_Pin Y7_Pin
#define Q1_GPIO_Port Y7_GPIO_Port
#define Q2_Pin Y10_Pin
#define Q2_GPIO_Port Y10_GPIO_Port
#define Q3_Pin Y11_Pin
#define Q3_GPIO_Port Y11_GPIO_Port

//#define M1_L_Pin M1_L_Pin
//#define M1_L_GPIO_Port M1_L_GPIO_Port
//#define M1_R_Pin M1_R_Pin
//#define M1_R_GPIO_Port M1_R_GPIO_Port

#define M2B_L_Pin Y4_Pin
#define M2B_L_GPIO_Port Y4_GPIO_Port
#define M2B_R_Pin Y5_Pin
#define M2B_R_GPIO_Port Y5_GPIO_Port

#define M2B_Channel TIM4->CCR3

//#define M1A_Channel TIM3->CCR1
//#define M1B_Channel TIM3->CCR2

#define PidInterval 20

extern float kp, ki, kd;

#define menu1_value mSpeed
#define menu2_value mSec
#define menu3_value mNumLoopCount

/* Speed */
#define minMenu1Value 30 //rpm
#define maxMenu1Value 60

/* Second */
#define minMenu2Value 10
#define maxMenu2Value 2*60*60 // 2 hours

/* Revolutions */
#define minMenu3Value 1
#define maxMenu3Value 99

#define minMenu4Value 30
#define maxMenu4Value 70

#define ecdFilter 6

#define pulsePerRev 11
#define gearRatio 270
#define encoderMul 4

#define totalPulse pulsePerRev*gearRatio*encoderMul
#endif
