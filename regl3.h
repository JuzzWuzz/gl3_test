#ifndef _REGL3_H
#define _REGL3_H


class reGL3App{
public:
	reGL3App(AppConfig conf);
	~reGL3App();

	bool		InitSDL();
	bool		InitGL();
};


#endif
