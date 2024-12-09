CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = TinyVim
OBJS = TinyVim.o RawMode.o screen.o 
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

TinyVim.o: TinyVim.c screen.h RawMode.h TinyVim.h
	$(CC) $(CFLAGS) -c TinyVim.c


screen.o: screen.c screen.h RawMode.h TinyVim.h 
	$(CC) $(CFLAGS) -c screen.c

RawMode.o: RawMode.c RawMode.h TinyVim.h
	$(CC) $(CFLAGS) -c RawMode.c


# editor.o: editor.c RawMode.h screen.h Tinyvim.h
# 	$(CC) $(CFLAGS) -c editor.c
clean:
	rm -f $(OBJS) $(TARGET)