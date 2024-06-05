#include<stdio.h>
#include<winsock2.h>
#define PORT 9909
#define ENDSERVER WSACleanup();return(EXIT_FAILURE);

struct sockaddr_in srv;
int nSocket;

int main(){
    int nRet = 0;
    //WSA 변수 초기화
    WSADATA ws;
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0)printf("WSA failed to initialize\n");
    else printf("WSA initialized successfully\n");

    //소켓 초기화
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(nSocket < 0){
        printf("The socket not opened\n");
        ENDSERVER
    } else printf("The socket opened successfully\n");

    //소켓환경 초기화
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    // srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //서버와 소켓 연결
    nRet = connect(nSocket, (struct sockaddr*)&srv, sizeof(srv));
    if(nRet < 0){
        printf("Failed to connect to the server\n");
        ENDSERVER
    } else {
        printf("Successfully connected to the server\n");
        char buff[255] = {0,};
        recv(nSocket, buff, 255, 0);
        printf("%s",buff);
    }
}