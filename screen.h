#ifndef SCREEN_H
#define SCREEN_H

#include <fcntl.h>
#include <string.h>
#include <stdio.h> 
#include <sys/types.h>
#include "RawMode.h"
#include "TinyVim.h"


struct abuf {
    char *b;
    int len;
};


#define ABUF_INIT {NULL, 0}

int editorReadKey();
void editorProcessKeypress();
void editorRefreshScreen();
void editorDrawRows(struct abuf *ab);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
void initEditor();
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorMoveCursor(int key);
void editorOpen(char *filename);
void editorAppendRow(char *s,size_t len);
void editorScroll();
void editorUpdateRow(erow *row);
int editorRowCxToRx(erow *row, int cx);
void editorRowInsertChar(erow *row, int at, int c);
void editorInsertChar(int c);
void editorSave();
void editorRowDelChar(erow *row, int at);
void editorDelchar();

#endif