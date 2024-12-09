#include "RawMode.h"
#include <time.h>
#include <stdarg.h>
#include "screen.h"

void editorDrawStatusBar(struct abuf *ab)
{
    abAppend(ab, "\x1b[7m", 4);//7号颜色
    char status[80], rstatus[80];
    //显示最多 20 个字符的文件名，后跟文件中的行数
    int len = snprintf(status, sizeof(status), "%.20s - %d lines", 
        E.filename ? E.filename : "[No Name]", E.numrows);
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numrows);
    if (len > E.screencols) len = E.screencols;//屏幕宽度
    abAppend(ab, status, len);
    while (len < E.screencols) {
        if (E.screencols - len == rlen){
            abAppend(ab, rstatus, rlen);
            break;
        } else {
        abAppend(ab, " ", 1);
        len++;
    }
    }
    abAppend(ab, "\x1b[m", 3);//恢复默认颜色
    abAppend(ab, "\r\n", 2);//回车换行
}

void editorDrawMessageBar(struct abuf *ab)
{
    abAppend(ab, "\x1b[K", 3);//清空行
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(ab, E.statusmsg, msglen);
}

void editorSetStatusMessage(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}