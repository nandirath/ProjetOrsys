################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LCD.cpp \
../broker.cpp 

OBJS += \
./LCD.o \
./broker.o 

CPP_DEPS += \
./LCD.d \
./broker.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I/home/nandi/rootfs/usr/local/include -I/home/nandi/rootfs/usr/include/arm-linux-gnueabihf -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


