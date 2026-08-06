#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>
#include <stdlib.h>

#define SERV_ADDR "cluster.app.socket"
#define CLIE_ADDR "cluster.cli.socket"
static int socketfd = 0;

int main(void)
{
    struct sockaddr_un servaddr, cliaddr;
    char buf[1024];
    int socketfd = socket(AF_UNIX, SOCK_DGRAM, 0); //1.socket

    bzero(&cliaddr, sizeof(cliaddr));

    cliaddr.sun_family = AF_UNIX;
    cliaddr.sun_path[0] = '\0';
    strcpy(cliaddr.sun_path + 1, CLIE_ADDR);
    int clen = offsetof(struct sockaddr_un, sun_path)
            + strlen(CLIE_ADDR) + 1;

    unlink(CLIE_ADDR);
    bind(socketfd, (struct sockaddr *)&cliaddr, clen); //2.bind
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    servaddr.sun_path[0] = '\0';
    strcpy(servaddr.sun_path + 1, SERV_ADDR);
    int slen = offsetof(struct sockaddr_un, sun_path)
            + strlen(SERV_ADDR) + 1;

    printf("****************************************************\n");
    printf("                     CLUSTER CONSOLE                \n");
    printf("****************************************************\n");

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (strcmp(buf, "\n") == 0) 
            continue;
        if (strcmp(buf, "exit\n") == 0) {
            sendto(socketfd, "switch 0", strlen("switch 0"), 0, (struct sockaddr *)&servaddr, slen);
            break;
        } else {
            sendto(socketfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, slen);
        }
    }

    close(socketfd);

    return 0;
}
