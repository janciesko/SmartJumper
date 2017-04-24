CXX=c++ 
#CXX=mcxx
LDFLAGS= -fopenmp
LDFLAGSTRC= -fopenmp
LDFLAGSDBG=-g -fopenmp
FLAGS =-O3 -std=c++11  -fopenmp
DBGFLAGS = -g -std=c++11 -fopenmp
TRCFLAGS= -std=c++11 -fopenmp

TARGET = smartjumper
TARGET_D = smartjumper_dbg
TARGET_T = smartjumper_trc

default: $(TARGET)
debug: $(TARGET_D)
trace: $(TARGET_T)
all: $(TARGET) $(TARGET_D) $(TARGET_T)

$(TARGET).o: src/$(TARGET).cpp
	$(CXX) $(FLAGS) -c $< -o $@

helper.o: src/helper.cpp
	$(CXX) $(FLAGS) -c $< -o $@

dummy.o: src/dummy.c
	$(CXX) $(FLAGS) -c $< -o $@

$(TARGET):$(TARGET).o helper.o dummy.o 
	$(CXX) $(LDFLAGS) $^ -o $@

$(TARGET_D).o: src/$(TARGET).cpp
	$(CXX) $(DBGFLAGS) -c $< -o $@

$(TARGET_D):$(TARGET_D).o helper.o dummy.o
	$(CXX) $(LDFLAGSDBG)   $^ -o $@

$(TARGET_T).o: src/$(TARGET).cpp
	$(CXX) $(TRCFLAGS) -c $< -o $@

$(TARGET_T):$(TARGET_T).o helper.o dummy.o
	$(CXX) $(LDFLAGSTRC) $^ -o $@

clean:
	rm -rf $(TARGET) $(TARGET_D) $(TARGET_T) *.o mcxx* *.out *.err

 
