CXX = clang++
CXXFLAGS = -g
override CXXFLAGS += -std=c++0x

% : %.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
%.s : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -S -o $@ $< -masm=intel
%.ll : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -S -o $@ $< -emit-llvm

main: $(patsubst %.cpp,%.o,$(wildcard *.cpp))

clean:
	rm -f main *.o *.s *.ll
