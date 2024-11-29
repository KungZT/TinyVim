#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include "enableRawMode.h"
#include "disableRawMode.h"

int main(){
    enableRawMode();
	while (1)
	{	
    	char c = '\0';
    	read(STDIN_FILENO , &c, 1);
	    if (iscntrl(c)){
		printf("%d\n", c);
	    }else{
		printf("%d('%c')\n", c , c);
	    }
		if (c == 'q') break;
	}
    return 0;
}