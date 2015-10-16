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
#include<openssl/md5.h>


struct ClientFileContent {
  char *username;
  char *password;
  char servers[10][1];
  char ports[10][10];
  char addresses[10][16];
};

struct FileDistributionCombination {
  int hash_modulo_value;
  int server1_files[2];
  int server2_files[2];
  int server3_files[2];
  int server4_files[2];
};

struct FileCombos {
  struct FileDistributionCombination x0;
  struct FileDistributionCombination x1;
  struct FileDistributionCombination x2;
  struct FileDistributionCombination x3;
};




void setup_file_matrix(struct FileCombos *matrix);
void send_to_server(char *message, int server_number, struct ClientFileContent *params);
void construct_put_message(char *filename, char *filesize, char *filecontent, struct ClientFileContent *params, char *final_message);
void deleteSubstring(char *original_string,const char *sub_string);
void parse_client_conf_file(struct ClientFileContent *params, char *file_name);
int handle_get (char *get_command);
int handle_put (char *put_command, struct ClientFileContent *params);
void handle_list ();

