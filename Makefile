CXX := clang++
CXXFLAGS := -std=c++17 -Wall

src := $(wildcard *.cpp)
obj := $(patsubst %.cpp, %.o, $(src))
bin := wordle_solver
# Declare names that indicate recipes, not files 
.PHONY: all clean

all: $(bin)

$(bin): $(obj)
	$(CXX) $(LDFLAGS) $^ -o $@

# Generic object file creation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(obj) $(bin)
