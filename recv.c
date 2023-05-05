#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

/**
 * receive data from s_fd and print
*/
int  recv_and_print(int s_fd, void* buf, size_t size, int falgs, const char* error_msg){
    int r_size;
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    ((char *)buf)[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", (char *)buf);
    return r_size;
}

/**
 * send buf to s_fd and print msg
*/
void send_with_msg(int s_fd, const void* buf, size_t size, int falgs, const char* msg){
    printf(">>> %s\n", msg);
    send(s_fd, buf, size, falgs);
}

void recv_mail()
{
    const char* host_name = ""; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = ""; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET; // protocol family: ipv4
    // htons is a function that converts a 16-bit number from host byte order to network byte order.
    s_addr.sin_port = htons(port); // port
    s_addr.sin_addr.s_addr = inet_addr(dest_ip); // IP address
    bzero(&(s_addr.sin_zero), 8); // zero
    connect(s_fd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr));

    // Print welcome message
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv welcome");

    // TODO: Send user and password and print server response
    // 1. send USER <name>
    sprintf(buf, "USER %s\r\n", user);
    send_with_msg(s_fd, buf, strlen(buf), 0, buf);
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv USER");

    // 2. send PASS <pass>
    sprintf(buf, "PASS %s\r\n", pass);
    send_with_msg(s_fd, buf, strlen(buf), 0, buf);
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv PASS");

    // TODO: Send STAT command and print server response
    const char *STAT = "STAT\r\n";
    send_with_msg(s_fd, STAT, strlen(STAT), 0, STAT);
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv STAT");

    // TODO: Send LIST command and print server response
    const char *LIST = "LIST\r\n";
    send_with_msg(s_fd, LIST, strlen(LIST), 0, LIST);
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv LIST");

    // TODO: Retrieve the first mail and print its content
    // Attention: The data returned at one time may not be complete
    const char *RETR = "RETR 1\r\n";
    send_with_msg(s_fd, RETR, strlen(RETR), 0, RETR);
    r_size = recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv RETR");
    int len = atoi(buf + 4);
    len -= r_size;
    while(len > 0){
        r_size = recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv EMAIL CONTENT");
        len -= r_size;
    }

    // TODO: Send QUIT command and print server response
    const char *QUIT = "QUIT\r\n";
    send_with_msg(s_fd, QUIT, strlen(QUIT), 0, QUIT);
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv QUIT");

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
