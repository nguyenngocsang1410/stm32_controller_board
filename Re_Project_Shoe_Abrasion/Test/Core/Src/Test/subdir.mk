################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Test/test_handler.c 

OBJS += \
./Core/Src/Test/test_handler.o 

C_DEPS += \
./Core/Src/Test/test_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Test/%.o Core/Src/Test/%.su: ../Core/Src/Test/%.c Core/Src/Test/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Core/Src/Test -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Test

clean-Core-2f-Src-2f-Test:
	-$(RM) ./Core/Src/Test/test_handler.d ./Core/Src/Test/test_handler.o ./Core/Src/Test/test_handler.su

.PHONY: clean-Core-2f-Src-2f-Test

