
CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = ircserv

SRC = main.cpp \
		Server.cpp \
		Client.cpp \
		Channel.cpp \

OBJ = $(SRC:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o

fclean: clean
	rm -f $(NAME)

re: fclean all
