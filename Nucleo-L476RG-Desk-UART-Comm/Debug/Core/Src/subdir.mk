################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/card_rfid.c \
../Core/Src/data_handler.c \
../Core/Src/desk_control.c \
../Core/Src/lab_deck.c \
../Core/Src/led_controller.c \
../Core/Src/main.c \
../Core/Src/power_relay.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/store_flash_data.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c 

OBJS += \
./Core/Src/card_rfid.o \
./Core/Src/data_handler.o \
./Core/Src/desk_control.o \
./Core/Src/lab_deck.o \
./Core/Src/led_controller.o \
./Core/Src/main.o \
./Core/Src/power_relay.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/store_flash_data.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o 

C_DEPS += \
./Core/Src/card_rfid.d \
./Core/Src/data_handler.d \
./Core/Src/desk_control.d \
./Core/Src/lab_deck.d \
./Core/Src/led_controller.d \
./Core/Src/main.d \
./Core/Src/power_relay.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/store_flash_data.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/card_rfid.cyclo ./Core/Src/card_rfid.d ./Core/Src/card_rfid.o ./Core/Src/card_rfid.su ./Core/Src/data_handler.cyclo ./Core/Src/data_handler.d ./Core/Src/data_handler.o ./Core/Src/data_handler.su ./Core/Src/desk_control.cyclo ./Core/Src/desk_control.d ./Core/Src/desk_control.o ./Core/Src/desk_control.su ./Core/Src/lab_deck.cyclo ./Core/Src/lab_deck.d ./Core/Src/lab_deck.o ./Core/Src/lab_deck.su ./Core/Src/led_controller.cyclo ./Core/Src/led_controller.d ./Core/Src/led_controller.o ./Core/Src/led_controller.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/power_relay.cyclo ./Core/Src/power_relay.d ./Core/Src/power_relay.o ./Core/Src/power_relay.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/store_flash_data.cyclo ./Core/Src/store_flash_data.d ./Core/Src/store_flash_data.o ./Core/Src/store_flash_data.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su

.PHONY: clean-Core-2f-Src

