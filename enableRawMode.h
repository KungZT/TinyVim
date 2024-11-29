#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios orig_termios;


void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | IXON);//关闭回显模式,允许逐字节而不是逐行读取，禁用CtrlC    
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");  
}

void die(const char *s){
    perror(s);//stdio.h
    exit(1);//stdlib.h
}
