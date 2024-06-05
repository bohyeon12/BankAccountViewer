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
    //WSA ���� �ʱ�ȭ
    WSADATA ws;
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0)printf("WSA failed to initialize\n");
    else printf("WSA initialized successfully\n");

    //���� �ʱ�ȭ
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(nSocket < 0){
        printf("The socket not opened\n");
        ENDSERVER
    } else printf("The socket opened successfully\n");

    //����ȯ�� �ʱ�ȭ
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    //srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //DB����
    if (connectDB()) {
        printf("cannot connect the database server");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // u_long optval = 0;//optval == 0 �̸� blocking, != 0 �̸� non-blocking
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

    //���ϰ� ��Ʈ�� ���ε�
    nRet = bind(nSocket, (struct sockaddr*)&srv, sizeof(struct sockaddr));
    if(nRet < 0){
        printf("Fail to bind to local port\n");
        ENDSERVER
    } else printf("Successfully binded to local port\n");

    //Listen Ȱ��ȭ
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

        //���ο� �����̳� ó����û�� ���� ������ ��ٸ�
        nRet = select(nMaxFd+1,&fr,&fw,&fe, &tv);
        if(nRet > 0){//�������� �����ϰų� �޼����� ���� ��
            // printf("Data on port... Processing now...\n");
            //
            // if(FD_ISSET(nSocket, &fe)){
            //     printf("There is an exception. Skip this request\n");
            // }
            // // if(FD_ISSET(nSocket, &fw)){
            // / /     printf("Ready to write\n");
            // // }
            ProcessNewRequest();//���ο� ���� ��û ó��

        } else if(nRet == 0){//���� ��û�� ���ų� �ƹ� ���ϵ� �غ���� �ʾ��� ��
            printf("Nothing on port : %d\n",PORT);
        } else {//������ �����ϰ� �װͿ� ���� ó���� ���
            printf("Failed to connect\n");
            closeDB();
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

}

void ProcessNewRequest(){
    //���ο� �����û�� ���� ��
    int i = 0;
    if(FD_ISSET(nSocket, &fr)){
        int nLen = sizeof(struct sockaddr);
        int nClientSocket = accept(nSocket, NULL, &nLen);
        if(nClientSocket > 0){//����� Ŭ���̾�Ʈ���� fd�� ����


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
            if(FD_ISSET(nArrClient[i], &fr)){//Ŭ���̾�Ʈ�κ��� �޼����� ����
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
        printf("��û : %s\n",buff);
        buff[2] = '\0';
        char* cursor;
        if (strcmp("MJ", buff) == 0) {
            char* args[3] = {0,};
            readargs(buff[3], args, 3);
            int done;
            if ( done = registuser(args[0], args[1], args[2])) {
                send(nClientSocket, "MJ:ȸ������ ����", 17, 0);
            }
            else {
                send(nClientSocket, "MJ:ȸ������ ����", 17, 0);
            }
        }
        else if (strcmp("AC", buff) == 0) {
            char* ownerid = buff[3];
            char* result = viewasset(ownerid);
            if (result) {
                send(nClientSocket, result, strlen(result)+1, 0);
            }
            else {
                send(nClientSocket, "AC:��ȸ����", 9, 0);
            }
        }
        else if (strcmp("LI", buff) == 0) {
            char* args[2] = { 0, };
            readargs(buff[3], args, 2);
            int done;
            if (done = login(args[0], args[1]) ) {
                send(nClientSocket, "LI:�α��� ����", 15, 0);
            }
            else {
                send(nClientSocket, "LI:�α��� ����", 15, 0);
            }
        }
        else if (strcmp("LO", buff) == 0) {
            send(nClientSocket, "LO:�α׾ƿ�", 12, 0);
            closesocket(nClientSocket);
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == nClientSocket) {
                    nArrClient[i] = 0;
                    break;
                }
            }
        }
        else if (strcmp("DO", buff) == 0) {
            char* args[3] = { 0, };
            readargs(buff[3], args, 3);
            int done;
            if (done = deleteaccount(args[0], args[1], args[2])) {
                send(nClientSocket, "DO:���� ����", 13, 0);
            }
            else {
                send(nClientSocket, "DO:���� ����", 13, 0);
            }
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