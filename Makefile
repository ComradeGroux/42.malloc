GREENGREEN = \033[38;5;46m
RED = \033[0;31m
GREY = \033[38;5;240m
RESET = \033[0m

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME =		libft_malloc_$(HOSTTYPE).so
LIBNAME =	libft_malloc.so

CC =         gcc
CFLAGS =     -Wall -Wextra -Werror -O3 -fPIC
RM =         rm -rf

LINKER_SCRIPT =	-Wl,--version-script=version.map

DIR_H = headers/
DIR_S =	srcs/
CREATE_DIR_O = @mkdir -p objs
DIR_O =	objs/

SRCS_LIST =	malloc.c \
			free.c \
			realloc.c \
			show_alloc_mem.c \
			utils.c

SRCS =		${addprefix ${DIR_S}, ${SRCS_LIST}}

OBJS =		${SRCS:${DIR_S}%.c=${DIR_O}%.o}

DIR_LIBFT = libft/
LIBFT_INC = -I ${DIR_LIBFT}
LIBFT =	${DIR_LIBFT}libft.a
FT_LNK = -L ${DIR_LIBFT} -lft

LIBS = ${FT_LNK}

${NAME}: ${LIBFT} ${OBJS}
	@echo "$(RESET)[$(GREENGREEN)${NAME}$(RESET)]: ${NAME} Objects were created${GREY}"
	${CC} -shared ${OBJS} ${LIBS} -o ${NAME} ${LINKER_SCRIPT}
	@echo "$(RESET)[$(GREENGREEN)${NAME}$(RESET)]: ${NAME} created !"
	ln -sf ${NAME} ${LIBNAME}

${LIBFT}:
	@echo "[$(GREENGREEN)${NAME}$(RESET)]: Creating Libft...${GREY}"
	${MAKE} -sC ${@D}
	@echo "$(RESET)[$(GREENGREEN)${NAME}$(RESET)]: Libft Objects were created"

all: ${NAME}

test: ${NAME}
	gcc -Wall -Wextra -Werror -I${DIR_H} -Wl,-rpath,'$$ORIGIN' test.c -L. -lft_malloc -o test

test_double: ${NAME}
	gcc -Wall -Wextra -DTEST_DOUBLE_FREE -I${DIR_H} -Wl,-rpath,'$$ORIGIN' test.c -L. -lft_malloc -o test

		
${DIR_O}%.o:${DIR_S}%.c
	@printf "\033[38;5;240m"
	@mkdir -p ${DIR_O}
	${CC} ${CFLAGS} ${LIBFT_INC} -I${DIR_H} -o $@ -c $<

clean:
	@echo "[$(RED)${NAME}$(RESET)]: Cleaning ${NAME} Objects...${GREY}"
	${RM} ${OBJS}
	${RM} ${DIR_O}
	@echo "[$(RED)${NAME}$(RESET)]: ${NAME} Objects were cleaned${GREY}"

libclean:
	@echo "${RESET}[$(RED)${NAME}$(RESET)]: Cleaning Libft...${GREY}"
	${MAKE} -sC ${DIR_LIBFT} fclean
	@echo "${RESET}[$(RED)${NAME}$(RESET)]: Libft Objects were cleaned"

fclean: clean libclean
	@echo "${RESET}[$(RED)${NAME}$(RESET)]: Cleaning ${NAME}...${GREY}"
	${RM} ${NAME}
	${RM} ${LIBNAME}
	@echo "${RESET}[$(RED)${NAME}$(RESET)]: ${NAME} was cleaned"

re: fclean all

.PHONY: all clean fclean re libclean 
