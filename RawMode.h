#ifndef RAWMODE_H
#define RAWMODE_H

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// 从 TinyVim.h 中获取 editorConfig 的定义
// #include "TinyVim.h"
// #include "screen.h"

// 声明全局变量 editorConfig
// extern struct editorConfig E;

//erow代表“编辑器行”，并将一行文本存储为指向动态分配的字符数据和长度的指针。 typedef让我们将类型称为erow而不是struct erow 。
typedef struct erow {
    int size;   
    char *chars;
} erow;

struct editorConfig{
    int cx , cy;
    int screenrows;
    int screencols;
    int numrows;
    erow row;
    struct termios orig_termios;
};

extern struct editorConfig E;
// 函数声明
void die(const char *s);
void disableRawMode();
void enableRawMode();

#endif /* RAWMODE_H */
