################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/can_usr.c \
../Src/freertos.c \
../Src/lcd.c \
../Src/main.c \
../Src/nextion_hardware.c \
../Src/steer.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_hal_timebase_tim.c \
../Src/stm32l4xx_it.c \
../Src/syscalls.c \
../Src/system_stm32l4xx.c \
../Src/uart.c 

OBJS += \
./Src/can_usr.o \
./Src/freertos.o \
./Src/lcd.o \
./Src/main.o \
./Src/nextion_hardware.o \
./Src/steer.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_hal_timebase_tim.o \
./Src/stm32l4xx_it.o \
./Src/syscalls.o \
./Src/system_stm32l4xx.o \
./Src/uart.o 

C_DEPS += \
./Src/can_usr.d \
./Src/freertos.d \
./Src/lcd.d \
./Src/main.d \
./Src/nextion_hardware.d \
./Src/steer.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_hal_timebase_tim.d \
./Src/stm32l4xx_it.d \
./Src/syscalls.d \
./Src/system_stm32l4xx.d \
./Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32L432xx -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Inc" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Drivers/STM32L4xx_HAL_Driver/Inc" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/rkama/OneDrive/Documents/GitHub/lcd_dashboard/lcd_steering/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


