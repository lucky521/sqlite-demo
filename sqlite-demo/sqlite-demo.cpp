#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include "sqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

std::string generate_sig(int len)
{
	std::string sig_str;

	srand((unsigned)time(NULL));
	
	double pRandomValue;

	for(int i=0;i<len;i++)
	{
		pRandomValue = (double)(rand() / (double)RAND_MAX);
		pRandomValue = pRandomValue*2 - 1; // -1, 1
		
		std::ostringstream ss;
		ss << pRandomValue;
		ss << "\n";
		std::string s(ss.str());
		sig_str += s;
	}
	return sig_str;
}

int do_transaction(sqlite3* db, int(*fun)(sqlite3*))
{
	char *zErrMsg = 0;

	sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	
	fun(db);

	sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
	return 1;
}

int query_data(sqlite3* db)
{
	char *zErrMsg = 0;
   // Query data from Table
   char *sql = "SELECT * from watermark";

   int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Operation done successfully\n");
   }
	return 1;
}

int open_db(char* filename, sqlite3* &db)
{
   /* Open database */
   int rc = sqlite3_open(filename, &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return -1;
   } else {
      fprintf(stdout, "Opened database successfully\n");
	  return 1;
   }
}

int create_table(sqlite3* db)
{
	char *zErrMsg = 0;
   /* Create Table */
   char *sql = "CREATE TABLE watermark("  \
         "ID INTEGER PRIMARY KEY    AUTOINCREMENT," \
         "DOMAINNAME           TEXT    NOT NULL," \
         "USERNAME            TEXT     NOT NULL," \
         "WATERMARK        TEXT NOT NULL);";

   int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
	  return -1;
   } else {
      fprintf(stdout, "Table created successfully\n");
	  return 1;
   }

}

int add_item(sqlite3* db)
{
	char *zErrMsg = 0;
	int rc;

   // Insert new item to Table
   for(int i=0;i<10;i++)
   {
	   std::string new_sig = generate_sig(1000);
	   //printf("%s \n", new_sig.c_str());
	   char buf[10000];
	   sprintf(buf, "INSERT INTO watermark (DOMAINNAME,USERNAME,WATERMARK)  \
			 VALUES ('liuluview', 'luliu%d', '%s'); ",
			 i, new_sig.c_str()
			 );

	   printf("%s \n", buf);

	   rc = sqlite3_exec(db, buf, callback, 0, &zErrMsg);
	   
	   if( rc != SQLITE_OK ){
		  fprintf(stderr, "SQL error: %s\n", zErrMsg);
		  sqlite3_free(zErrMsg);
	   } else {
		  fprintf(stdout, "Records created successfully\n");
	   }
   }
   return 1;
}



int main(int argc, char* argv[]) 
{
   sqlite3 *db;

   open_db("watermark.db", db);

   //create_table(db);
   //add_item(db);
   //query_data(db);

   do_transaction(db, create_table);

   do_transaction(db, add_item);

   do_transaction(db, query_data);

   sqlite3_close(db);
   return 0;
}

