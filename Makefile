CC = gcc
CFLAGS = -Wall -Wextra -O2 -DUNICODE -D_UNICODE
LDFLAGS = -municode -mwindows

TARGET = a.exe
SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	del /f $(TARGET)