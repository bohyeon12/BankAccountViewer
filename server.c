
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
    //srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //DB연결
    if (connectDB()) {
        printf("cannot connect the database server");
        ENDSERVER
    }

    // u_long optval = 0;//optval == 0 이면 blocking, != 0 이면 non-blocking
    // nRet = ioctlsocket(nSocket, FIONBIO, &optval);
    // if(nRet != 0){
    //     printf("ioctlsocket call failed\n");
    // } else {
    //     printf("ioctlsocket call passed\n");
    // }

    //setsocket
    int nOptVal = 0;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if(!nRet){
        printf("The setsockopt call successed\n");
    } else {
        printf("The setsockopt call failed\n");
        ENDSERVER
    }

    //소켓과 포트를 바인드
    nRet = bind(nSocket, (struct sockaddr*)&srv, sizeof(struct sockaddr));
    if(nRet < 0){
        printf("Fail to bind to local port\n");
        ENDSERVER
    } else printf("Successfully binded to local port\n");

    //Listen 활성화
    nRet = listen(nSocket, 5);
    if(nRet < 0){
        printf("Fail to start listen to local port\n");
        ENDSERVER
    } else printf("Successfully started listening local port\n");

    int nMaxFd = nSocket;//
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while(1){
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        for(int i = 0; i < MAXCLIENT; i++){
            if(nArrClient[i] != 0){
                FD_SET(nArrClient[i], &fr);
                FD_SET(nArrClient[i], &fe);
            }
        }

        //새로운 연결이나 처리요청이 있을 때까지 기다림
        nRet = select(nMaxFd+1,&fr,&fw,&fe, &tv);
        if(nRet > 0){//누군가가 연결하거나 메세지를 보낼 때
            // printf("Data on port... Processing now...\n");
            //
            // if(FD_ISSET(nSocket, &fe)){
            //     printf("There is an exception. Skip this request\n");
            // }
            // // if(FD_ISSET(nSocket, &fw)){
            // / /     printf("Ready to write\n");
            // // }
            ProcessNewRequest();//새로운 연결 요청 처리

        } else if(nRet == 0){//연결 요청이 없거나 아무 소켓도 준비되지 않았을 때
            printf("Nothing on port : %d\n",PORT);
        } else {//연결이 실패하고 그것에 대한 처리와 출력
            printf("Failed to connect\n");
            closeDB();
            ENDSERVER
        }
    }

}

void ProcessNewRequest(){
    //새로운 연결요청이 있을 시
    int i = 0;
    if(FD_ISSET(nSocket, &fr)){
        int nLen = sizeof(struct sockaddr);
        int nClientSocket = accept(nSocket, NULL, &nLen);
        if(nClientSocket > 0){//연결된 클라이언트정보 fd에 저장


            for(; i < MAXCLIENT; i++){
                if(nArrClient[i] == 0){
                    nArrClient[i] = nClientSocket;
                    send(nClientSocket, "Got the connection done successfully\n", 37,0);
                    break;
                }
            }
            if(i == MAXCLIENT){
                printf("No space for a new connection\n");
            }
        }
    } else {
        for(;i < MAXCLIENT; i++){
            if(FD_ISSET(nArrClient[i], &fr)){//클라이언트로부터 메세지를 받음
                ProcessNewMessage(nArrClient[i]);
            }
        }
    }
}

void ProcessNewMessage(int nClientSocket){
    char buff[MAXLENOFMESSAGE+1] = {0,};
    int nRet = recv(nClientSocket, buff, MAXLENOFMESSAGE, 0);
    if(nRet < 0){
        printf("Error on the connection... closing the connection for client\n");
        closesocket(nClientSocket);
        for(int i = 0; i < MAXCLIENT; i++){
            if(nArrClient[i] == nClientSocket){
                nArrClient[i] = 0;
                break;
            }
        }
    } else {
        printf("% s\n",buff);
        buff[2] = '\0';
        char* cursor;
        if (strcmp("MJ", buff) == 0) { //회원가입
            char* args[3] = {0,}; //이름, id, pw
            readargs(buff[3], args, 3);
            int done;

            int len = 17 + strlen(args[1]);
            char* message = malloc(sizeof(char)*len);

            if ( done = registuser(args[0], args[1], args[2])) {
                sprintf_s(message, len, "MJ:%s,회원가입 실패", args[1]);
            }
            else {
                sprintf_s(message, len, "MJ:%s,회원가입 성공", args[1]);
            }
            send(nClientSocket, message, len, 0);
            closesocket(nClientSocket);
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == nClientSocket) {
                    nArrClient[i] = 0;
                    break;
                }
            }

        }
        else if (strcmp("AC", buff) == 0) {
            char* ownerid = buff[3];
            char* result = NULL;
            result = viewasset(ownerid);

            int len = 0;
            char* message = NULL;
            
             if (result) {
                 len = 5 + strlen(ownerid) + strlen(result);
                 message = malloc(sizeof(char) * len);
                 sprintf_s(message, len, "AC:%s,%s", ownerid,result);
                 free(result);
            }
            else {
                 len = 13 + strlen(ownerid);
                 message = malloc(sizeof(char) * len);
                 sprintf_s(message, len, "AC:%s,조회실패", ownerid, result);
            }
            send(nClientSocket, message, len, 0);
            free(message);
        }
        else if (strcmp("LI", buff) == 0) {
            char* args[2] = { 0, };
            readargs(buff[3], args, 2);
            int len = 16 + strlen(args[1]);
            char* message = malloc(sizeof(char) * len);
            
            int done;
            if (done = login(args[0], args[1]) ) {
                sprintf_s(message, len, "LI:%s,로그인 실패", args[1]);
            }
            else {
                sprintf_s(message, len, "LI:%s,로그인 성공", args[1]);
            }
            send(nClientSocket, message, len, 0);
            free(message);
        }
        else if (strcmp("LO", buff) == 0) {
            char* ownerid = buff[3];
            int len = 13 + strlen(ownerid);
            char* message = malloc(sizeof(char) * len);
            sprintf_s(message, len, "LO:%s,로그아웃", ownerid);
            send(nClientSocket, message, len, 0);
            closesocket(nClientSocket);
            free(message);
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == nClientSocket) {
                    nArrClient[i] = 0;
                    break;
                }
            }
        }
        else if (strcmp("DO", buff) == 0) {//계좌삭제
            char* args[3] = { 0, };//은행,계좌번호,id
            readargs(buff[3], args, 3);
            int done;
            int len = 18 + strlen(args[2]);
            char* message = malloc(sizeof(char) * len);
            if (done = deleteaccount(args[0], args[1], args[2])) {
                sprintf_s(message, len, "LI:%s,계좌삭제 실패", args[2]);
            }
            else {
                sprintf_s(message, len, "LI:%s,계좌삭제 성공", args[2]);
            }
            send(nClientSocket, message, len, 0);
            free(message);
        }
        else if (strcmp("GP", buff) == 0) {
            char* ownerid = buff[3];
            char* result = NULL;
            int len = 0;
            char* message = NULL;
            result = getpercentileof(ownerid);
            if (result) {
                len = 5 + strlen(result) + strlen(ownerid);
                message = malloc(sizeof(char) * len);
                sprintf_s(message, len, "LI:%s,%s", ownerid,result);
                free(result);
            }
            else {
                len = 24 + strlen(ownerid);
                message = malloc(sizeof(char) * len);
                sprintf_s(message, len, "LI:%s,자산백분위조회 실패", ownerid);
            }
            send(nClientSocket, message, len, 0);
            free(message);
        }
       // else if (strcmp("EX", buff) == 0) {

        //}
    }
}

void readargs(char* cursor, char** args, int n) {
    args[0] = cursor;
    int i = 1;
    while (i < n) {
        if (*cursor == ',') {
            *cursor++ == '\0';
            args[i++] = cursor;
        }
    }
    return;
}