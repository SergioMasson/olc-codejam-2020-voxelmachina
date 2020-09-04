CC=g++
CFLAGS=-Wall -g

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

NAME=terminal

all: $(NAME)

$(NAME): $(OBJ)
		$(CC) -o $@ $^

%.o: %.cpp %.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o $(NAME)
