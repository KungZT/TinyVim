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
// #include "StatusBar.c"

enum editorKey {
    BACKSPACE = 127,
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
    case '\r':
    break;

    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);//from unistd,,esc=27
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;

    case CTRL_KEY('s'):
        editorSave();
        break;

    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
        editorMoveCursor(c);
        break;    
    case BACKSPACE:
        editorDelchar();
        break;
//允许任何未映射到其他编辑器功能的按键直接插入到正在编辑的文本中。
    default:
        editorInsertChar(c);
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
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);
//屏幕刷新后光标到正确位置
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, 
    E.rx - E.coloff + 1);//这是因为E.cy不再指的是 屏幕上的光标。它指的是光标在范围内的位置 文本文件。为了将光标定位在屏幕上，我们现在必须减去 E.rowoff来自E.cy的值。
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
                "TinyVim Version %s  author: 康镇涛 刘金环",TinyVimVersion);
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
        // if (y < E.screenrows - 1){
            abAppend(ab, "\r\n", 2);
            // }
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
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
    E.screenrows -= 2;
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
    free(E.filename);
    E.filename = strdup(filename);//strdup()来自<string.h> 。它复制给定的字符串，分配所需的内存并假设您将free()该内存。
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
    E.rx = 0;
    if (E.cy < E.numrows){
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }
    if (E.cy < E.rowoff){
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows){
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff){
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols){
        E.coloff = E.rx - E.screencols + 1;
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


// //循环遍历行的chars并计算选项卡的数量，以便知道要为render分配多少内存。
// 每个制表符所需的最大字符数为 8。 row->size已经为每个制表符计为 1，
// 因此我们将制表符数量乘以 7 并将其添加到row->size以获得我们将要使用的最大内存量需要渲染的行。
int editorRowCxToRx(erow *row, int cx)
{
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++){
        if (row->chars[j] == '\t')
        rx += (VIM_TABSTOP - 1) - (rx % VIM_TABSTOP);
        rx++;
    }
    return rx;
}
//对于每个字符，如果它是制表符，
// 我们使用rx % KILO_TAB_STOP来找出最后一个制表位右侧有多少列，
// 然后从KILO_TAB_STOP - 1中减去该值以找出最后一个制表位左侧有多少列下一个制表位。
// 我们将该金额添加到rx以到达下一个制表位的左侧，然后无条件rx++语句使我们正确到达下一个制表位。
// 请注意即使我们当前处于制表位，它也是如何工作的。



void editorRowInsertChar(erow *row, int at, int c)
{
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at+1], &row->chars[at], row->size-at+1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
}
//at允许超出字符串末尾一个字符，在这种情况下，应将该字符插入到字符串末尾。

void editorInsertChar(int c)
{
    if (E.cy == E.numrows){
        editorAppendRow("", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}
//如果E.cy == E.numrows ，则光标位于文件末尾后的波浪线上，
// 因此我们需要在插入字符之前向文件追加新行。插入一个字符后，
// 我们将光标向前移动，以便用户插入的下一个字符位于刚刚插入的字符之后。


char *editorRowsToString(int *buflen)
{
    int totallen = 0;
    int j;
    for (j = 0; j < E.numrows; j++){
        totallen += E.row[j].size + 1;
    }
    *buflen = totallen;

    char *buf = malloc(totallen);
    char *p = buf;
    for (j = 0; j < E.numrows; j++){
        memcpy(p,E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}
// //将每行文本的长度相加，为我们将添加到每行末尾的换行符添加1 。
// 我们将总长度保存到buflen中，以告诉调用者该字符串有多长。
// 在分配所需的内存后，我们循环遍历行，并且 memcpy()将每行的内容复制到缓冲区的末尾，
// 并在每行后面附加一个换行符。


void editorSave()
{
    if (E.filename == NULL) return;
    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    ftruncate(fd, len);
    write(fd, buf, len);
    close(fd);
    free(buf);
}
//如果它是一个新文件，那么E.filename将为NULL ，
// 我们不知道该文件保存在哪里，所以我们暂时不做任何事情就return 。稍后，我们将弄清楚如何提示用户输入文件名。
// 否则，我们调用editorRowsToString() ，并将字符串write()到E.filename中的路径。我们告诉open()我们要创建一个新文件（如果它尚不存在）（ O_CREAT ），并且我们要打开它进行读写（ O_RDWR ）。因为我们使用了O_CREAT标志，所以我们必须传递一个 
// 包含新文件应具有的模式（权限）的额外参数。 0644是您通常需要的文本文件的标准权限。它授予文件所有者读取和写入文件的权限，其他人仅获得读取该文件的权限。
// 覆盖文件的正常方法是将O_TRUNC标志传递给open() ，这会在将新数据写入其中之前完全截断文件，使其成为空文件。
// 通过我们自己将文件截断为与我们计划写入的数据相同的长度，
// 我们使整个覆盖操作更加安全，以防ftruncate()调用成功，
// 但 write()调用失败。在这种情况下，该文件仍将包含之前的大部分数据。
// 但是如果文件被open()完全截断 调用然后write()失败，您最终会丢失所有数据。

void editorRowDelChar(erow *row, int at){
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at+1], row->size-at-1);
    row->size--;
    editorUpdateRow(row);
}
// 与editorRowInsertChar()非常相似，只是我们不需要进行任何内存管理。
// 我们只需使用memmove()用后面的字符覆盖已删除的字符（请注意，
// 末尾的空字节包含在移动中）。然后我们减少行的size ，
// 调用editorUpdateRow() ，并增加E.dirty 。


void editorDelchar(){
    if (E.cy == E.numrows) return;
    erow *row = &E.row[E.cy];
    if (E.cx > 0){
        editorRowDelChar(row, E.cx-1);
        E.cx--;
    }
}

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
    // if (msglen && time(NULL) - E.statusmsg_time < 5)
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