#include<limits.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<sys/stat.h>
#include<dirent.h>
#define ERROR -1
#define MAX_CLIENTS 5
#define MAX_DATA 1024
#define MAX_RESPONSE 1024*1024
#define MAX_PATH_LENGTH 512

#define NUM_THREADS 4


#define FALSE 0
#define TRUE 1
#define NOT_FOUND 404
#define BAD_METHOD 4040
#define BAD_URI 4041
#define BAD_HTTP_VERSION 4042
#define NUM_OF_FILE_TYPES 8
#define MAX_USERS 5


/*--------------/
 * HEADER STUFF
 *--------------*/

/* Struct Definitions */

struct Username_Passwords {
  char username[MAX_USERS][64];
  char password[MAX_USERS][64];
};

struct TextfileData {
  int port_number;
  char document_root[MAX_PATH_LENGTH];
  char default_web_page[20];
  char extensions[NUM_OF_FILE_TYPES+1][512];
  char encodings [NUM_OF_FILE_TYPES+1][512];
};

/* Function Declarations */
void parse_server_conf_file(struct Username_Passwords *name_password);
void deleteSubstring(char *original_string,const char *sub_string);
int setup_socket (int port_number, int max_clients);
int validate_user(char *username, char *password, struct Username_Passwords *name_password);
void client_handler(int client, int port_number, struct Username_Passwords *name_password);
void create_file_from_portion(char *file_name, char *body, int port_number, char *user_name);
int parse_message_header(char *file_content, char *username, char *password, char *file_name, unsigned long *header_size, unsigned long *body_size);

