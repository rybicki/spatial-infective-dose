MODELS=toxin
SOURCES=main.cpp
INCLUDES=external/include/
INC_PARAMS=$(INCLUDES:%=-I%)
DEBUG_FLAGS=-fsanitize=address -DDEBUG
ICCFLAGS=-xAVX -axCORE-AVX2,CORE-AVX-I
GCCFLAGS=-march=native # -march=sandybridge -mtune=haswell
ICC=icc $(ICCFLAGS)
GCC=g++ $(GCCFLAGS)
CC=$(GCC)
CCFLAGS=-std=c++11 $(INC_PARAMS) -O3 $(SOURCES) -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter
TEST_FLAGS=-std=c++11 $(INC_PARAMS) -O3 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter 

all: release

clean:
	rm -f $(MODELS)

release: CCFLAGS += -DNDEBUG
release: $(MODELS)
debug: CCFLAGS += $(DEBUG_FLAGS)
debug: $(MODELS)
verbose: CCFLAGS += -DDEBUG_MSG
verbose: debug 

test: ppsim/tests/tester.cpp
	$(CC) $(TEST_FLAGS)  $(DEBUG_FLAGS) ppsim/tests/tester.cpp -o run_tests

$(MODELS): %: %.model $(SOURCES) ppsim/*.h external
	    $(CC) $(CCFLAGS) -DMODEL='"$<"' -o $@

external: | $(INCLUDES)

$(INCLUDES):
	cd external
	./setup.sh
	cd ..

