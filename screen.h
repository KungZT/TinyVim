#ifndef SCREEN_H
#define SCREEN_H

#include <string.h>
#include <stdio.h> 
#include "RawMode.h"
#include "TinyVim.h"

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

char editorReadKey();
void editorProcessKeypress();
void editorRefreshScreen();
void editorDrawRows(struct abuf *ab);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
void initEditor();
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);


#endif