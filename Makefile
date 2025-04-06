CC = gcc
CFLAGS = -Wall -Wextra -Iinclude $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lGL -lGLU -lm
SRC = src/main.c src/game.c src/rl.c
OBJ = $(SRC:.c=.o)
EXEC = snake

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(OBJ) $(EXEC)

run: $(EXEC)
	./$(EXEC)
