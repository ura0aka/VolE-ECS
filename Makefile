#compiler variable
CC = g++
INCL = -Isrc/include
LIBS = -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
#optimization variable
OPT = -O0

#add cpp files here
CPPFILES = main.cpp Game.cpp
#define .o files to be created here
OBJECTS = main.o Game.o

BINARY = app

all: $(BINARY)

#executable var is dependant on the existence of our object files
$(BINARY):$(OBJECTS)
#g++ -o app main.o -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
	$(CC) -o $@ $^ $(LIBS)

#regex that states that any object file, to be created, must be created from %(anything).cpp file
%.o:%.cpp
	$(CC) -c -o $@ $< $(INCL)
# $@ = %.o
# $^ = %.cpp

# all .o files possess a dependency on Game.hpp
$(OBJECTS): Game.hpp

clean: 
	rm -rf $(BINARY) $(OBJECTS)
