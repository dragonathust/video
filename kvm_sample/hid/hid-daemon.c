#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <libgen.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif


static int do_exit = 0;

/* ctrl-c signal handler */
void sigintproc(int signum)
{
signum = signum;
do_exit = 1;
}

/* TERM signal handler */
void sigtermproc(int signum)
{
signum = signum;
do_exit = 1;
}

typedef enum {
		KEYBOARD	= 0x01,
		MOUSE		= 0x02,
}HIDDevice;

typedef struct  {
unsigned int type;
char report[8];
} hid_msg_s ;

hid_msg_s msg;

static char keyboard_dev_name[UNIX_PATH_MAX];
static char mouse_dev_name[UNIX_PATH_MAX];
int keyboard_dev_fd = 0;
int mouse_dev_fd = 0;
int port = 0;

int server_main(int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
    int sock;
    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }
    struct sockaddr_in clientAddr;
    int n;
    int len = sizeof(clientAddr);
	
		
    while (!do_exit)
    {
        n = recvfrom(sock, &msg, sizeof(hid_msg_s), 0, (struct sockaddr*)&clientAddr, &len);
        if(n<=0){
			if(errno == EINTR) {
			/* just a signal */
			continue;
			} 
	   perror("recv");
           break;
		}
		
		printf("HID 0x%x msg[0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x]\n",msg.type,
		msg.report[0],msg.report[1],msg.report[2],msg.report[3],
		msg.report[4],msg.report[5],msg.report[6],msg.report[7]);
		
		switch(msg.type)
		{
			case KEYBOARD:
				if (write(keyboard_dev_fd, msg.report, 8) != 8) {
				perror(keyboard_dev_name);
				}
			break;
			
			case MOUSE:
				if (write(mouse_dev_fd, msg.report, 4) != 4) {
				perror(mouse_dev_name);
				}			
			break;
			
			default:
			printf("Unknow message!\n");
			break;
		}
    }

    close(sock);
    return 0;

}

void usage__r(void)
{
	printf("USAGE: hid-daemon [-k keyboard device name] [-m mouse device name] [-p port]\n");
	printf("EXAMPLE: hid-daemon -k /dev/hidg0 -m /dev/hidg1 -p 4000\n");	
}

int main(int argc, char *argv[])
{
    int opt;
    char *end;
    int config = 0;
	
    while ((opt = getopt(argc, argv, "k:m:p:Hh")) != -1)
    {
        switch (opt)
        {
            case 'k':
                strncpy(keyboard_dev_name, argv[ optind - 1 ], sizeof(keyboard_dev_name));
				config++;
                break;

            case 'm':
                strncpy(mouse_dev_name, argv[ optind - 1 ], sizeof(mouse_dev_name));
				config++;
                break;
				
            case 'p':
                port = strtol(argv[ optind - 1 ], &end, 0);
				config++;
                break;

            case 'H':
            case 'h':
                usage__r();
                return 0;

            default:
                usage__r();
                return -1;
        }
    }

    if (config != 3)
    {
        usage__r();
        return -1;
    }

	printf("dev[%s,%s],port[%d]\n",keyboard_dev_name,mouse_dev_name,port);

	if ((keyboard_dev_fd = open(keyboard_dev_name, O_RDWR, 0666)) == -1) {
		perror(keyboard_dev_name);
		return -1;
	}
	
	if ((mouse_dev_fd = open(mouse_dev_name, O_RDWR, 0666)) == -1) {
		perror(mouse_dev_name);
		close(keyboard_dev_fd);
		return -1;
	}
	
 	/* setup signal handlers */
	//signal(SIGINT, sigintproc);
	//signal(SIGTERM, sigtermproc);

	server_main(port);

	close(mouse_dev_fd);	
	close(keyboard_dev_fd);

	return 0;
}


