# Makefile for a C++ project

# Compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++11

OBJDIR = obj

all: $(OBJDIR) ./server ./client clean

$(OBJDIR):
	mkdir -p $(OBJDIR)

./server: $(OBJDIR)/serverMain.o $(OBJDIR)/httpServer.o  $(OBJDIR)/globals.o  
	$(CXX) $(CXXFLAGS) $^ -o $@

./client: $(OBJDIR)/clientMain.o $(OBJDIR)/httpClient.o  $(OBJDIR)/globals.o  
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/serverMain.o: serverMain.cpp $(OBJDIR)/httpServer.o  $(OBJDIR)/globals.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/clientMain.o: clientMain.cpp $(OBJDIR)/httpClient.o  $(OBJDIR)/globals.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/httpServer.o: httpServer.cpp httpServer.h $(OBJDIR)/globals.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/httpClient.o: httpClient.cpp httpClient.h $(OBJDIR)/globals.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/globals.o: globals.cpp globals.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -r $(OBJDIR)/*o 
