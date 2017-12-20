
#ifndef PACKET_H
#define PACKET_H
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

static uint32_t dRLength(char* bytes);
static uint8_t eRLength(uint32_t remainingLength, char* destination);
//FLAGS
#define CONNECT     1 /**/
#define CONNACK     2 /**/
#define PUBLISH     3
#define PUBACK      4
#define SUBSCRIBE   8
#define SUBACK      9
#define UNSUBSCRIBE 10
#define UNSUBACK    11 
#define CREATP      12 
#define CREATPACK   5 
#define LISTUSR     13 //list user 
#define LISTUSRACK  7 
#define LISTTP      0 //list topic 
#define LISTTPACK   6 
#define DISCONECT   14 

#define MAX_RM_LEN  4
#define HEADER_LEN  1


#define ALL         0
#define CONT_ME     1 /* CONTAIN 'ME' */
#define N_CONT_ME   2 /* NOT CONTAIN 'ME' */
#define OFF         0 // offline
#define ON          1 // online
#define RC_FAILED   0 
#define RC_SUCCESS  1
#define TOPIC       0 /* Send to TOPIC */
#define USER        1 /* Send to USER */
struct Topic {
    char *name;
    Topic *next;
    Topic():name(nullptr),next(nullptr){}
};
struct Code { //// list of RETURN CODE 1 for SUCCESS and 2 for FAILED
    char retCode;
    Code *next;
    Code():retCode('0'),next(nullptr){}
};
struct User{
    char *name;
    char status;
    User *next;
    User():name(nullptr),status('0'),next(nullptr){}
};
struct Connect {
    bool regist;
    char *username;
    char *password;
    Connect():regist(true),username(nullptr),password(nullptr){}
};
struct CrTopic{
    char *name;
    User *first;
    CrTopic():name(nullptr),first(nullptr){}
};
struct CrTopicAck{
    char rc; /* RESULT CODE */
    Code *first;
    CrTopicAck():rc('0'),first(nullptr){}
};
struct Publish {
    int topic_or_user;
    char *sender_name;
    char *target;
    char *message;
    Publish():topic_or_user(0),sender_name(nullptr),target(nullptr),message(nullptr){}
};
std::string current_time();

Code *addCode(Code *first, char retCode);

void printListCode(Code *cd);

User *addUser(User *first, char *name, char status);

void printListUser(User *first);

Topic *addTopic(Topic *first, char *name);

void printListTopic(Topic *top);

char *field(char *pm, int *len);

void printPacket(char *packet, int len);

void printConnect(Connect *con);



void printCreateTopic( CrTopic *cr);

void printCreateTopicAck( CrTopicAck *cr);


char *merge(char *ptr, char* string);

void printPublish( Publish *pub);

char *encode_Connect( Connect *conn, int *len);

Connect* decode_Connect(char *packet);

char *encode_Connack(int retCode, int *len);

int decode_Connack(char *packet);

char *encode_Publish(Publish *pub, int *len);

Publish *decode_Publish(char *packet);

char *encode_Puback(int retCode, int *len);

int decode_Puback(char *packet);

char *encode_Subscribe(Topic *first, int *len);

Topic *decode_Subscribe(char *packet);

char *encode_Suback(Code *first, int *len);

Code *decode_Suback(char *packet);


char *encode_Unsubscribe(Topic *un, int *len);

Topic *decode_Unsubscribe(char *packet);

char *encode_Unsuback(Code *first, int *len);

Code *decode_Unsuback(char *packet);


char *encode_Listtpack(Topic *first, int *len);

Topic *decode_Listtpack(char *packet);

char *encode_Listtp(int option, int *len);

int decode_Listtp(char *packet);

char *encode_Listusr(char *tpname, int *len);

char *decode_Listusr(char *packet);

char *encode_Listusrack(User *first, int *len);

User *decode_Listusrack(char *packet);


char *encode_Creatp(CrTopic *ctp, int *len);

CrTopic *decode_Creatp(char *packet);

char *encode_Creatpack(CrTopicAck* cr, int *len);

CrTopicAck *decode_Creatpack(char *packet);

char *encode_Disconnect(int *len);



#endif
