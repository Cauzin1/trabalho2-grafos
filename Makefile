CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
BIN      = cmstp
SRC      = src/main.cpp src/Grafo.cpp src/Solucao.cpp src/Resolvedor.cpp
OBJ      = $(SRC:.cpp=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
