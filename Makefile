CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -municode -mwindows -lole32

TARGET = a.exe
SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	del /f $(TARGET)