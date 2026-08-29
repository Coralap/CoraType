CC = gcc
CFLAGS = -Wall -Wextra -O2 -DUNICODE -D_UNICODE
LDFLAGS = -municode -mwindows -lcomdlg32

TARGET = a.exe
SRCS = src/main.c src/window.c src/buffer.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	del /f $(TARGET)