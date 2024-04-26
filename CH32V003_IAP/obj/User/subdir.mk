################################################################################
# MRS Version: 1.9.1
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v00x_it.c \
../User/flash.c \
../User/iap.c \
../User/main.c \
../User/system_ch32v00x.c 

OBJS += \
./User/ch32v00x_it.o \
./User/flash.o \
./User/iap.o \
./User/main.o \
./User/system_ch32v00x.o 

C_DEPS += \
./User/ch32v00x_it.d \
./User/flash.d \
./User/iap.d \
./User/main.d \
./User/system_ch32v00x.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\BMSCE\3rd year\6th Sem\Internship\VSD\Downloads\CH32V003EVT\EVT\EXAM\SRC\Debug" -I"D:\BMSCE\3rd year\6th Sem\Internship\VSD\Downloads\CH32V003EVT\EVT\EXAM\SRC\Core" -I"D:\BMSCE\3rd year\6th Sem\Internship\VSD\Downloads\CH32V003EVT\EVT\EXAM\USART_IAP\CH32V003_IAP\User" -I"D:\BMSCE\3rd year\6th Sem\Internship\VSD\Downloads\CH32V003EVT\EVT\EXAM\SRC\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

