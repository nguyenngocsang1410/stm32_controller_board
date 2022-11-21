################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/user_code/BUTTON.c \
../Core/Src/user_code/CLCD_I2C.c \
../Core/Src/user_code/UserCode.c 

OBJS += \
./Core/Src/user_code/BUTTON.o \
./Core/Src/user_code/CLCD_I2C.o \
./Core/Src/user_code/UserCode.o 

C_DEPS += \
./Core/Src/user_code/BUTTON.d \
./Core/Src/user_code/CLCD_I2C.d \
./Core/Src/user_code/UserCode.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/user_code/%.o Core/Src/user_code/%.su: ../Core/Src/user_code/%.c Core/Src/user_code/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Core/Src/Test -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-user_code

clean-Core-2f-Src-2f-user_code:
	-$(RM) ./Core/Src/user_code/BUTTON.d ./Core/Src/user_code/BUTTON.o ./Core/Src/user_code/BUTTON.su ./Core/Src/user_code/CLCD_I2C.d ./Core/Src/user_code/CLCD_I2C.o ./Core/Src/user_code/CLCD_I2C.su ./Core/Src/user_code/UserCode.d ./Core/Src/user_code/UserCode.o ./Core/Src/user_code/UserCode.su

.PHONY: clean-Core-2f-Src-2f-user_code

