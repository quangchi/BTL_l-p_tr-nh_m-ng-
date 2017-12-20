#include <bits/stdc++.h>
extern "C"{
#include<stdio.h>
 #include<stdlib.h>
 #include<string.h>
 #include<errno.h>
 #include<ctype.h>
 #include<sys/types.h>
 #include<sys/socket.h>
 #include<netinet/in.h>
 #include<netdb.h>
 #include<unistd.h>
 #include<fcntl.h>
 #include<arpa/inet.h>
 #include<stdbool.h>


}
#include "packet.hpp"
#include "data.hpp"
using namespace std;
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */


Connect* _connect(char *buffer, int connfd, int *rc){
    Connect *con = decode_Connect(buffer);
    int len;
    int retCode;
    *rc = db_auth(con->username, con->password, connfd);
    if (*rc == FAILED){
        retCode = CON_AUTH_FAILED;
    } else {
        retCode = CON_OK;
    }
    char *packet = encode_Connack(retCode, &len);
    if (retCode == CON_OK){
         char sql[1024];
        sprintf(sql, "update users set status = 'on', connfd = '%d' where uname = '%s';", connfd, con->username);
        db_exec_stm(sql);
    }
    write(connfd, packet, len);
    return con;
}

void list_topic(int connfd, char* username, int opt){

    char sql[1024];
    if (opt == 0){
        sprintf(sql, "select * from topics;");
    } else if (opt == 1){
        sprintf(sql, "select * from topic_user where uname = '%s';", username);
    } else {
        sprintf(sql, "select tpname from topics where tpname not in (select tpname from topic_user where uname = '%s');", username);
    }
    int len;
    Topic *first = db_listTopic((char*)sql);
    char *pp = encode_Listtpack(first, &len);
    write(connfd, pp, len);
    printf("=====================function list topics end=============\n");

}

void list_user(int connfd, char *packet){
    printf("=====================function list users==================\n");
    char *uname = decode_Listusr(packet);
    int len;
    User *first = db_listUser(uname);
    char *pp = encode_Listusrack(first, &len);
    printPacket(pp, len);
    write(connfd, pp, len);
    
}

void create_topic(int connfd, char *buffer, char *user){
    
    CrTopic *cr = decode_Creatp(buffer);
    CrTopicAck *ack = db_createTopic(cr, user);
    printCreateTopicAck(ack);
    int len;
    char *pp = encode_Creatpack(ack, &len);
    write(connfd, pp, len);
    
}

void subscribe(int connfd, char *buffer, char *user){
    
    Topic *first = decode_Subscribe(buffer);
    Code *cd = db_subscribe(first, user);
    int len;
    char *pp = encode_Suback(cd, &len);
    write(connfd, pp, len);
    
}


void unsubscribe(int connfd, char *buffer, char *user){
    Topic *first = decode_Unsubscribe(buffer);
    Code *cd = db_unsubscribe(first, user);
    int len;
    char *pp = encode_Unsuback(cd, &len);
    write(connfd, pp, len);
    
}

void publish(int connfd, char *buffer, char *user){
    Publish *pub = decode_Publish(buffer);
    int ret = db_sendMessage(pub);
    int len;
    char *pp = encode_Puback(ret, &len);
    write(connfd, pp, len);
}


void disconnect(char *username){
    
    char sql[1024];
    sprintf(sql, "update users set status = 'off' where uname = '%s';", username);
    db_exec_stm(sql);
}

void* do_response(void *ptr){
	int connfd = *(int *)ptr;
	char *buffer =new char[1024];
    read(connfd, buffer, 1024);
    if(strlen(buffer) == 0){
        /* if client disconnect then server close connection */
        return NULL;
    }

    int rc;
    Connect *con = _connect(buffer, connfd, &rc);
    /* username & passoword */

    if(rc == FAILED){
        return NULL;
    }

	while(1){
		buffer = new char[1024];
        int len = read(connfd, buffer, 1024);
        if(len == 0){
            disconnect(con->username);
            break;
        }

        int type = (*buffer >> 4) & 15;
        switch (type){
            case LISTTP:
                list_topic(connfd, con->username, *buffer & 3);
                continue;

            case LISTUSR:
                list_user(connfd, buffer);
                continue;

            case CREATP:
                create_topic(connfd, buffer, con->username);
                continue;

            case SUBSCRIBE:
                subscribe(connfd, buffer, con->username);
                continue;

            case UNSUBSCRIBE:
                unsubscribe(connfd, buffer, con->username);
                continue;

            case PUBLISH:
                publish(connfd, buffer, con->username);
                continue;

            case DISCONECT:
                disconnect(con->username);
                break;

            default:
                break;
        }
        free(buffer);
        
	}
	cout<<RED<<"Client has disconnected\n"<<RESET;
	return NULL;
}

int main(){
	int listenfd = 0;
    int *connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    char cli_addr_str[100];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        cout<<RED<< "Error: Create listen socket failed!\n"<<RESET;
        exit(1);
    }
    int level=1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &level, sizeof(int)) < 0) {
        cout<<RED<<"Error: Set flag SO_REUSEADDR failed!\n"<<RESET;
        exit(1);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(7777);

    if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout<<RED <<"Error: Bind failed!\n"<<RESET;
        exit(1);
    }

    if (listen(listenfd, 20) < 0) {
        cout<<RED <<"Error: Listening failed!\n"<<RESET;
        exit(1);
    }

    while (1) {
        connfd = new int;
        *connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len);
        strcpy(cli_addr_str, inet_ntoa(cli_addr.sin_addr));
        fprintf(stdout, "Client connect: %s:%d\n", cli_addr_str, cli_addr.sin_port);
        if (*connfd < 0) {
            fprintf(stderr, "Error: Accept failed!\n");
        }

        pthread_t tid;
        pthread_create(&tid, NULL, &do_response, (void*) connfd);
    }

}
