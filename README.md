# TinyVim详解

## 函数说明

`tcgetatter`: 获取终端属性。https://pubs.opengroup.org/onlinepubs/009696799/functions/tcsetattr.html
`atexit`: 来自<stdlib.h>,退出时自动调用

void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = E.orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | IXON);//关闭回显模式,允许逐字节而不是逐行读取，禁用CtrlC    
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");  
}
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");


    struct editorConfig{
    int cx , cy;
    int screenrows;
    int screencols;
    struct termios orig_termios;
};


初始化窗口


void initEditor()
{
    E.cx = 0;
    E.cy = 0;
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}



ANSI转义序列  https://vt100.net/docs/vt100-ug/chapter3.html
