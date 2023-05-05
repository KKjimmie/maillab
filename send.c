#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

char buf[MAX_SIZE+1];

/**
 * get file name from file path 
 * @param filepath 
*/
const char *get_filename(const char *filepath){
    char ch = '/'; 
    const char *q = strrchr(filepath, ch);
    if (q == NULL){
        q = filepath;
    } else {
        q ++;
    }
    return q;
}

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

/**
 * read all data from file
*/
char *read_file(const char* file_path){
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL){
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp);
    rewind(fp);
    char *data = NULL;
    if ((data = (char *)malloc(size + 1)) == NULL){
        fclose(fp);
        return NULL;
    }
    fread(data, size, 1, fp);
    data[size] = '\0';
    fclose(fp);
    return data;
}

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = ""; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = ""; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    const char* from = ""; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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

    // Send EHLO command and print server response
    const char* EHLO = "EHLO kjm\r\n"; // TODO: Enter EHLO command here
    send_with_msg(s_fd, EHLO, strlen(EHLO), 0, "send EHLO");

    // TODO: Print server response to EHLO command
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv EHLO");
    
    // TODO: Authentication. Server response should be printed out.
    // 1. Send AUTH LOGIN command and print server response
    const char* AUTH_LOGIN = "AUTH LOGIN\r\n";
    send_with_msg(s_fd, AUTH_LOGIN, strlen(AUTH_LOGIN), 0, "send AUTH LOGIN");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv AUTH_LOGIN");

    // 2. Send base64 encoded user
    char *user_base64 = encode_str(user);
    strcat(user_base64, "\r\n");
    send_with_msg(s_fd, user_base64, strlen(user_base64), 0, "send username");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv user");
    free(user_base64);

    // 3. Send base64 encoded password
    char *pass_base64 = encode_str(pass);
    strcat(pass_base64, "\r\n");
    send_with_msg(s_fd, pass_base64, strlen(pass_base64), 0, "send password");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv pass");
    free(pass_base64);

    // TODO: Send MAIL FROM command and print server response
    sprintf(buf, "MAIL FROM:<%s>\r\n", user);
    send_with_msg(s_fd, buf, strlen(buf), 0, "send MAIL FROM");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv MAIL FROM");

    // TODO: Send RCPT TO command and print server response
    sprintf(buf, "RCPT TO:<%s>\r\n", receiver);
    send_with_msg(s_fd, buf, strlen(buf), 0, "send RCPT TO");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv RCPT TO");
    
    // TODO: Send DATA command and print server response
    const char *DATA = "DATA\r\n";
    send_with_msg(s_fd, DATA, strlen(DATA), 0, "send DATA");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv DATA");

    // TODO: Send message data
    // 1. send header
    if (subject){
        sprintf(buf, "From: %s <%s>\nTo: %s\nSubject: %s\n"
            "Mime-Version: 1.0\nContent-Type: multipart/mixed; "
            "boundary=\"qwertyuiopasdfghjklzxcvbnm\"\r\n\r\n", from, user, receiver, subject);
    } else {
        sprintf(buf, "From: %s <%s>\nTo: %s\nMime-Version: 1.0\n"
            "Content-Type: multipart/mixed; "
            "boundary=\"qwertyuiopasdfghjklzxcvbnm\"\r\n\r\n", from, user, receiver);
    }
    send_with_msg(s_fd, buf, strlen(buf), 0, "send header");

    // 2. send message content
    struct stat st;
    if (msg != NULL){
        sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\n"
            "Content-Type: text/plain; charset=UTF-8\r\n\r\n");
        send_with_msg(s_fd, buf, strlen(buf), 0, "send msg boundary & header");
        if (stat(msg, &st) == 0){
            // if msg is a file
            FILE *fp = fopen(msg, "r");
            // read all data from msg
            char *data = read_file(msg);
            send_with_msg(s_fd, data, strlen(data), 0, "send message");
            free(data);
        } else {
            send_with_msg(s_fd, msg, strlen(msg), 0, "send message");
        }
    }
    send_with_msg(s_fd, "\r\n", 2, 0, "<CR><LF>");

    // Send attachment
    if(att_path != NULL && stat(att_path, &st) == 0){
        // send boundary
        sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\n");
        send_with_msg(s_fd, buf, strlen(buf), 0, "send boundary");
        // send attachment header
        // get file name
        const char *filename = get_filename(att_path);
        sprintf(buf, "Content-Type: application/octet-stream\n"
            "Content-Disposition: attachment; "
            "filename=\"%s\"\nContent-Transfer-Encoding: base64\r\n\r\n", filename);
        send_with_msg(s_fd, buf, strlen(buf), 0, "send attachment header");
        FILE *fp = fopen(att_path, "r");
        FILE *fp64 = fopen("tmp.txt", "w");
        encode_file(fp, fp64);
        fclose(fp); 
        fclose(fp64);
        char *att_data = read_file("tmp.txt");
        send_with_msg(s_fd, att_data, strlen(att_data), 0, "send attachment");
        send_with_msg(s_fd, "\r\n", 2, 0, "<CR><LF>");
        free(att_data);
        if (remove("tmp.txt") != 0){
            perror("Fail to remove tmp.txt");
        }
    }
    sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\n");
    send_with_msg(s_fd, buf, strlen(buf), 0, "send boundary");

    // TODO: Message ends with a single period
    send_with_msg(s_fd, end_msg, strlen(end_msg), 0, "send end msg");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv end msg");

    // TODO: Send QUIT command and print server response
    const char *QUIT = "QUIT\r\n";
    send_with_msg(s_fd, QUIT, strlen(QUIT), 0, "send QUIT");
    recv_and_print(s_fd, buf, MAX_SIZE, 0, "recv QUIT");

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
