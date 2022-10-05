################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SGAttackOperation.cpp \
../SGLPPMOperation.cpp \
../SGMetric.cpp \
../community.cpp \
../graph.cpp \
../graph_binary.cpp \
../main.cpp 

OBJS += \
./SGAttackOperation.o \
./SGLPPMOperation.o \
./SGMetric.o \
./main.o 

CPP_DEPS += \
./SGAttackOperation.d \
./SGLPPMOperation.d \
./SGMetric.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUG -I"/home/adess/Documents/my_tools/apps/sglt/sg-LPM/../lpm" -m64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


