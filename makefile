
# Source files, Headers for distribution, internal headers
LIB_SRC_files	= regl3.cpp re_timer.cpp re_input.cpp re_math.cpp re_shader.cpp
LIB_HDR_files	= regl3.h re_timer.h re_math.h re_input.h re_shader.h
SRC_files		= main.cpp juzz_proto.cpp

# The compiled object filenames
LIB_OBJ_files	= $(LIB_SRC_files:.cpp=.o)
OBJ_files		= $(SRC_files:.cpp=.o)

# Directories for input and output
LIB_SRCDIR	= src/
LIB_BINDIR	= bin/
DISTDIR		= dist/
SRCDIR		= proto_src/
BINDIR		= proto_bin/


# Prepend directories to filenames
LIB_SRC		= $(addprefix $(LIB_SRCDIR),$(LIB_SRC_files))
LIB_HDR		= $(addprefix $(LIB_SRCDIR),$(LIB_HDR_files))
LIB_OBJ		= $(addprefix $(LIB_BINDIR),$(LIB_OBJ_files))
SRC			= $(addprefix $(SRCDIR),$(SRC_files))
OBJ			= $(addprefix $(BINDIR),$(OBJ_files))

# Ouput library name
LIBNAME		= libregl3.a
TARNAME		= regl3.tgz
OUTFILE		= prototype

# Compiler, Archiver and flags
CXX			= g++
AR			= ar
CFLAGS		= -I$(LIB_SRCDIR) -Iproto_src/
LDFLAGS 	= -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL  -lGL -lGLU
ARFLAGS		= rcs


# default makes prototype, but depends on regl3 not having changed
test: $(DISTDIR)$(LIBNAME) $(SRC) $(OBJ) $(OUTFILE)


# Link all object files and the needed libraries
$(OUTFILE): $(OBJ) $(DISTDIR)$(LIBNAME)
	$(CXX) -o $@ $(LDFLAGS) $(OBJ) $(LIB_OBJ)

# Compile all the source files in the source directory into 
# object files in the bin directory
$(LIB_BINDIR)%.o : $(LIB_SRCDIR)%.cpp $(LIB_SRCDIR)%.h
	mkdir -p $(LIB_BINDIR)
	$(CXX) -g -c $< -o $@

$(BINDIR)%.o : $(SRCDIR)%.cpp
	mkdir -p $(BINDIR)
	$(CXX) $(CFLAGS) -O2 -c $< -o $@

# makes a library archive for regl3, copying it and the necessary headers to dist directory
$(DISTDIR)$(LIBNAME): $(LIB_OBJ)
	@echo "building library $(LIBNAME)"
	mkdir -p $(DISTDIR)
	$(AR) $(ARFLAGS) $(DISTDIR)$(LIBNAME) $(LIB_OBJ)
	cp $(LIB_HDR) $(DISTDIR)
	cd $(DISTDIR);tar zcvf $(TARNAME) *

clean:
	rm -f $(OUTFILE)
	rm -rf $(BINDIR)
	rm -rf $(DISTDIR)
	rm -f *.o
