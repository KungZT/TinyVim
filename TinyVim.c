// #include <unistd.h>
// #include <ctype.h>
// #include <stdio.h>
#include "TinyVim.h"
#include "RawMode.h"
#include "screen.h"
// #include "editor.c"


// #define CTRL_KEY(k) ((k) & 0x1f)

int main(int argc, char *argv[]){
    enableRawMode();
	initEditor();
	if (argc >= 2) {
		editorOpen(argv[1]);
	}
	while (1)
	{	
		editorRefreshScreen();
		editorProcessKeypress();
    	// char c = '\0';
    	// read(STDIN_FILENO , &c, 1);
	    // if (iscntrl(c)){
		// printf("%d\n", c);
	    // }else{
		// printf("%d('%c')\n", c , c);
	    // }
		// if (c == CTRL_KEY('q')) break;
	}
    return 0;
}