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

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_UP ,
    ARROW_RIGHT ,
    ARROW_DOWN 
};


int editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == '\x1b')
    {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
            case 'A': return ARROW_UP;
            case 'B': return ARROW_DOWN;
            case 'C': return ARROW_RIGHT;
            case 'D': return ARROW_LEFT;
            }
        }
        return '\x1b';
    }else{
    return c;
    }
}

void editorProcessKeypress()
{
    int c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;

    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
        editorMoveCursor(c);
        break;    
    }
}

void editorRefreshScreen()
{
    editorScroll();
    struct abuf ab = ABUF_INIT; //初始化缓冲区

    abAppend(&ab, "\x1b[?25l", 6);//隐藏光标
    abAppend(&ab, "\x1b[H", 3);//移到左上角
    editorDrawRows(&ab);

//屏幕刷新后光标到正确位置
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, E.cx - E.coloff + 1);//这是因为E.cy不再指的是 屏幕上的光标。它指的是光标在范围内的位置 文本文件。为了将光标定位在屏幕上，我们现在必须减去 E.rowoff来自E.cy的值。
    abAppend(&ab, buf, strlen(buf));
//恢复光标
    // abAppend(&ab, "\x1b[H", 3);
    abAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
//     write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
//     write(STDOUT_FILENO, "\x1b[H", 3);

//     editorDrawRows();
//     write(STDOUT_FILENO, "\x1b[H", 3);
}

void editorDrawRows(struct abuf *ab)
{
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows){
        if (E.numrows == 0 && y == E.screenrows / 3)
        {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "TinyVim Version %s",TinyVimVersion);
            if (welcomelen > E.screencols) welcomelen = E.screencols;
            //居中
            int padding = (E.screencols - welcomelen) / 2;
            if (padding){
                abAppend(ab, "~", 1);
                padding--;
            }
            while (padding--) abAppend(ab, " ", 1);
            abAppend(ab, welcome , welcomelen);
        }else{
            abAppend(ab, "~", 1);
        }
        }else{
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            abAppend(ab, &E.row[filerow].render[E.coloff], len);//这是因为E.cy不再指的是 屏幕上的光标。它指的是光标在范围内的位置 文本文件。为了将光标定位在屏幕上，我们现在必须减去 E.rowoff来自E.cy的值。
        }
        abAppend(ab , "\x1b[K", 3);
        if (y < E.screenrows - 1){
            abAppend(ab, "\r\n", 2);
            }
    }
}

int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    
    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}
    int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
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
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}


void abAppend(struct abuf *ab, const char *s, int len)//len 字节的内容从 s 被复制到新的缓冲区 new 中的合适位置
{
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab)
{
    free(ab->b);
}

void editorMoveCursor(int key)
{
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key){
        case ARROW_LEFT:
        if (E.cx != 0){
        E.cx--;}else if (E.cy > 0){
            E.cy--;
            E.cx = E.row[E.cy].size;
        }//允许向左移动到上一行
        break;
        case ARROW_RIGHT:
        // 向右不允许到末尾
        if (row && E.cx < row->size){
        E.cx++;
        }else if (row && E.cx == row->size){
            E.cy++;
            E.cx = 0;
        }//允许向右移动到下一行
        break;
        case ARROW_UP:
        if (E.cy != 0){
        E.cy--;}
        break;
        case ARROW_DOWN:
        if (E.cy < E.numrows){
        E.cy++;}
        break;
    }
    //纠正E.cx如果它最终超出了所在行的末尾
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen){
    E.cx = rowlen;
    }
}

void editorOpen(char *filename){
    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line, &linecap, fp)) != -1){
        while (linelen > 0 && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
        linelen--;
    editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
}

void editorAppendRow(char *s,size_t len){
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len+1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);
    E.numrows++;
}


void editorScroll()
{
    if (E.cy < E.rowoff){
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows){
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.cx < E.coloff){
        E.coloff = E.cx;
    }
    if (E.cx >= E.coloff + E.screencols){
        E.coloff = E.cx - E.screencols + 1;
    }
}
//第一个if语句检查光标是否位于可见窗口上方，
// 如果是，则向上滚动到光标所在的位置。第二个if 语句检查光标是否超出可见窗口的底部，
// 并且 包含稍微复杂的算术，因为E.rowoff指的是屏幕顶部的内容，
// 而我们必须让E.screenrows参与讨论屏幕底部的内容。


void editorUpdateRow(erow *row)
{
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t') tabs++;    
    }
    free(row->render);
    //循环遍历行的chars并计算选项卡的数量，
    // 以便知道要为render分配多少内存。每个制表符所需的最大字符数为 8。 
    // row->size已经为每个制表符计为 1，因此我们将制表符数量乘以 7 
    // 并将其添加到row->size以获得我们将要使用的最大内存量需要渲染的行。
    row->render = malloc(row->size + tabs*(VIM_TABSTOP-1) + 1);
    int idx = 0;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t'){
            row->render[idx++] = ' ';
            while (idx % VIM_TABSTOP != 0) row->render[idx++] = ' ';
        }else{
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}