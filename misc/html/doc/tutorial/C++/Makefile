# Name of the Executable
Target = main

# Defining Sources
sources = main.cc system.cc

# Do Not Edit the Following Lines
CXX = g++
libsources = 
objects = $(sources:.cc=.o) 
CPPFLAGS= -g3 -Wall -Werror -Wno-unknown-pragmas `pkg-config --cflags mbsim` 
CXXFLAGS= -std=c++11

$(Target) : $(objects)
	$(CXX) -o $@ $(objects) `pkg-config --libs mbsim` 

%.d: %.cc
	set -e; $(CXX) -MM $(CPPFLAGS) $< \
	  | sed 's/\(.*\)\.o[ :]*/\1.o \1.d : /g' > $@; \
	  [ -s $@ ] || rm -f $@

include $(sources:.cc=.d)

.PHONY : clean
clean :
	-rm $(Target) $(objects) $(sources:.cc=.d)

