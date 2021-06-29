.PHONY: clean

CC          :=	gcc
LD          :=	gcc
INCLUDES    :=  -I/usr/local/ssl/include
LIBDIR      :=  -L/usr/local/ssl/lib/ -Wl,-rpath,/usr/local/ssl/lib
CCFLAGS     :=	$(INCLUDES) -g -std=c99 -Wall -Werror -D_GNU_SOURCE
LDFLAGS     :=
exe         :=  test
obj         :=	multi-process.o opt.o common.o work.o
link        :=  -lcrypto

all:$(obj)
	$(LD) $(LDFLAGS) -o $(exe) $^ $(LIBDIR) $(link) 

%.o:%.c
	$(CC) ${CCFLAGS} -c $^ -o $@

clean:
	rm -rf $(obj) $(exe)
