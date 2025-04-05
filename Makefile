CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

SRC = $(wildcard src/*.cpp)

OBJ = $(SRC:.cpp=.o)

TARGET = mygit

LIBS = -lssl -lcrypto

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: clean

