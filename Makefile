NAME		:= webserv
SRC_DIR     := srcs/
SRCS        := Client.cpp Config.cpp Epoll.cpp EventListener.cpp main.cpp Server.cpp Webserv.cpp Request.cpp Response.cpp
SRCS        := $(SRCS:%=$(SRC_DIR)/%)
BUILD_DIR	:= .build
OBJS        := $(subst .cpp,.o,$(SRCS))
OBJS        := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJS))
DEPS        := $(subst .o,.d,$(OBJS))

CC          := clang++
CPPFLAGS    := -Wall -Wextra -Werror -std=c++98 -MMD -MP -I includes -g3
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

dbuild:
	docker build -t valgrind-webserv -f Dockerfile_valgrind .
	docker build -t webserv .
drun:
	docker run -ti --sig-proxy=true --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv webserv
dvrun:
	docker run --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv valgrind-webserv
drm:
	docker rm -f my_webserv
dclean:
	docker system prune --all --volumes --force

clean:
	$(RM) $(BUILD_DIR)

fclean: clean dclean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re
.SILENT:

-include $(DEPS)
