NAME		:=	webserv
SRC_DIR     :=	srcs
SRCS		:=	Client.cpp Config.cpp EventListener.cpp main.cpp Server.cpp Webserv.cpp\
				Request.cpp Response.cpp Location.cpp _utils.cpp Multipart.cpp
SRCS        := $(SRCS:%=$(SRC_DIR)/%)
BUILD_DIR	:= .build
OBJS        := $(subst .cpp,.o,$(SRCS))
OBJS        := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJS))
DEPS        := $(subst .o,.d,$(OBJS))

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -MMD -MP -I includes -g3

RM          := rm -rf
MAKE        := $(MAKE) --jobs --no-print-directory
DIR_DUP		= mkdir -p $(@D)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	$(info CREATED $(NAME))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	$(info CREATED $@)

dbuild:
	docker build -t webserv-img -f ./docker/my_webserv/Dockerfile .
	docker build -t nginx-img -f ./docker/nginx/Dockerfile .
	docker build -t siege-img -f ./docker/siege/Dockerfile .
drun: 
	docker run -d --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv webserv-img ./webserv conf/default.config
	docker attach my_webserv --sig-proxy=false

dnrun:
	docker run --rm -p 6001-6010:6001-6010 --name my_nginx nginx-img

dsrun:
	docker run -ti --rm --name siege_container --net host siege-img

dvrun:
	docker run --rm -p 5001-5010:5001-5010 --name my_webserv -v $(shell pwd):/webserv webserv-img valgrind --leak-check=full --show-leak-kinds=all ./webserv conf/default.config
drm:
	docker rm -f my_webserv
	docker rm -f my_nginx
dclean:
	docker system prune --all --volumes --force

clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re all
.SILENT:

-include $(DEPS)
