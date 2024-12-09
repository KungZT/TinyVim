# TinyVim
### 一个简单的类VIM文本编辑器
### 汇报人：康镇涛 刘金环

***
## 功能实现

### 1 .终端属性配置

`tcgetatter`: [获取终端属性](https://pubs.opengroup.org/onlinepubs/009696799/functions/tcsetattr.html)
`atexit`: 来自<stdlib.h>,退出时自动调用

```
struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; 
}
```
[ANSI转义序列](https://vt100.net/docs/vt100-ug/chapter3.html)

|标志|含义|效果|
|------|------|------|
|IXON|启用软件流控制|禁用后，`ctrl+s`和`ctrl+q`将不会被发送给程序|
|OPOST|启用输出处理|禁用后，所有字符原样传递|
|ECHO|启用回显|禁用后，输入的字符不会显示在屏幕上|
|IEXTEN|启用信号键|禁用后，拓展键如`ctrl+v`将不会被发送给程序|
|VMIN|读取的最小字符数|设置为0时，读取字符数将不会被限制|
|VTIME|读取超时|设置为0时，读取将一直等待，直到有字符为止|


### 2.初始化窗口

窗口属性
```
void initEditor()
{
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.filename = NULL;
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}
```
获取窗口大小
- 使用`ioctl`函数获取信息
    - `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)`通过标准输出获取窗口大小
    - `ws.ws_row`行数，`ws.ws_col`列数
- 使用ANSI 控制字符获取窗口大小
    - `\x1b[999C\x1b[999B`：将光标向下向右移动999行
    - `write`: 向标准输出写入数据
```
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
```

获取光标位置
- `\x1b[6n`：光标位置信息
- `buf`:存储终端返回的响应数据
    - 响应格式为`\x1b[row;colR`，光标信息
- 循环读取响应数据，每次1字节
- 解析字符串，返回行列信息
```
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
```

### 3.刷新屏幕内容

- `editorScroll()`: 滚动窗口
    - 调整视图的滚动偏移量`E.rowoff`和`E.coloff`
- `editorDrawRows()`: 将每一行内容都绘制到缓冲区中
- `snprintf`: 将格式化字符串写入缓冲区
    - `\x1b[%d;%dH`：光标位置
    - `E.cy - E.rowoff`：实际行号
- `write`: 将缓冲区内容一次性写入标准输出
```
void editorRefreshScreen()
{
    editorScroll();
    struct abuf ab = ABUF_INIT; //初始化缓冲区
    abAppend(&ab, "\x1b[?25l", 6);//隐藏光标
    abAppend(&ab, "\x1b[H", 3);//移到左上角
    editorDrawRows(&ab);
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, 
    E.rx - E.coloff + 1);
    abAppend(&ab, buf, strlen(buf));
    abAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
```

### 4.调整滚动偏移量

`rx`:光标的实际列数
`editorRowCxToRx()`: 将光标列数转换为实际列数
- 列数超过屏幕宽度，则将光标列数转换为实际列数
- 否则，返回光标列数
- 列数小于屏幕宽度，则返回光标列数

```
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
```
### 5.绘制内容到缓冲区
- 遍历每行
- 计算文件行号
- 显示欢迎消息
- 文件不空
    -计算显示的内容长度

```
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
            abAppend(ab, &E.row[filerow].render[E.coloff], len);
        }
        abAppend(ab , "\x1b[K", 3);//清除光标后内容，确保没有残余字符
        if (y < E.screenrows - 1){
            abAppend(ab, "\r\n", 2);
            }
    }
}
```


### 6.处理键盘输入

```
void editorProcessKeypress()
{
    int c = editorReadKey();
    switch (c)
    {
    case '\r':
    break;

    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);//清屏
        write(STDOUT_FILENO, "\x1b[H", 3);光标移到左上角
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
```

- `editorReadKey()`: 读取用户输入
    - 对于普通按键，返回ASCII
    - 对于特殊按键，返回自定义的编码值
  
### 7.处理光标移动
- `cx`:光标列数
- `cy`:光标行数
```
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
```

### 8.删除字符
-对比光标位置与文件内容
-删除后字符向左移
-字符减一

```
void editorDelchar(){
    if (E.cy == E.numrows) return;
    erow *row = &E.row[E.cy];
    if (E.cx > 0){
        editorRowDelChar(row, E.cx-1);
        E.cx--;
    }
}
```
```
void editorRowDelChar(erow *row, int at){
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at+1], row->size-at-1);
    row->size--;
    editorUpdateRow(row);
}
```
### 9.插入字符
- 如果在最后一行，则创建新行
- 不在最后则插入字符
```
void editorInsertChar(int c)
{
    if (E.cy == E.numrows){
        editorAppendRow("", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}
```
```
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
```
```
void editorRowInsertChar(erow *row, int at, int c)
{
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at+1], &row->chars[at], row->size-at+1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
}
```
### 10.打开文件
- 更新文件名
- 逐行读取文件
- 将数据存储到E.row[]中
```
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
```
```
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
```

### 11.保存文件
- 将所有行合并成一个字符串
- `open(E.filename, O_RDWR | O_CREAT):读写模式，0644:文件权限

```
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
```


### 12.待实现功能
状态栏
语法高亮
按键关联
多种光标跳转方式
搜索



***
## 12.9新增状态栏

### 对之前函数的修改
对屏幕操作的处理
        // if (y < E.screenrows - 1){
            abAppend(ab, "\r\n", 2);
            // }
    E.screenrows -= 1;

函数

refresh调用
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);

        char statusmsg[80];

### 新增函数

```
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
```