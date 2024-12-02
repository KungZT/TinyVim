#ifndef TINYVIM_H
#define TINYVIM_H

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define TinyVimVersion "0.0.1"
#define VIM_TABSTOP 8

#endif
