#ifndef RAWMODE_H
#define RAWMODE_H

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
# include <time.h>
// 从 TinyVim.h 中获取 editorConfig 的定义
// #include "TinyVim.h"
// #include "screen.h"

// 声明全局变量 editorConfig
// extern struct editorConfig E;

//erow代表“编辑器行”，并将一行文本存储为指向动态分配的字符数据和长度的指针。 typedef让我们将类型称为erow而不是struct erow 。
typedef struct erow {
    int size;   
    int rsize;
    char *chars;
    char *render;
} erow;

struct editorConfig{
    int cx , cy;
    int rx;
    int rowoff;//垂直
    int coloff;//水平
    int screenrows;
    int screencols;
    int numrows;
    erow *row;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct termios orig_termios;
};

extern struct editorConfig E;
// 函数声明
void die(const char *s);
void disableRawMode();
void enableRawMode();

#endif /* RAWMODE_H */
