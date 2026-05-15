obj-m += lkmasg2reader.o
obj-m += lkmasg2writer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc test.c -o test

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test
