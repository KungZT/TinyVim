// #include <unistd.h>
// #include <errno.h>
// #include <sys/ioctl.h>
// #include <termios.h>
// #include "RawMode.h"

// #define CTRL_KEY(k) ((k) & 0x1f)
// #include "TinyVim.h"
#include "TinyVim.h"
#include "RawMode.h"
#include "screen.h"
char editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

void editorProcessKeypress()
{
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void editorDrawRows()
{
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

int getCursorPosition(int *rows, int *cols)
{
    // char buf[32];
    // unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    printf("\r\n");
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (iscntrl(c)){
            printf("%d\r\n",c);
        }else{
        printf("%d ('%c')\r\n",c,c);
        }
    }
    editorReadKey();

    return -1;
}
    int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        editorReadKey();
        return -1;
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void initEditor()
{
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}
