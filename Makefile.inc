EXEC_DIR?=./exec
INCLUDES += -I$(EXEC_DIR)/include

EXEC_OBJS:= \
        objs/exec_exec.o

objs/exec_exec.o: $(EXEC_DIR)/src/exec.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;
