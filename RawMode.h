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


struct editorConfig{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

extern struct editorConfig E;
// 函数声明
void die(const char *s);
void disableRawMode();
void enableRawMode();

#endif /* RAWMODE_H */
