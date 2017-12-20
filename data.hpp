
#ifndef DB_H
#define DB_H
#include<bits/stdc++.h>
extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h> 
}
#include "packet.hpp"

#define FAILED 3
#define OK 4

#define CON_OK  0
#define CON_AUTH_FAILED  4
#define CON_ACC_EXISTED  5

extern sqlite3 *db;

struct Acc{
	char *username;
	char *password;
        Acc():username(nullptr),password(nullptr){}
};

void opendb();

void  closedb();

int db_addUser(char *username, char *password, int connfd);

int db_auth(char *username, char *password, int connfd);

Topic *db_listTopic(char *sql);

User *db_listUser(char *tpname);

CrTopicAck * db_createTopic(CrTopic *cr, char *user);

void  db_exec_stm(char *sql);

Code * db_subscribe(Topic *first, char *user);

Code *db_unsubscribe(Topic *first, char *user);

int db_sendMessage(Publish *pub);




#endif
