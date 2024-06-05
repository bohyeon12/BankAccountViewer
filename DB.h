#pragma once
#include <string.h>
#include <stdarg.h> 
#include <stdio.h>
#include <mysql.h>
#include<gsl/gsl_cdf.h>
#include <math.h>

struct DAO
{
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;
};
static struct DAO* db;
int connectDB(void);
int registuser(char*, char*, char*);
char* makequery(char*, int, ...);
void closeDB(void);
float getpercentileof(char*);
char* viewasset(char*);
int deleteaccount(char*, char*, char*);
int putaccount(char* , char* , char* , char*);
int login(char*, char*);
void freeresult(MYSQL_RES*);