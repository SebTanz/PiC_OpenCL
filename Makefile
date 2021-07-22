SRC := array.cc pic.cc init.cc step.cc output.cc
tSRC := array.cc picTimer.cc init.cc step.cc output.cc
OBJ := $(SRC:%.cc=%.o)
dOBJ := $(SRC:%.cc=d%.o)
tOBJ := $(tSRC:%.cc=t%.o)

test:  pic
	./pic

$(OBJ): %.o: %.cc pic.h array.h
	g++ $< -Wall -O2 -c -l OpenCL

pic:  $(OBJ)
	g++ -o pic $^ -l OpenCL

$(dOBJ): d%.o: %.cc pic.h array.h
	g++ $< -g -o $@ -Wall -O2 -c -l OpenCL

$(tOBJ): t%.o: %.cc pic.h array.h
	g++ $< -g -o $@ -Wall -O2 -c -l OpenCL

dpic: $(dOBJ)
	g++ -g -o pic $^ -l OpenCL

tpic: $(tOBJ)
	g++ -o picTimer $^ -l OpenCL

debug: dpic
	gdb pic

timer: tpic
	./picTimer

clean:
	rm -f pic $(OBJ) $(dOBJ) $(tOBJ) data/partData.* data/fieldData.*



