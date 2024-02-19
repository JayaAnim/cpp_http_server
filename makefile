# Makefile for a C++ project

# Compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++11

OBJDIR = obj

all: $(OBJDIR) ./server clean

$(OBJDIR):
	mkdir -p $(OBJDIR)

./server: $(OBJDIR)/main.o $(OBJDIR)/server.o  
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/main.o: main.cpp $(OBJDIR)/server.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/server.o: server.cpp server.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -r $(OBJDIR)/*o 
