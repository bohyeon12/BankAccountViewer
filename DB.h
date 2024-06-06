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
int registuser(char* id,char*pw, char* name, char* age); //MJ T0F1
char* makequery(char* stmt, int n, ...);
void closeDB(void);
char* getpercentileof(char* id);//GP T1F0
char* viewasset(char* id);//AC T1F0
int deleteaccount(char* id, char* bank, char* acctnum);//DO T0F1
int putaccount(char* id,char* acctnum, char* bank, char* deposit);//
int login(char* id, char* pw);
void freeresult(MYSQL_RES*);