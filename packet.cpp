
#include "packet.hpp"
#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;
static uint32_t dRLength(char* bytes) {
    uint32_t multiplier = 1;
    uint32_t value = 0;
    uint8_t currentByte = 0;
    uint8_t encodedByte;
    do {
      encodedByte = bytes[currentByte++];
      value += (encodedByte & 127) * multiplier;
      multiplier *= 128;
    } while ((encodedByte & 128) != 0);

    return value;
  }


static uint8_t eRLength(uint32_t remainingLength, char* destination) {
    uint8_t currentByte = 0;
    uint8_t bytesNeeded = 0;

    do {
      uint8_t encodedByte = remainingLength % 128;
      remainingLength /= 128;
      if (remainingLength > 0) {
        encodedByte = encodedByte | 128;
      }

      destination[currentByte++] = encodedByte;
      bytesNeeded++;
    } while (remainingLength > 0);

    return bytesNeeded;
}
std::string current_time(){
    time_t now = time(NULL);
    struct tm tstruct;
    char buf[40];
    tstruct = *localtime(&now);
    
    strftime(buf, sizeof(buf), "%A:%d:%m:%Y:", &tstruct);
    time_t now1 = time(NULL);
    struct tm tstruct1;
    char buf1[40];
    tstruct1 = *localtime(&now);
    
    strftime(buf1, sizeof(buf1), "%X", &tstruct1);
    strcat(buf,buf1);
    return buf;

}
//add code into list Code 
Code *addCode(Code *first, char c){

    Code *cd =new Code;
    cd->retCode = c;
    cd->next = NULL;
    if (first == NULL){   
        return cd;
    }
    Code *q = first;
    while(q->next != NULL){
        q = q->next;
    }
    q->next = cd;
    return first;
}

void printListCode(Code *q){
    Code *pt = q;
    int i = 1;
    while (pt != NULL){
        printf("%d. %s\n", i, pt->retCode ? YELLOW "Ok" RESET : RED "Failed" RESET);
        pt = pt->next;
        i++;
    }
}

Topic *addTopic(Topic *first, char *name){
   
    Topic *tp = new Topic;
    tp->name = name;
    tp->next = NULL;
    if (first == NULL){   
        return tp;
    }
    Topic *top = first;
    while(top->next != NULL){
        top = top->next;
    }
    top->next = tp;
    return first;
}

void printListTopic(Topic *top){
    if(top == NULL){
        cout<<RED<<"There is no topic for your order!\n"<<RESET;
        return;
    }
    Topic *pt = top;
    while (pt != NULL){
        cout<<"Topic name: "<<BLUE<<pt->name<<RESET;
        pt = pt->next;
    }
}

User *addUser(User *first, char *name, char status){
    User *us =new User;
    us->name = name;
    us->status = status;
    us->next = NULL;

    if (first == NULL){   
        return us;
    }
    User *q = first;
    while(q->next != NULL){
        q = q->next;
    }
    q->next = us;
    return first;
}

void printListUser(User *first){
    if (first == NULL){
        cout<<RED<<"There is no user for your order!\n"<<RESET;
    }
    User *u = first;
    int i = 1;
    while (u != NULL){
        printf("%d. %s %s\n", i, u->status ? YELLOW "Online" RESET : RED "Off   " RESET, u->name);
        u = u->next;
        i++;
        
    }
}
static char* encodeList(Topic *top, int *len){
    int count = 0;
    int ploadLen = 0;
    Topic *pt = top;
    while (pt != NULL){
        count++;
        ploadLen = ploadLen + 2 + strlen(pt->name);
        pt = pt->next;
    }
    char *packet = new char[2 + ploadLen];
    char *ptr_packet = packet;
    ptr_packet++;
    *ptr_packet = count;
    ptr_packet++;
    pt = top;
    while(pt != NULL){
        char* dlength=new char[2];
        eRLength(strlen(pt->name),dlength);
        std::copy_n(dlength,2,ptr_packet);
        ptr_packet=ptr_packet+2;
        strncpy(ptr_packet, pt->name, strlen(pt->name));
        ptr_packet = ptr_packet + strlen(pt->name);
        pt = pt->next;
        delete []dlength;
    }
    *len = 2 + ploadLen;

    return packet;
}

static Topic *decodeList(char* packet){
    Topic *first = NULL;
    char *ptr_packet = packet;
    ptr_packet++;
    int count = *ptr_packet;
    ptr_packet++;
    int len = 0;
    int i = 0;
    while(i < count){
        char *tpname = field(ptr_packet, &len);
        first = addTopic(first, tpname);
        ptr_packet = ptr_packet + len;
        i++;
    }
    return first;
}

static char *encodeListCode(Code *first, int *len){
    Code *c = first;
    int count = 0;

    while(c != NULL){
        count++;
        c = c->next;
    }
    char *packet = new char[count + HEADER_LEN + 1];
    char *ptr_packet = packet;
    ptr_packet++;
    *ptr_packet = count;
    ptr_packet++;
    c = first;
    while(c != NULL){
        *ptr_packet = c->retCode;
        ptr_packet++;
        c = c->next;
    }
    *len = HEADER_LEN + count + 1;
    return packet;
}

static Code *decodeListCode(char *packet){
    char *ptr_packet = packet;
    ptr_packet++;
    Code *first = NULL;
    int count = *ptr_packet;
    int i = 0;
    while (i < count){
        ptr_packet++;
        first = addCode(first, *ptr_packet);
        i++;
    }

    return first;
}


//======================================================================================================

char *field(char *pm, int *len){
    char* dlength=new char[2];
    std::copy_n(pm,2,dlength);
    *len=dRLength(dlength);
    char *data = new char[*len - 2];
    strncpy(data, pm + 2, *len - 2);
    *(data + *len - 2) = 0;
    delete []dlength;
    return data;

}

void printPacket(char *packet, int len){
    char *ptr = packet;
    for(int i = 0; i < len; i++){
        printf("char[%d]: %c \tvalue of char: %d\n", i, *(ptr + i), *(ptr + i));
    }
}

void printConnect(Connect *con){
    
    printf("register: %s\n", con->regist ? "True" : "False");
    cout<<"username:"<<con->username<<endl;
    cout<<"password: "<<con->password<<endl;
}

void printCreateTopic(CrTopic *cr){

    cout<<"name: "<<cr->name<<endl;
    printListUser(cr->first);
}


void printCreateTopicAck(CrTopicAck *cr){
    cout<<"retCode: "<<cr->rc<<endl;
    if(cr->rc == 1){
        printListCode(cr->first);
    }
}

void printPublish(Publish *pub){
    if(pub->topic_or_user>1){
        int fi;
        string name(pub->target,pub->target+strlen(pub->target));
        string name1(current_time());
        name="("+name1+")"+name;
        int len =name.length();
        char* namef=new char[len];
        std::copy_n(name.begin(),len,namef);
        if((fi=open(namef,O_WRONLY|O_CREAT,0644))<0) {
	        cerr<<"lỗi tạo file "<<endl;
	        exit(1);
	    }else{
          int lenf=strlen(pub->message);
          write(fi,pub->message,lenf);
        }
        close(fi);

    }else{
        if (pub->topic_or_user == TOPIC){
            cout<<"From topic"<<GREEN<<pub->target<<RESET<<" "<<GREEN<<pub->sender_name<<RESET<<" said : "<<YELLOW<<pub->message<<RESET<<endl;
        } else {
            cout<<GREEN<<pub->sender_name<<RESET<<" have sent message to you: "<<YELLOW<<pub->message<<RESET<<endl;
        }
    }
}



char *merge(char *ptr, char* str){
    eRLength(strlen(str),ptr);
    strcat(ptr + 2, str);
    ptr = ptr + 2 + strlen(str);
    return ptr;
}

char *encode_Connect( Connect *con, int *len){
    
    int ploadLen = 0;
    ploadLen = ploadLen + 2 + strlen(con->username);
    ploadLen = ploadLen + 2 + strlen(con->password);
    int packetLen = HEADER_LEN + ploadLen;
    char *packet = new char[packetLen];
    char *ptr_packet = packet;
    *ptr_packet = CONNECT << 4;
    *ptr_packet |= con->regist;
    ptr_packet = ptr_packet + HEADER_LEN;
    ptr_packet = merge(ptr_packet, con->username);
    ptr_packet = merge(ptr_packet, con->password);
    *len = packetLen;
    return packet;
}

Connect* decode_Connect(char *packet){
    char *ptr_packet = packet;
    Connect *con = new Connect;
    con->regist = *ptr_packet & 1;
    ptr_packet = ptr_packet + 1;
    int len = 0;
    con->username = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;
    con->password = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;
    return con;
}

char *encode_Connack(int retCode, int *len){
    char *packet = new char;
    *packet = CONNACK << 4;
    *packet |= retCode;
    *len = 1;
    return packet;
}

int decode_Connack(char *packet){
    return *packet  & 15;
}

char *encode_Publish(Publish *pub, int *len){
    int ploadLen = 0;
    ploadLen = ploadLen + 2 + strlen(pub->sender_name);
    ploadLen = ploadLen + 2 + strlen(pub->target);
    ploadLen = ploadLen + 2 + strlen(pub->message);

    char *packet = new char[ploadLen + HEADER_LEN];
    char *ptr_packet = packet;
    *ptr_packet = PUBLISH << 4;
    *ptr_packet |= pub->topic_or_user;
    ptr_packet++;

    char* dlength=new char[2];
    eRLength(strlen(pub->sender_name),dlength);
    std::copy_n(dlength,2,ptr_packet);
    ptr_packet=ptr_packet+2;
    strncpy(ptr_packet, pub->sender_name, strlen(pub->sender_name));
    ptr_packet = ptr_packet + strlen(pub->sender_name);
    
    eRLength(strlen(pub->target),dlength);
    std::copy_n(dlength,2,ptr_packet);
    ptr_packet=ptr_packet+2;
    strncpy(ptr_packet, pub->target, strlen(pub->target));
    ptr_packet = ptr_packet + strlen(pub->target);

    eRLength(strlen(pub->message),dlength);
    std::copy_n(dlength,2,ptr_packet);
    ptr_packet=ptr_packet+2;
    strncpy(ptr_packet, pub->message, strlen(pub->message));
    ptr_packet = ptr_packet + strlen(pub->message);


    *len = ploadLen + HEADER_LEN;
    return packet; 

}

Publish *decode_Publish(char *packet){
    char *ptr_packet = packet;
    Publish *pub = new struct Publish;
    pub->topic_or_user = *ptr_packet & 1;
    ptr_packet++;

    int len;
    pub->sender_name = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;

    pub->target = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;

    pub->message = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;

    return pub;
}

char *encode_Puback(int retCode, int *len){
    char *packet = new char;
    *packet = PUBACK << 4;
    *packet |= retCode;
    *len = 1;
    return packet;
}

int decode_Puback(char *packet){
    return *packet & 15;
}

char *encode_Listtpack(Topic *top, int *len){
    char *packet = encodeList(top, len);
    *packet = LISTTPACK << 4;
    return packet;
}

Topic *decode_Listtpack(char *packet){
    return decodeList(packet);
}

char *encode_Listtp(int option, int *len){
    char *packet = new char;
    *packet = LISTTP << 4;
    *packet |= option;
    *len = 1;
    return packet;
}

int decode_Listtp(char *packet){
    return *packet & 3;
}

char *encode_Listusrack(User *first, int *len){
    int count = 0;
    int ploadLen = 0;
    User *pu = first;
    while (pu != NULL){
        count++;
        ploadLen = ploadLen + 3 + strlen(pu->name);
        pu = pu->next;
    }
    char *packet = new char[2 + ploadLen];
    char *ptr_packet = packet;
    *ptr_packet = LISTUSRACK << 4;
    ptr_packet++;
    *ptr_packet = count;
    ptr_packet++;
    pu = first;
    while(pu != NULL){
        char* dlength=new char[2];
        eRLength(strlen(pu->name),dlength);
        std::copy_n(dlength,2,ptr_packet);
        ptr_packet=ptr_packet+2;
        strncpy(ptr_packet, pu->name, strlen(pu->name));
        ptr_packet = ptr_packet + strlen(pu->name);
        *ptr_packet = pu->status;
        ptr_packet++;
        pu = pu->next;
    }
    *len = 2 + ploadLen;
    return packet;
}

User *decode_Listusrack(char *packet){
    char *ptr_packet = packet;
    ptr_packet++;
    User *first = NULL;
    int count = *ptr_packet;
    ptr_packet++;
    int i = 0;
    while(i < count){
        int len;
        char *name = field(ptr_packet, &len);
        ptr_packet = ptr_packet + len;
        char status = *ptr_packet;
        first = addUser(first, name, status);
        ptr_packet++;
        i++;
    }
    return first;
}

char *encode_Listusr(char *topic, int *len){
    int ploadLen = HEADER_LEN + 2 + strlen(topic);
    char *packet =new char[ploadLen];
    char *ptr_packet = packet;
    *ptr_packet |= LISTUSR << 4;
    ptr_packet++;
    char* dlength=new char[2];
    eRLength(strlen(topic),dlength);
    std::copy_n(dlength,2,ptr_packet);
    ptr_packet=ptr_packet+2;
    strcpy(ptr_packet, topic);
    *len = ploadLen;
    return packet;
}

char *decode_Listusr(char *packet){
    char *ptr_packet = packet;
    ptr_packet++;
    int len;
    return field(ptr_packet, &len);
}


char *encode_Creatp( CrTopic *ctp, int *len){
    int ploadLen = 0;
    ploadLen = ploadLen + 2 + strlen(ctp->name);
    User *f = ctp->first;
    int count = 0;
    while (f != NULL){
        ploadLen = ploadLen + 2 + strlen(f->name);
        f = f->next;
        count++;
    }
    char *packet = new char[2 + ploadLen];
    char *ptr_packet = packet;
    *ptr_packet = CREATP << 4;
    ptr_packet++;
    char* dlength=new char[2];
    eRLength(strlen(ctp->name),dlength);
    std::copy_n(dlength,2,ptr_packet);
    ptr_packet=ptr_packet+2;
    strcpy(ptr_packet, ctp->name);
    ptr_packet = ptr_packet + strlen(ctp->name);
    *ptr_packet = count;
    ptr_packet++;
    f = ctp->first;
    while (f != NULL){
        eRLength(strlen(f->name),dlength);
        std::copy_n(dlength,2,ptr_packet);
        ptr_packet=ptr_packet+2;
        strcpy(ptr_packet, f->name);
        ptr_packet = ptr_packet + strlen(f->name);
        f = f->next;
    }
    delete []dlength;
    *len = 2 + ploadLen;
    return packet;
}

CrTopic *decode_Creatp(char *packet){
    CrTopic *cr = new  CrTopic;
    char *ptr_packet = packet;
    ptr_packet++;
    int len;
    cr->name = field(ptr_packet, &len);
    ptr_packet = ptr_packet + len;

    User *u1 = NULL;
    int count = *ptr_packet;
    ptr_packet++;
    int i = 0;
    while (i < count){
        char *name = field(ptr_packet, &len);
        ptr_packet = ptr_packet + len;
        u1 = addUser(u1, name, 0);
        i++;
    }
    cr->first = u1;
    return cr;
}

char *encode_Creatpack(CrTopicAck* cr, int *len){
    char *packet = encodeListCode(cr->first, len);
    *packet = CREATPACK << 4;
    *packet |= cr->rc;
    return packet;
}


CrTopicAck *decode_Creatpack(char *packet){
    CrTopicAck *cr = new CrTopicAck;
    char *ptr_packet = packet;
    cr->rc = *ptr_packet & 1;
    if(cr->rc == 1){
        cr->first = decodeListCode(packet);
    }
    return cr;
}

char *encode_Subscribe(Topic *first, int *len){
    char *packet = encodeList(first, len);
    *packet = SUBSCRIBE << 4;
    return packet;
}

Topic *decode_Subscribe(char *packet){
    return decodeList(packet);
}

char *encode_Suback(Code *first, int *len){
    char *packet = encodeListCode(first, len);
    *packet = SUBACK << 4;
    return packet;
}

Code *decode_Suback(char *packet){
    return decodeListCode(packet);
}

char *encode_Unsubscribe(Topic *first, int *len){
    char *packet = encodeList(first, len);
    *packet = UNSUBSCRIBE << 4;
    return packet;
}

Topic *decode_Unsubscribe(char *packet){
    return decodeList(packet);
}

char *encode_Unsuback(Code *first, int *len){
    char *packet = encodeListCode(first, len);
    *packet = UNSUBACK << 4;
    return packet;
}

Code *decode_Unsuback(char *packet){
    return decodeListCode(packet);
}

char *encode_Disconnect(int *len){
    char *packet = new char[2];
    packet[0] = DISCONECT << 4;
    *len = 1;
    return packet;
}

