CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -luser32 -lgdi32

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = keymap.exe
RESOURCE_FILE = resource.rc

# Detect MSYS2 as a POSIX-compliant system
ifdef MSYSTEM
	RM = rm -f
else
	RM = del /Q /F
endif


all: release

release: CFLAGS += -O3
release: LDFLAGS += -mwindows
release: $(EXEC)

debug: CFLAGS += -g
debug: $(EXEC)

$(EXEC): $(OBJS) $(RESOURCE_FILE)
	windres $(RESOURCE_FILE) -O coff -o resource.o
	$(CC) $(OBJS) resource.o -o $(EXEC) $(CFLAGS) $(LDFLAGS)
	$(RM) resource.o

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJS) $(EXEC)