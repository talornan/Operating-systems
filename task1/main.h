#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <dirent.h>
#include <errno.h>
#include <stdio.h>


#define	MAX_SIZE_CMD	256
#define ServerPort 5000
#define localHost "127.0.0.1"

void get_cmd();
void shell_prompt();
void openTcpConnection();
void out(char *output);
void localOutput();
void copyFile();
void DeleteFile();

