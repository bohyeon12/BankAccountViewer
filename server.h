#pragma once
#include <winsock2.h>
#include"DB.h"
#define PORT 9909
#define ENDSERVER WSACleanup();exit(EXIT_FAILURE);
#define MAXCLIENT 100
#define MAXLENOFMESSAGE 256
struct sockaddr_in srv;
fd_set fr, fw, fe; //File Descriptor

int nSocket;
int nArrClient[MAXCLIENT];
void ProcessNewRequest();
void ProcessNewMessage(int);
void readargs(char* buff, char** args, int n);
