
SRC 		= main.cpp regl3.cpp ReTimer.cpp Input.cpp
OBJ 		= $(SRC:.cpp=.o)
HDR 		= common.h regl3.h ReTimer.h
OUTDIR		= build
OUTFILE 	= gl3test
LIB 		= -L/usr/local/lib -Wl,-rpath,/usr/local/lib  -lSDL  -lGL -lGLU
CC			= g++

all: $(SRC) $(HDR) $(OUTFILE)


$(OUTFILE): $(OBJ) 
	mkdir $(OUTDIR)
	$(CC) -o $(OUTDIR)/$@ $(LIB) $(OBJ)

.cpp.o: 
	$(CC) -c $< -o $@


debug: $(SRC) $(HDR) $(OUTFILE)
	$(CC) -pg -o $(OUTFILE) $(LIB) $(SRC)

release: $(SRC) $(HDR) $(OUTFILE)
	$(CC) -O2 -o $(OUTFILE) $(LIB) $(SRC)
	rm *.o

clean:
	rm -rf $(OUTDIR)
	rm -f *.o
