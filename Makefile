override CXXFLAGS += -std=c++0x
override CC=${CXX} # for linking

main: $(patsubst %.cpp,%.o,$(wildcard *.cpp))

clean:
	rm -f main *.o
