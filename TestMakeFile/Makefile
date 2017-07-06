SRCS = $(wildcard *.c) #当前目录下面所有以.c 结尾的文件
OBJS = $(SRCS:.c=.o)
OBJSS := $(patsubst %.c,%.o,$(SRCS))
pomelo : main.o test.o
	gcc -o pomelo main.o test.o
	echo $(SRCS)
	echo $(OBJS)
	echo "hello"
	echo $(OBJSS)
main.o : main.c test.h
	gcc -c main.c
test.o : test.c test.h
	gcc -c test.c

clean :
	rm pomelo main.o test.o
