CPP_SRC=$(wildcard src/*.cpp)
HEADER=$(wildcard src/*.hpp)
GLAD=glad.c
OBJ=$(CPP_SRC:%=%.o)
CPP=c++
BIN_NAME=infworld
INCLUDE=-Iinclude
FLAGS=$(INCLUDE) -std=c++17 -O2 -DDISALLOW_ERRORS
LD_FLAGS=-lglfw3 -lGL

output: $(OBJ)
	$(CPP) $(OBJ) $(GLAD) -o $(BIN_NAME) $(FLAGS) $(LD_FLAGS)

%.cpp.o: %.cpp $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ) $(BIN_NAME)

run: output
	./$(BIN_NAME)
