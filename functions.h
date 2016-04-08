/* Functions header file
   includes functions for main.cpp
*/

/*********************************************
   include guard
*********************************************/
#ifndef _FUNCTIONS_H_INCLUDED_
#define _FUNCTIONS_H_INCLUDED_

#include<netinet/in.h>
#include<string>
#include"cipher.h"

void *get_in_addr(struct sockaddr *sa);

std::string decryptMessage(std::string);

std::string encryptMessage(std::string);

std::string get_user(std::string);

int getMsgType(std::string);

int getMsgNum(std::string);

#endif
