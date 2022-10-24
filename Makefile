NAME		:= webserv
SRC_DIR     := srcs/
SRCS        := Client.cpp Config.cpp Epoll.cpp EventListener.cpp main.cpp Server.cpp Webserv.cpp
SRCS        := $(SRCS:%=$(SRC_DIR)/%)
BUILD_DIR	:= .build
OBJS        := $(subst .cpp,.o,$(SRCS))
OBJS        := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJS))
DEPS        := $(subst .o,.d,$(OBJS))

CC          := clang++ 
CPPFLAGS    := -Wall -Wextra -Werror -std=c++98 -MMD -MP -I Includes -g3
RM          := rm -rf
MAKE        := $(MAKE) --jobs --no-print-directory
DIR_DUP		= mkdir -p $(@D)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $^ -o $@
	$(info CREATED $(NAME))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)
docker:
	docker build -t webserv .
	docker run --rm -p 5000:5000 --name my_webserv webserv
docker_stop:
	docker stop my_webserv
clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re
.SILENT:

-include $(DEPS)
