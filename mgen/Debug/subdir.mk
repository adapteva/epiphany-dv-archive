################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../main.cpp 

OBJS += \
./main.o 

CPP_DEPS += \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
main.o: ../main.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.1 -I"/home/oraikhman/dv-workspace/igen" -I"/usr/local/systemc/systemc-2.2/include" -O0 -c -fPIC -I../../igen -MMD -MP -MF"$(@:%.o=%.d)" -MT"main.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


