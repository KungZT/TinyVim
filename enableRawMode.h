#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios orig_termios;


void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | IXON);//关闭回显模式,允许逐字节而不是逐行读取，禁用CtrlC    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
