#include<bits/stdc++.h>
#include "data.hpp"
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
sqlite3 *db;
static int ch_exist_pr(void *data, int argc, char **argv, char **azColName){
    struct Acc *acc  = (struct Acc *)data;
    if(strcmp(acc->username, argv[1]) == 0){
        return  -1;
    }
    return 0;
}

/*authorized account */
static int auth_pr(void *data, int argc, char **argv, char **azColName){
    struct Acc *acc  = (struct Acc *)data;
    // printf("row: uname: %d, pass: %d\n", strcmp(acc->username, argv[0]), strcmp(acc->password, argv[1]));
    if(strcmp(acc->username, argv[0]) == 0 && strcmp(acc->password, argv[1]) == 0){
        return -1;
    }
    
    return 0;
}

static int  listtp_pr(void *data, int argc, char **argv, char **azColName){
    Topic *first = (Topic*) data;
    Topic *tp =new Topic;

    tp->name = new char[1024];
    strcpy(tp->name, argv[0]);
    tp->next = NULL;

    Topic *top = first;
    while(top->next != NULL){
        top = top->next;
    }
    top->next = tp;

    return 0;
}


static int  listusr_pr(void *data, int argc, char **argv, char **azColName){
    User *first = (User*) data;
    User *tp = new User;

    tp->name = new char[1024];
    strcpy(tp->name, argv[1]);
    tp->status = strcmp(argv[2], "on") == 0 ? ON : OFF;
    tp->next = NULL;

    User *top = first;
    while(top->next != NULL){
        top = top->next;
    }
    top->next = tp;

    return 0;
}

static int  publish_pr(void *data, int argc, char **argv, char **azColName){
    Code *first = (Code*)data;

    Code *cd = new Code;
    cd->retCode = atoi(argv[0]);
    cd->next = NULL;

    Code *q = first;
    while(q->next != NULL){
        q = q->next;
    }
    q->next = cd;

    return 0;
}


void opendb(){
    int rc = sqlite3_open("MQTT_data.db", &db);
    if (rc){
        cout<<RED<<"Can't open database: "<<sqlite3_errmsg(db)<<endl;
        return;
    } else {
        cout<<GREEN<<"Opened database successfully\n"<<RESET;
    }
    
}

void closedb(){
    sqlite3_close(db);
}

//================================================================================================================

/* db add user */
int db_addUser(char *username, char *password, int connfd){
    opendb();
    int rc;
    char *err = 0;
    char *sql1 = "select * from users;";
    struct Acc *acc = new struct Acc;
    acc->username = username;
    acc->password = password;

    rc = sqlite3_exec(db, sql1, ch_exist_pr, (void *) acc, &err);

    /* callback "ch_exist_pr" return 0 then RESULT CODE 'rc' = SQLITE_ABORT 
                                                            else continue insert user to db */
    if(rc == SQLITE_ABORT){
        return FAILED;
    }

    char sql2[1024];
    sprintf(sql2, "insert into users (uname, pass, connfd, status) values('%s', '%s', '%d', '%s');", username, password, connfd, "on");
    rc = sqlite3_exec(db, sql2, NULL, 0, &err);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
    }
    closedb();
    return OK;
}


// db authorized account 
int db_auth(char *username, char *password, int connfd){
    opendb();
    int rc;
    char *err;
    char *sql = "select * from users;";
    struct Acc *acc =new struct Acc;
    acc->username = username;
    acc->password = password;
    rc = sqlite3_exec(db, sql, auth_pr, (void *) acc, &err);
    if(rc == SQLITE_ABORT){
        char sql2[1024];
        sprintf(sql2, "update users set connfd = '%d', status = 'on' where uname = '%s';", connfd, username);
        sqlite3_exec(db, sql2, NULL, 0, &err);
        return OK;
    }
    closedb();
    return FAILED;
}

//=================================list all topic from db 
Topic *db_listTopic(char *sql){
    opendb();
    int rc;
    char *err;

    Topic *first = new Topic;
    first->name = "temp";
    first->next = NULL;
    printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, listtp_pr, (void*)first, &err);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return NULL;

    } else {
        // remove first topic 'temp' 
        first = first->next;
    }

    closedb();
    return first;
}

User *db_listUser(char *tpname){
    opendb();
    int rc;
    char *err;
    char sql[1024];

    sprintf(sql, "select tpname, tu.uname, status from topic_user as tu inner join users as u where tu.uname = u.uname and tpname = '%s';", tpname);

    User *first = new User;
    first->name = "temp";
    first->status = 0;
    first->next = NULL;

    rc = sqlite3_exec(db, sql, listusr_pr, (void*)first, &err);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return NULL;

    } else {
        // remove first topic 'temp' 
        first = first->next;
    }

    closedb();
    return first;
}



void db_exec_stm(char *sql){
    opendb();
    char *err;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return;

    }

    closedb();
}


CrTopicAck *db_createTopic(CrTopic *cr, char *user){
    opendb();
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    CrTopicAck *ack = new CrTopicAck;
    int rc;
    char sql1[1024];
    sprintf(sql1, "insert into topics (tpname, usrcreated) values ('%s', '%s');", cr->name, user);
    rc = sqlite3_exec(db, sql1, NULL, NULL, NULL);
    if( rc != SQLITE_OK ){
        ack->rc = RC_FAILED;
        return ack;
    }
    ack->rc = RC_SUCCESS;

    char sql2[1024];
    sprintf(sql2, "insert into topic_user (tpname, uname) values ('%s', '%s');", cr->name, user);
    cout<< sql2<<endl;
    rc = sqlite3_exec(db, sql2, NULL, NULL, NULL);

    Code *cd = NULL;
    User *u1 = cr->first;
    while(u1 != NULL){
        char sql[1024];
        printf("day %d\n", strcmp(cr->name, user));
        if(strcmp(u1->name, user) == 0){
            cd = addCode(cd, RC_SUCCESS);
            u1 = u1->next;
            continue;
        }
        sprintf(sql, "insert into topic_user (tpname, uname) values ('%s', '%s');", cr->name, u1->name);
        cout<< sql<<endl;
        rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
        if( rc == SQLITE_OK ){
            cd = addCode(cd, RC_SUCCESS);
        } else {
            cd = addCode(cd, RC_FAILED);
        }
        u1 = u1->next;
    }
    ack->first = cd;
    printListCode(cd);

    closedb();
    return ack;
}

Code * db_subscribe(Topic *top, char *user){
    opendb();
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    int rc;
    Code *first = NULL;
    Topic *tp = top;
    while(tp != NULL){
        char sql[1024];
        sprintf(sql, "insert into topic_user (tpname, uname) values ('%s', '%s');", tp->name, user);
        cout<<sql<<endl;
        rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
        if( rc == SQLITE_OK ){
            first = addCode(first, RC_SUCCESS);
        } else {
            first = addCode(first, RC_FAILED);
        }
        tp = tp->next;
    }
    closedb();
    return first;
}

Code * db_unsubscribe(Topic *top, char *user){
    opendb();
    int rc;
    Code *first = NULL;
    Topic *tp = top;
    while(tp != NULL){
        char sql[1024];
        sprintf(sql, "delete from topic_user where tpname = '%s' and uname = '%s';", tp->name, user);
        cout<<sql<<endl;
        rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
        if( rc == SQLITE_OK ){
            first = addCode(first, RC_SUCCESS);
        } else {
            first = addCode(first, RC_FAILED);
        }
        tp = tp->next;
    }
    closedb();
    return first;
}

int db_sendMessage(Publish *pub){
    printf("=======================db_sendMessage=====================\n");
    opendb();
    char sql[1024];
    int opt=(pub->topic_or_user)&1;
    if (opt == TOPIC){
        sprintf(sql, "select u.connfd, u.uname from topic_user tu inner join users u where tu.uname = u.uname and u.status = 'on' and tu.tpname = '%s' and u.uname != '%s';", pub->target, pub->sender_name);
    } else {
        sprintf(sql, "select connfd from users where uname = '%s' and status = 'on';", pub->target);
    }

    cout<<"sql: "<<sql<<endl;

    Code *first = new Code;
    first->retCode = 0;
    first->next = NULL;

    sqlite3_exec(db, sql, publish_pr, (void*) first, NULL);

    first = first->next;
    if (first == NULL){
        return 0;
    }
    int len;
    char *pp = encode_Publish(pub, &len);

    Code *pc = first;
    while(pc != NULL){
        write(pc->retCode, pp, len);
        pc = pc->next;
    }

    closedb();
    return 1;
}


