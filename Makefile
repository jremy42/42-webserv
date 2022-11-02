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
	docker build -t webserv-img -f ./docker/my_webserv/Dockerfile .
	docker build -t nginx-img -f ./docker/nginx/Dockerfile .
drun:
	docker run --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv webserv-img ./webserv conf/default.config &
dnrun:
	docker run --rm -p 6001-6010:6001-6010 --name my_nginx nginx-img
dvrun:
	docker run --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv webserv-img valgrind --leak-check=full --show-leak-kinds=all ./webserv conf/default.config
drm:
	docker rm -f my_webserv
	docker rm -f my_nginx
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
