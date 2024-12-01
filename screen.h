#ifndef SCREEN_H
#define SCREEN_H


#include "RawMode.h"
#include "TinyVim.h"


char editorReadKey();
void editorProcessKeypress();
void editorRefreshScreen();
void editorDrawRows();
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
void initEditor();

#endif