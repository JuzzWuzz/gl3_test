
SRC_files	= main.cpp regl3.cpp ReTimer.cpp Input.cpp
HDR_files	= common.h regl3.h ReTimer.h
OBJ_files	= $(SRC_files:.cpp=.o)
SRCDIR		= src/
BINDIR		= bin/

SRC			= $(addprefix $(SRCDIR),$(SRC_files))
HDR			= $(addprefix $(SRCDIR),$(HDR_files))
OBJ			= $(addprefix $(BINDIR),$(OBJ_files))

OUTFILE 	= gl3test
LIBNAME		= libregl3.a

LDFLAGS 	= -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL  -lGL -lGLU
CFLAGS		= 
CXX			= g++
AR			= ar
ARFLAGS		= rcs

# The rule to rule them all
all: $(SRC) $(HDR) $(OUTFILE)

# Link all object files and the needed libraries
$(OUTFILE): $(OBJ) 
	@echo $(filter-out $(BINDIR)main.o,$(OBJ))
	$(CXX) -o $@ $(LDFLAGS) $(OBJ)

# Compile all the source files in the source directory into 
# object files in the bin directory
$(BINDIR)%.o : $(SRCDIR)%.cpp
	mkdir -p $(BINDIR)
	$(CXX) -c $< -o $@

# Create executable with debug information
debug: 
	$(CXX) -pg -o $(OUTFILE) $(LDFLAGS) $(SRC)

# Create executable with Optimization
release: 
	$(CXX) -O2 -o $(OUTFILE) $(LDFLAGS) $(SRC)

# makes a library archive for regl3
lib: $(OBJ)
	@echo "building library $(LIBNAME)"
	$(AR) $(ARFLAGS) $(LIBNAME) $(filter-out $(BINDIR)main.o, $(OBJ))

clean:
	rm -f $(OUTFILE)
	rm -rf $(BINDIR)
	rm -f *.o
