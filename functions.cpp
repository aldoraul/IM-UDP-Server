/* Functions.cpp
   contain function calls used
   in main.cpp
*/

#include<stdio.h>
#include"functions.h"
#include<stdlib.h>

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// input buf is message from client 
// returns the same message  decrypted
std::string decryptMessage(std::string buf){	
	int check_msg = 0;
	int length = buf.length();
	int next = 0;
	for (int i = 0; i < length; i++) {
		if (buf[i] == ';')
			check_msg += 1;
		if (check_msg == 2) {
			next = i + 1;
			break;
		}
	}
	for (int j = next; j < length; j++)
		buf[j] = decrypt(buf[j]);
	return buf;
}

// input buf is message to be sent to client 
// returns encrypted msg
std::string encryptMessage(std::string buf){
	int length = buf.length();
	for(int i = 0; i < length; i++){
		buf[i] = encrypt(buf[i]);
		}
	return buf;
}

// input message is original msg from client
// returns user name of client
std::string get_user(std::string message){
	int check_msg = 0;
	int length = message.length();
	int next = 0;
	std::string name = "";
	for(int i = 0; i < length; i++) {
		if(message[i] == ';')
			check_msg +=1;
		if(check_msg == 2) {
			next = i + 1;
			break;
		}
	}
	while((message[next] != '\n') && (next < length)){
		name += message[next];
		next++;
		}


	return name;
}

// input message is original message from client
// returns type of message [1,2,3]
int getMsgType(std::string message){
	int length = message.length();
	char num;
	for(int i = 0; i< length; i++) {
		if(message[i] == ';'){
			num = message[i+1];
			break;
			
			}
		}
	return num - '0';
}

// returns msg number created by client
int getMsgNum(std::string message){
	std::string num = "";
	int i = 0;	
	while(message[i] != ';'){
		num += message[i];
		i++;
		}
	
	return atoi(num.c_str());
}

// if msg type = 2, returns user name to foward message
std::string getBuddyName(std::string message){
	std::string buddy = "";
	int check = 0;
	for(int i = 0; i < message.length();i++){
		if(message[i] == '\n'){
			check = i+1;
			break;
			}
		}
	while(message[check] != '\n'){
		buddy += message[check];
		check++; 
		}
	return buddy;
}

// if msg type = 2, returns body of msg to be fowarded
std::string getMessage(std::string message){
	std::string msg = "";
	int check = 0;
	int i = 0;
	do{
		if(message[i] == '\n')
			check++;
		i++;
		}while(check != 2);
	for(int j = i; j < message.length();j++){
		msg += message[j];
		}
	return msg;
}
