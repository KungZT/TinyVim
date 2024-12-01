// #include <termios.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <errno.h>
// #include "TinyVim.h"
#include "RawMode.h"

struct editorConfig E;


void die(const char *s){
    write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);//stdio.h
    exit(1);//stdlib.h
}

void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = E.orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | IXON);//关闭回显模式,允许逐字节而不是逐行读取，禁用CtrlC    
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");  
}




