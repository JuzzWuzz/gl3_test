
SRC 		= main.cpp regl3.cpp 
OBJ 		= $(SRC:.cpp=.o)
HDR 		= common.h regl3.h
OUTFILE 	= gl3test
LIB 		= -lGLU -lGL -lSDL 
CC			= g++

all: $(SRC) $(HDR) $(OUTFILE)


$(OUTFILE): $(OBJ)
	$(CC) -o $@ $(LIB) $(OBJ)

.cpp.o:
	$(CC) -c $< -o $@


debug: $(SRC) $(HDR) $(OUTFILE)
	$(CC) -pg -o $(OUTFILE) $(LIB) $(SRC)

release: $(SRC) $(HDR) $(OUTFILE)
	$(CC) -O2 -o $(OUTFILE) $(LIB) $(SRC)

clean:
	rm -f *.o
	rm -f $(OUTFILE)
