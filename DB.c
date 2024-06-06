#include "DB.h"

int connectDB() {
    db = malloc(sizeof(struct DAO));
    char* server = "localhost";
    char* user = "root";
    char* password = "1234"; /* set me first */
    char* database = "BANK";

    db->conn = mysql_init(NULL);
    printf("%u\n", db->conn);

    /* Connect to database */
    if (!mysql_real_connect(db->conn, server, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        return 1;
    }
    else return 0;
}

int registuser(char* id, char* pw, char* name, char* age) {
    char* query = makequery("INSERT INTO USER VALUES ('?','?','?',?)", 3,name, id, pw,age);
    /* send SQL query */
    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return 1;
    }
    free(query);
    return 0;
}
int login(char* id, char* pw) {
    char* query = makequery("SELECT * FROM USER WHERE ID = ? AND PW = ?", 2, id, pw);

    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return 1;
    }

    db->res = mysql_store_result(db->conn);
    free(query);
    if (mysql_fetch_row(db->res)) {
        freeresult(db->res);
        return -1;
    }
    else {
        if (db->res) {
            freeresult(db->res);
        }
        return 0;
    }    
}

int putaccount(char* id, char* acctnum, char* bank, char* deposit) {
    char* query = makequery("SELECT * FROM ACCOUNT WHERE NUM = ? AND BANK = ?",2,acctnum,bank);
    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return 1;
    }

    db->res = mysql_store_result(db->conn);
    free(query);
    if (mysql_fetch_row(db->res)) {;
        freeresult(db->res);
        return 0;
    }
    freeresult(db->res);
     
    query = makequery("INSERT INTO ACCOUNT(NUM,BANK,DEPOSIT,OWNER_ID) VALUES (?,'?',?,'?')", 4, acctnum, bank, deposit,id);
    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return 1;
    }

    if (query) free(query);
    return 0;
}

int deleteaccount(char* id, char* bank, char* acctnum) {
    char* query = makequery("DELETE FROM ACCOUNT WHERE BANK = ? AND NUM = ? AND ONWER_ID = ?", 3, bank, acctnum, id);

    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return 1;
    }

    if (query)free(query);
    return 0;
}

char* viewasset(char* id) {
    char* query = makequery("SELECT NUM, BANK, DEPOSIT FROM ACCOUNT WHERE OWNER_ID = ?", 1, id);
    
    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return NULL;
    }

    db->res = mysql_store_result(db->conn);
    free(query);
    
    int len = 0;
    int rowcnt = 0;
    while (db->row = mysql_fetch_row(db->res)) {
        len += strlen(db->row[0]) + strlen(db->row[0]) + strlen(db->row[0]) + 3;
    }
    if (len == 0) {
        return NULL;
    }

    char* result = malloc(sizeof(char)*len);
    char* cursor = result;
    mysql_data_seek(db->res, 0);
    while (db->row = mysql_fetch_row(db->res)) {
        for (int i = 0; i < 3; i++) {
            char* p = db->row[i];
            while (*p) {
                *cursor++ = *p++;
            }
            if (i == 2)*cursor++ = '\n';
            else *cursor++ = ',';
        }
    }
    *(--cursor) = '\0';
    freeresult(db->res);
    return result;
}

char* getpercentileof(char* id) {
    char* query = makequery("SELECT AGE  FROM USER WHERE ID = ? ", 1, id);

    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return -1;
    }
    if (query) free(query);
    db->res = mysql_store_result(db->conn);
    if (db->res == NULL) {
        fprintf(stderr, "Store Result Error: %s\n", mysql_error(db->conn));
        return -1;
    }

    db->row = mysql_fetch_row(db->res);
    char* agestr = db->row[0];
    freeresult(db->res);
    query = makequery("SELECT SUM(DEPOSIT) AS ASSET  FROM ACCOUNT WHERE OWNER_ID = ? ",1,id);

    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return -1;
    }
    if (query) free(query);
    db->res = mysql_store_result(db->conn);
    if (db->res == NULL) {
        fprintf(stderr, "Store Result Error: %s\n", mysql_error(db->conn));
        return -1;
    }

    db->row = mysql_fetch_row(db->res);
    char* ownerassetstr = db->row[0];
    int ownerasset = atoi(db->row[0]);
    freeresult(db->res);
    
    query = makequery("SELECT AVG(ASSET) AS AVG, STD(ASSET) AS STD FROM (SELECT SUM(DEPOSIT) AS ASSET, OWNER_ID FROM (SELECT * FROM ACCOUNT WHERE AGE >= TRUNCATE(?,-1) AND AGE < ? + (10 - MOD(?,10))) AS SAMEAGE GROUP BY OWNER_ID) AS ASSET",1,agestr,agestr,agestr);
    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        return -1;
    }

    db->res = mysql_store_result(db->conn);
    db->row = NULL;
    if (db->row = mysql_fetch_row(db->res)) {
        printf("%s | %s \n", db->row[0], db->row[1]);
        printf("%f | %f \n", atof(db->row[0]), atof(db->row[1]));
    }
    
    float x = ((float)ownerasset - atof(db->row[0])) / atof(db->row[1]);
    float p = gsl_cdf_gaussian_P(x, 1.0); 
    int len = 6 + strlen(ownerassetstr);
    char* result = malloc(sizeof(char)*len);
    sprintf_s(result, len, "%s,%.2f", ownerassetstr,p);
    freeresult(db->res);
    return result;
}

char* makequery(char* stmt, int n, ...) {
    printf("%s\n", stmt);
    va_list ap;
    va_start(ap, stmt);
    char **strarr = malloc(sizeof(char*) * (n+1));
    strarr[0] = stmt;
    int len = strlen(stmt) + 1 - n;
    for (int i = 1; i <= n; i++) {
        strarr[i] = va_arg(ap, char*);
        len += strlen(strarr[i]); 
    }
    va_end(ap);
    char* query = malloc(sizeof(char) * len);
    char*  cursor1 = query;
    char* cursor2 = strarr[0];
    int i = 1;
    while (*cursor2) {
        if (*cursor2 == '?') {
            char* cursor3 = strarr[i++];
            while(*cursor3) {
                *cursor1++ = *cursor3++;
            }
            cursor2++;
        }
        else {
            *cursor1++ = *cursor2++;
        }
    } 
    *cursor1 = '\0';
    
    if(strarr)free(strarr); 
    return query;
}

void closeDB() {
    /* close connection */
    if(db->conn)mysql_close(db->conn);
    if(db)free(db);
}

void freeresult(MYSQL_RES* res) {
    if (res) {
        mysql_free_result(res);
        res = NULL;
    }
}
/*
void updateage(int i) {
    char id[5] = { 0, };
    sprintf_s(id, 5, "%d", i);
    char* query = makequery("UPDATE USER SET AGE = FLOOR(RAND()*(75)+15) WHERE ID = ?", 1, id);

    if (mysql_query(db->conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(db->conn));
        if (query) free(query);
        return -1;
    }
    if (query)free(query);
}*/