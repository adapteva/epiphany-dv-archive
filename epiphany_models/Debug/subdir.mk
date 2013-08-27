################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Sp.cpp \
../monitor.cpp \
../namedPipeMemServer.cpp \
../sc_main.cpp \
../stimulus.cpp \
../verilated.cpp 

OBJS += \
./Sp.o \
./monitor.o \
./namedPipeMemServer.o \
./sc_main.o \
./stimulus.o \
./verilated.o 

CPP_DEPS += \
./Sp.d \
./monitor.d \
./namedPipeMemServer.d \
./sc_main.d \
./stimulus.d \
./verilated.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.1 -DRSP_TRACE -DDEBUG____ -I../ -I../../e -I/soft/verilator-3.813/include -I/soft/SystemPerl-1.335/src -I"/soft/systemc-2.2.0/include" -O0 -g3 -c  -MMD   -DDUMPVCD=1 -DDV_FAKECLK -DDV_FAKELAT -DDV_FAKEIO -DVL_PRINTF=printf -DVM_TRACE=1  -DSYSTEMPERL -DUTIL_PRINTF=sp_log_printf -Wno-deprecated -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


