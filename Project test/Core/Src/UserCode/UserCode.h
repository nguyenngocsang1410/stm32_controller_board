#ifndef USER_CODE_H_
#define USER_CODE_H_
#include "CLCD_I2C.h"

extern I2C_HandleTypeDef hi2c2;

void setup();
void loop();

void cbShort();
void cbLong();
void cbDouble();
#endif
