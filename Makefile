GXX = g++
CLANGXX = clang++
CXX := ${GXX}
# CXX := ${CLANGXX}
# clang has better error messages, but does not generate useful debug info, even with current patches
CXXFLAGS = -g -Wall -Wextra
override CXXFLAGS += -std=c++0x
override CPPFLAGS += -include gcc-versions.hpp

% : %.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
%.o : %.cpp %.hpp %.tcc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
%.s : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -S -o $@ $< -masm=intel
%.ll : %.cpp
	$(CLANGXX) $(CXXFLAGS) $(CPPFLAGS) -S -o $@ $< -emit-llvm
%.hpp:
%.tcc:

main: $(patsubst %.cpp,%.o,$(wildcard *.cpp))

clean:
	rm -f main *.o *.s *.ll
