
INCLUDES=-I$(eraser)/src -I$(GMOCK_DIR)/include -I$(GTEST_DIR)/include
LIBS=$(GMOCK_DIR)/lib/.libs/libgmock_main.a $(GMOCK_DIR)/lib/.libs/libgmock.a $(GTEST_DIR)/lib/.libs/libgtest.a -lpthread

.PHONY: all clean

all: shared_var_test locked_test universal_set_test
run: all
	./universal_set_test && ./shared_var_test && ./locked_test

shared_var_test: ./shared_var.cpp ../eraser/shared_var.cpp ../eraser/shared_var.h ../eraser/shared_var_impl.h ../eraser/shared_var_inst.h
	g++ -DERASER_DEBUG ../eraser/shared_var.cpp ./shared_var.cpp -g $(INCLUDES) $(LIBS) -o shared_var_test
	
locked_test: ./locked.cpp ../eraser/shared_var.cpp ../eraser/shared_var.h ../eraser/shared_var_impl.h ../eraser/shared_var_inst.h
	g++ -DERASER_DEBUG ../eraser/shared_var.cpp ./locked.cpp -g $(INCLUDES) $(LIBS) -o locked_test

universal_set_test: ./universal_set.cpp ../eraser/universal_set.h
	g++ -DERASER_DEBUG ./universal_set.cpp -g $(INCLUDES) $(LIBS) -o universal_set_test

clean:
	rm -f shared_var_test locked_test universal_set_test
