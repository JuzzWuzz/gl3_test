
# Source files, Headers for distribution, internal headers
SRC_files		= regl3.cpp re_timer.cpp re_input.cpp re_math.cpp re_shader.cpp
HDR_dist_files	= regl3.h re_timer.h re_math.h re_input.h re_shader.h
HDR_files		= util.h
TEST_SRC		= main.cpp
# The compiled object filenames
OBJ_files	= $(SRC_files:.cpp=.o)

# Directories for input and output
SRCDIR		= src/
BINDIR		= bin/
DISTDIR		= dist/

# Prepend directories to filenames
SRC			= $(addprefix $(SRCDIR),$(SRC_files))
HDR_dist	= $(addprefix $(SRCDIR),$(HDR_dist_files))
HDR			= $(HDR_dist) $(addprefix $(SRCDIR),$(HDR_files))
OBJ			= $(addprefix $(BINDIR),$(OBJ_files))

# Ouput library name
LIBNAME		= libregl3.a
TARNAME		= regl3.tgz
OUTFILE		= gl3test

# Compiler, Archiver and flags
CXX			= g++
AR			= ar
CFLAGS		= 
LDFLAGS 	= -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL  -lGL -lGLU
ARFLAGS		= rcs


# default makes test
test: $(SRC) bin/main.o $(HDR) $(OUTFILE)

all: lib test

# The rule to make the library
lib: $(SRC) $(HDR) $(LIBNAME)

# Link all object files and the needed libraries
$(OUTFILE): $(OBJ) bin/main.o
	$(CXX) -o $@ $(LDFLAGS) $(OBJ) $(SRCDIR)$(TEST_SRC)

# Compile all the source files in the source directory into 
# object files in the bin directory
$(BINDIR)%.o : $(SRCDIR)%.cpp
	mkdir -p $(BINDIR)
	$(CXX) -g -c $< -o $@

# makes a library archive for regl3, copying it and the necessary headers to dist directory
$(LIBNAME): $(OBJ)
	@echo "building library $(LIBNAME)"
	mkdir -p $(DISTDIR)
	$(AR) $(ARFLAGS) $(DISTDIR)$(LIBNAME) $(filter-out $(BINDIR)main.o, $(OBJ))
	cp $(HDR_dist) $(DISTDIR)
	cd $(DISTDIR);tar zcvf $(TARNAME) *



clean:
	rm -f $(OUTFILE)
	rm -rf $(BINDIR)
	rm -rf $(DISTDIR)
	rm -f *.o
