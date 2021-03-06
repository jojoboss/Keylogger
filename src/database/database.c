#include "database.h"

char *itoa(int nb) {
  char *res= malloc(sizeof(char)*4); 
  sprintf(res, "%d", nb);
  return res;
}

static int callback(void *data, int argc, char **argv, char **azColName){
  int i;
  (void) data;
  FILE *file= fopen("../../files/client_files/client_infos.txt", "a");
  for(i=0; i < argc; i++) {
    //(char*)data[i]= argv[i];
    printf("%d i\n", i);
    fprintf(file, "%s= %s\n", azColName[i], argv[i] ? argv[i]: "NULL");
  }
  fclose(file);
  return 0;
}

void create_clients(sqlite3 *db, char mac_addr[25], char *name) {
  char *sql= "INSERT INTO clients(id, mac_addr, name) "\
	"VALUES(NULL, '";
  char *res;
  if ((res = calloc(1024, sizeof(char))) == NULL)
    return;
  strcat(res, sql);
  strcat(res, mac_addr);
  strcat(res, "', '");
  strcat(res, name);
  strcat(res, "');");
  //printf("%s\n", res);

  if(sqlite3_exec(db, res, NULL, 0, 0) != SQLITE_OK)
    perror("SQL error create client\n");
  else
    printf("create client success\n");
  free(res);
}

void research_clients(sqlite3 *db, int id_client) { 
  char *sql= "SELECT * FROM clients "\
	      "WHERE id= "; 
  char *res= calloc(256, sizeof(char)); 
   strcat(res, sql); 
   strcat(res, itoa(id_client)); 
   strcat(res, ";"); 
   printf("%s\n", res);
   if(sqlite3_exec(db, res, callback, 0, 0) != SQLITE_OK) {
     perror("sql error research client\n"); 
     //return 1;
   }
   else {
     printf("research client %d successfully\n", id_client);
     //return 0;
   }
   free(res);
 }

void get_all_clients(sqlite3 *db) {
  char *sql= "SELECT * FROM clients;";
  //printf("%s\n", sql);

  if(sqlite3_exec(db, sql, callback, 0, 0) != SQLITE_OK)
    perror("SQL error\n");
  else
    printf("get all clients success\n");
}

void update_data(sqlite3 *db, char *data, int id_client) {
  char *sql= "UPDATE data SET data= '";
  char *res= calloc(1024, sizeof(char));
  strcat(res, sql);
  strcat(res, data);
  strcat(res, "' WHERE id_client= ");
  strcat(res, itoa(id_client));
  strcat(res, ";");
  //printf("%s\n", res);

  if(sqlite3_exec(db, res, NULL, 0, 0) != SQLITE_OK)
    perror("SQL error\n");
  else {
    printf("update data success\n");
  }
  free(res);
}

void create_data(sqlite3 *db, int id_server, int id_client, char *data) {
  char *sql= "INSERT INTO data(id_server, id_client, data) "\
	      "VALUES(";
  char *res= calloc(1024, sizeof(char));
  strcat(res, sql);
  strcat(res, itoa(id_server));
  strcat(res, ", ");
  strcat(res, itoa(id_client));
  strcat(res, ", '");
  strcat(res, data);
  strcat(res, "');");
  //printf("%s\n", res);

  if(sqlite3_exec(db, res, NULL, 0, 0) != SQLITE_OK)
    perror("sqlite error create data\n");

  else
    printf("create data success\n");
  free(res);
}

int research_data(sqlite3 *db, int id_client) {
  char *sql= "SELECT * FROM data "\
	     "WHERE id_client=";
  char *res= calloc(256, sizeof(char));
  strcat(res, sql);
  strcat(res, itoa(id_client));
  strcat(res, ";");
  //printf("%s\n", res);

  if(sqlite3_exec(db, res, callback, 0, 0) != SQLITE_OK) {
    perror("sqlite error research data\n");
    return 1;
  }
  else 
    return 0;
}

void delete_clients(sqlite3 *db, int id_client) { 
   char *sql= "DELETE FROM clients "\
	       "WHERE id="; 
   char *res= calloc(256, sizeof(char)); 
   strcat(res, sql); 
   strcat(res, itoa(id_client)); 
   strcat(res, ";"); 
   //printf("%s\n", res); 

   if(sqlite3_exec(db, res, NULL, 0, 0) != SQLITE_OK) 
     perror("SQL error\n"); 
   else
     printf("delete client %d successfully\n", id_client);
   free(res);
 } 
