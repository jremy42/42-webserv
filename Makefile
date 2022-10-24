NAME		:= webserv
SRC_DIR     := srcs/
SRCS        := main.cpp Epoll.cpp
SRCS        := $(SRCS:%=$(SRC_DIR)/%)
BUILD_DIR	:= .build/
OBJS        := $(subst .cpp,.o,$(SRCS))
OBJS        := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJS))
DEPS        := $(subst .o,.d,$(OBJS))
-include $(DEPS)

CC          := clang++
CFLAGS      := -Wall -Wextra -Werror -std=c++98
CPPFLAGS    := -MMD -MP -I includes
RM          := rm -f
MAKE        := $(MAKE) --silent --jobs --no-print-directory
DIR_DUP		= mkdir -p $(@D)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $^ -L/usr/local/lib -lreadline -o $@
	$(info CREATED $(NAME))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)
docker:
	docker build -t webserv .
	docker run --rm -p 5000:5000 --name my_webserv webserv
docker_stop:
	docker stop my_webserv
clean:
	$(RM) -R $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re
.SILENT:

