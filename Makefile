.PHONY: clean

CC			:=	gcc
LD			:=	gcc
CCFLAGS		:=	"-g"
LDFLAGS		:=
exe			:=  test
obj			:=	multi-process.o
link		:=

all:$(obj)
	$(LD) $(LDFLAGS) -o $(exe) $^ $(link) 

%.o:%.c
	$(CC) ${CCFLAGS} -c $^ -o $@

clean:
	rm -rf $(obj) $(exe)
