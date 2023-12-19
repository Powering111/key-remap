CC = g++
CFLAGS = -Wall -Wextra -std=c++17

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = keymap.exe

# RM for cross-platform compatibility
ifeq ($(OS),Windows_NT)
	RM = del /Q /F
else
	RM = rm -f
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(CFLAGS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJS) $(EXEC)