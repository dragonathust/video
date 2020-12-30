ifdef INST
ifeq ("$(origin INST)", "command line")
INSTALL_DIR := $(INST)
endif
endif

CC          = $(CROSS_COMPILE)gcc
INCLUDE     = .
OBJS        = 
LDLIBS      = -lc -lrt -lgcc 
EXTRA_FLAGS = -lpthread
CFLAGS     += -Wall -O2
EXEC_TEST   = hid-test
EXEC_DAEMON = hid-daemon

all: $(EXEC_TEST) $(EXEC_DAEMON)

$(EXEC_TEST).o: $(OBJS) $(EXEC_TEST).c
	$(CC) $(CFLAGS) -c $(EXEC_TEST).c

$(EXEC_DAEMON).o: $(OBJS) $(EXEC_DAEMON).c
	$(CC) $(CFLAGS) -c $(EXEC_DAEMON).c

clean:
	rm -f *.elf *.gdb *.o $(EXEC_TEST) $(EXEC_DAEMON)

install:
	cp $(EXEC_TEST) $(EXEC_DAEMON) $(INSTALL_DIR)
	cp gadget-setup.sh $(INSTALL_DIR)
	cp kybd-descriptor.bin $(INSTALL_DIR)
