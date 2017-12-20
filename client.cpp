
 /*
    .
    |1. get.topics.[option]                             |
    |               [0] All topics                      |
    |               [1] Contain me                      |
    |3. create.[topic name].[user_1]. .[user_n]         |
    |4. chat.[option].[target].[message]                |
    |            [0] send to topic                      |
    |            [1] send to user                       |
    |5. chat.[option].[target].[filename]                |
    |            [2] send to topic                      |
    |            [3] send to user                       |
    |6. subs.[topic_1]. .[topic_n]                      |
    |7. unsub.[topic_1]. .[topic_n]                     |
    |8. exit                                            |
    
    */

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
using namespace std;
#define RESET   "\033[0m"
//#define BLACK   "\033[30m"      /* Black */
//#define RED     "\033[31m"      /* Red */
//#define GREEN   "\033[32m"      /* Green */
//#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
//#define MAGENTA "\033[35m"      /* Magenta */
//#define CYAN    "\033[36m"      /* Cyan */
//#define WHITE   "\033[37m"      /* White */

#define CON_OK  0
#define CON_AUTH_FAILED  4
#define CON_ACC_EXISTED  5

int sockfd = 0;
char *username;
char *password;
int key=1;
void get_list_topics(int opt){
    int len;
    char *pp = encode_Listtp(opt, &len);
    write(sockfd, pp, len);
}

void get_list_users(char *topic){
    int len;
    char *pp = encode_Listusr(topic, &len);
    write(sockfd, pp, len);
}

void create_topic(char *tpname, User *first){
    CrTopic *cr = new CrTopic;
    cr->name = tpname;
    cr->first = first;
    printListUser(cr->first);
   
    int len;
    char *pp = encode_Creatp(cr, &len);
    write(sockfd, pp, len);
}

void chatting(int option, char *target, char *message){
    Publish *pub = new Publish;
    pub->topic_or_user = option;
    pub->sender_name = username;
    pub->target = target;
    if(option>1){
       int sent_sum;
       ssize_t read_bytes,sent_bytes,sent_file_size;
       char send_buffer[256];
       int fi;
       sent_sum=0;
       sent_file_size=0;
       if((fi=open(message,O_RDONLY))<0){
           cout<<RED<<"not find file ! \n"<<RESET;
           close(fi);
        
       }else{
           cout<<BLUE<<"sending file......\n"<<RESET;
           while((read_bytes=read(fi,send_buffer,256))>0){
               pub->message=new char[read_bytes];
               std::copy_n(send_buffer,read_bytes,pub->message);
               int len ;
               char *pp=encode_Publish(pub,&len);
               write(sockfd,pp,len);
           }
           close(fi);
       }
    }else{
        pub->message = message;
        int len;
        char *pp = encode_Publish(pub, &len);
        write(sockfd, pp, len);
    }
   
}

void subscribe(Topic *tp){
    int len;
    char *pp = encode_Subscribe(tp, &len);
    write(sockfd, pp, len);
}

void unsubscribe(Topic *tp){
    int len;
    char *pp = encode_Unsubscribe(tp, &len);
    write(sockfd, pp, len);
}


void disconnect(){
    int len;
    char *pp = encode_Disconnect(&len);
    write(sockfd, pp, len);
}

void* receiveMsg(void *ptr){
    int connfd = *(int *)ptr;

    while (1){
        char *buffer = new char[1024];
        read(connfd, buffer, 1024);
        int type = (*buffer >> 4) & 15;
        if(type == LISTTPACK ){
            Topic *top = decode_Listtpack(buffer);
            printListTopic(top);
            key = 1;
        } else if( type == LISTUSRACK) {
            User *first = decode_Listusrack(buffer);
            printListUser(first);
            key = 1;
        } else if (type == CREATPACK){
            CrTopicAck *ack = decode_Creatpack(buffer);
            printCreateTopicAck(ack);
            key = 1;
        } else if (type == SUBACK ){
            Code *first = decode_Suback(buffer);
            printListCode(first);
            key = 1;
        } else if (type == UNSUBACK){
            Code *first = decode_Unsuback(buffer);
            printListCode(first);
            key = 1;
        } else if(type == PUBACK){
            int ret = decode_Puback(buffer);
            if(ret == 0){
                cout<<"Noone receive message!\n";
            } else {
                cout<<"Some people received your message!\n";
            }
            key = 1;
        } else if (type == PUBLISH){
            Publish *pub = decode_Publish(buffer);
            printPublish(pub);
            cout<<GREEN <<">>> " <<RESET;
            fflush(stdout);
           
        }
        
    }   
}


int 
main(int argc, char *argv[]){
    
    struct sockaddr_in serv_addr;
    // char *IP_ADDR = "127.0.0.1";

    if (argc != 2) {
       cout<<RED<<"Error: Missing IP address!\n"<<RESET;
       exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout<<RED<<"Error: Create socket failed!\n"<<RESET;
        exit(1);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7777);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        cout<<RED<< "Error: IP failed!\n"<<RESET;
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout<<RED<<"Error: Connection failed!\n"<<RESET;
        exit(1);
    }

    username = new char[256];
    password = new char[256];

    cout<<YELLOW<<"Username:\n"<<RESET;
    cout<<GREEN<<">>> "<<RESET;
    fgets(username, 256, stdin);
    *(username + strcspn(username, "\n")) = 0;
  
    cout<<YELLOW<<"Password:\n"<<RESET;
    cout<<GREEN<<">>> "<<RESET;
    fgets(password, 256, stdin);
    *(password + strcspn(password, "\n")) = 0;

    Connect *con = new Connect;
    con->username = username;
    con->password = password;
    con->regist   =true;
   
    int len;
    char *code = encode_Connect(con, &len);
    write(sockfd, code, len);

    char *buff = new char[1024];
    read(sockfd, buff, 1024);
    int retCode = decode_Connack(buff);

    if (retCode == CON_AUTH_FAILED){
        cout<<RED<<"Wrong username or password!\n"<<RESET;
        return 0;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, &receiveMsg, &sockfd);


    const char *ch = ".";

    while (1){
        cout<<GREEN ">>> "<<RESET;
        char *input = new char[1024];
        fgets(input, 1024, stdin);
        *(input + strcspn(input, "\n")) = 0;
        fflush(stdin);
        char *tok = strtok(input, ch);

        key = 0;

        if (strcmp(tok, "get") == 0){
            tok = strtok(NULL, ch);
            if (strcmp(tok, "topics") == 0){
                int opt = atoi(strtok(NULL, ch));
                get_list_topics(opt);
            } else if (strcmp(tok, "users") == 0){
                tok = strtok(NULL, ch);
                get_list_users(tok);
            }
            

        } else if (strcmp(tok, "create") == 0){
            char *name = strtok(NULL, ch);
            User *u1 = NULL;
            while(1){
                tok = strtok(NULL, ch);
                if(tok == NULL){
                    break;
                } else {
                    u1 = addUser(u1, tok, 0);
                }
            }
            create_topic(name, u1);

        } else if (strcmp(tok, "chat") == 0){
            const char* ch1="\n";
            int opt = atoi(strtok(NULL, ch));
            char *target = strtok(NULL, ch);
            char *message = strtok(NULL, ch1);
        
            chatting(opt, target, message);


        } else if (strcmp(tok, "subs") == 0){
            Topic *tp = NULL;
            while(1){
                tok = strtok(NULL, ch);
                if(tok == NULL){
                    break;
                } else {
                    tp = addTopic(tp, tok);
                }
            }
            subscribe(tp);
        } else if (strcmp(tok, "unsub") == 0){
            Topic *tp = NULL;
            while(1){
                tok = strtok(NULL, ch);
                if(tok == NULL){
                    break;
                } else {
                    tp = addTopic(tp, tok);
                }
            }
            unsubscribe(tp);
        } else if (strcmp(tok, "exit") == 0){
            disconnect();
            cout<<BLUE<<"Bye bye!\n"<<RESET;
            key = 1;
            break;
        } else {
            cout<<YELLOW<<"Unkown!\n"<<RESET;
            key = 1;
            continue;
        }

        while(!key);
        delete []input;

    }
    
    return 0;
}
