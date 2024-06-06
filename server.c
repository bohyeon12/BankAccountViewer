#include "server.h"

struct sockaddr_in srv;
fd_set fr, fw, fe; // File Descriptor

int nSocket;
int nArrClient[MAXCLIENT] = { 0 };

void ProcessNewRequest();
void ProcessNewMessage(int nClientSocket);
void readargs(char* buff, char** args, int n);

int main() {
    int nRet = 0;
    // Initialize WSA
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        printf("WSA failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    printf("WSA initialized successfully\n");

    // Initialize socket
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSocket < 0) {
        printf("The socket not opened\n");
        ENDSERVER
    }
    printf("The socket opened successfully\n");

    // Setup socket environment
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    // Set socket options
    int nOptVal = 1;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if (nRet != 0) {
        printf("The setsockopt call failed\n");
        ENDSERVER
    }
    printf("The setsockopt call succeeded\n");

    // Bind socket
    nRet = bind(nSocket, (struct sockaddr*)&srv, sizeof(struct sockaddr));
    if (nRet < 0) {
        printf("Failed to bind to local port\n");
        ENDSERVER
    }
    printf("Successfully binded to local port\n");

    // Start listening
    nRet = listen(nSocket, 10);
    if (nRet < 0) {
        printf("Failed to start listening on local port\n");
        ENDSERVER
    }
    printf("Successfully started listening on local port\n");

    int nMaxFd = nSocket;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (1) {
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        // Add client sockets to the set
        for (int i = 0; i < MAXCLIENT; i++) {
            if (nArrClient[i] != 0) {
                FD_SET(nArrClient[i], &fr);
                FD_SET(nArrClient[i], &fe);
                if (nArrClient[i] > nMaxFd) {
                    nMaxFd = nArrClient[i];  // Update nMaxFd to the highest socket descriptor
                }
            }
        }

        // Wait for new connection or message
        nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
        if (nRet > 0) {
            ProcessNewRequest(); // Process new connection requests
        }
        else if (nRet == 0) {
            printf("Nothing on port : %d\n", PORT);
        }
        else {
            printf("Failed to connect\n");
            ENDSERVER
        }
    }
}

void ProcessNewRequest() {
    if (FD_ISSET(nSocket, &fr)) {
        int nLen = sizeof(struct sockaddr);
        int nClientSocket = accept(nSocket, NULL, &nLen);
        if (nClientSocket > 0) {
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == 0) {
                    nArrClient[i] = nClientSocket;
                    printf("Got the connection done successfully\n");
                    break;
                }
            }
        }
        else {
            printf("No space for a new connection\n");
        }
    }

    for (int i = 0; i < MAXCLIENT; i++) {
        if (nArrClient[i] != 0 && FD_ISSET(nArrClient[i], &fr)) {
            ProcessNewMessage(nArrClient[i]);
        }
    }
}

void ProcessNewMessage(int nClientSocket) {
    char buff[MAXLENOFMESSAGE + 1] = { 0 };
    int nRet = recv(nClientSocket, buff, MAXLENOFMESSAGE, 0);
    if (nRet <= 0) {
        printf("Error on the connection... closing the connection for client\n");
        closesocket(nClientSocket);
        for (int i = 0; i < MAXCLIENT; i++) {
            if (nArrClient[i] == nClientSocket) {
                nArrClient[i] = 0;
                break;
            }
        }
    }
    else {
        printf("%s\n", buff);
        buff[2] = '\0';
        if (strcmp("MJ", buff) == 0) { // ȸ������
            char* args[4] = { NULL, }; // id, pw, �̸�, ����
            readargs(buff + 3, args, 4);
            printf("args %s,%s,%s,%s\n", args[0], args[1], args[2], args[3]);
            int done = 1;

            char message[25] = { 0 };
            done = registuser(args[0], args[1], args[2], args[3]);
            sprintf_s(message, 25, "MJ:%s,%d", args[1], done);
            printf("%s\n", message);
            send(nClientSocket, message, strlen(message), 0);
        }
        else if (strcmp("AC", buff) == 0) {
            char* id = buff + 3;
            char* result = viewasset(id);

            int len = 0;
            char* message = NULL;
            if (result) {
                len = 5 + strlen(id) + strlen(result);
                message = malloc(len);
                sprintf_s(message, len, "AC:%s,%s", id, result);
                free(result);
            }
            else {
                message = malloc(25);
                sprintf_s(message, 25, "AC:%s,0", id);
            }
            send(nClientSocket, message, strlen(message), 0);
            free(message);
        }
        // Add other commands similarly
    }
}

void readargs(char* cursor, char** args, int n) {
    args[0] = cursor;
    int i = 1;
    while (i < n) {
        if (*cursor == ',') {
            *cursor++ = '\0';
            args[i++] = cursor;
        }
        cursor++;
    }
}

/*
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
        ENDSERVER
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
    nRet = listen(nSocket, 10);
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
            ENDSERVER
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
                    printf("Got the connection done successfully\n");
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
        printf("% s\n",buff);
        buff[2] = '\0';
        char* cursor;
        if (strcmp("MJ", buff) == 0) { //ȸ������
            char* args[4] = {NULL,}; //id, pw, �̸�, ����
            readargs(buff[3], args, 4);
            printf("%s,%s,%s,%s\n", args[0], args[1], args[2], args[3]);
            int done = 1;

            char message[25] = { NULL, };

            done = registuser(args[0],args[1], args[2], args[3]);
            sprintf_s(message, 25, "MJ:%s,%d", args[1],done);
            printf("%s\n", message);
            send(nClientSocket, message, strlen(message), 0);
            closesocket(nClientSocket);
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == nClientSocket) {
                    nArrClient[i] = 0;
                    break;
                }
            }

        }
        else if (strcmp("AC", buff) == 0) {
            char* id = buff[3];
            char* result = NULL;
            result = viewasset(id);

            int len = 0;
            char* message = NULL;
            
             if (result) {
                 len = 5 + strlen(id) + strlen(result);
                 message = malloc(sizeof(char) * len);
                 sprintf_s(message, len, "AC:%s,%s", id,result);
                 free(result);
            }
            else {
                 message = malloc(sizeof(char) * 25);
                 sprintf_s(message, 25, "AC:%s,0", id, result);
            }
            send(nClientSocket, message, len, 0);
            free(message);
        }
        else if (strcmp("LI", buff) == 0) {
            char* args[2] = { 0, };
            readargs(buff[3], args, 2);
            char message[25] = { 0, };
            
            int done = login(args[0],args[1]);
            sprintf_s(message, 25, "LI:%s,%d", args[0],done);
            send(nClientSocket, message, strlen(message), 0);
        }
        else if (strcmp("LO", buff) == 0) {
            char* ownerid = buff[3];
            char message[25] = {0,};
            sprintf_s(message, 25, "LO:%s,0", ownerid);
            send(nClientSocket, message, strlen(message), 0);
            closesocket(nClientSocket);
            free(message);
            for (int i = 0; i < MAXCLIENT; i++) {
                if (nArrClient[i] == nClientSocket) {
                    nArrClient[i] = 0;
                    break;
                }
            }
        }
        else if (strcmp("DO", buff) == 0) {//���»���
            char* args[4] = { 0, };//id,pw,����,���¹�ȣ
            readargs(buff[3], args, 4);
            int done = login(args[0], args[1]);
            char message[25] = { 0, };
            if (done != 0) {
                sprintf_s(message, 25, "DO:%s,%d", args[0],done);
                send(nClientSocket, message, strlen(message), 0);
            }
            else {
                done = deleteaccount(args[0], args[2], args[3]);
                sprintf_s(message, 25, "DO:%s,%d", args[2], done);
                send(nClientSocket, message, strlen(message), 0);
            }
        }
        else if (strcmp("GP", buff) == 0) {
            char* ownerid = buff[3];
            char* result = NULL;
            result = getpercentileof(ownerid);
            char message[35] = { 0, };
            if (result) {
                sprintf_s(message, 35, "GP:%s,%s", ownerid,result);
                free(result);
            }
            else {
                sprintf_s(message, 25, "GP:%s,0", ownerid);
            }
            send(nClientSocket, message, strlen(message), 0);
        }
        else if (strcmp("IA", buff) == 0) {
            char* args[5] = { NULL, }; //id,pw,���¹�ȣ,�����̸�,�ܾ�
            readargs(buff[3], args, 5);
            int done = login(args[0], args[1]);
            char message[25] = { 0, };
            if (done != 0) {
                sprintf_s(message, 25, "IA:%s,%d", args[0],done);
                send(nClientSocket, message, strlen(message), 0);
            }
            else {
                done = putaccount(args[0], args[2], args[3], args[4]);
                sprintf_s(message, 25, "IA:%s,%d", args[2], done);
                send(nClientSocket, message, strlen(message), 0);
            }
        }
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
        cursor++;
    }
    return;
}*/