CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
EXECUTABLE = $(BINDIR)/battleship

# Detect OS and set appropriate libraries
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS = -lpthread
endif
ifeq ($(UNAME_S),Darwin)
    LIBS = -lpthread
endif
ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32 -lwsock32
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean