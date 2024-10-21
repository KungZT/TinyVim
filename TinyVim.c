#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include "enableRawMode.h"
#include "disableRawMode.h"

int main(){
    enableRawMode();
    char c;
    while (read(STDIN_FILENO , &c, 1)==1 && c != 'q') {
	    if (iscntrl(c)){
		printf("%d\n", c);
	    }else{
		printf("%d('%c')\n", c , c);
	    }
	}
    return 0;
}