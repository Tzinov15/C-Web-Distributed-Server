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
  char ports[10][10];
  char addresses[10][16];
};

struct FileDistributionCombination {
  int hash_combo_zero_servers[8];
  int hash_combo_one_servers[8];
  int hash_combo_two_servers[8];
  int hash_combo_three_servers[8];
};

static struct FileDistributionCombination destination_matrix = {
  {4,1,1,2,2,3,3,4},
  {1,2,2,3,3,4,4,1},
  {2,3,3,4,4,1,1,1},
  {3,4,4,1,1,2,2,3}};


struct FilePortionLocations {
  int portion_locations[4][2];
};



// void setup_file_matrix(struct FileCombos *matrix);
int check_locations_array(struct FilePortionLocations *locations);
void update_locations_array(char *server_message, struct FilePortionLocations *locations, int port_number);
void send_file (int first_server_number, int second_server_number, int portion_number, ssize_t portion_size, FILE *user_file, struct ClientFileContent *params, char *portion_file_name);
int create_socket_to_server(int server_number, struct ClientFileContent *params);
void send_to_server(char *message, int server_number, struct ClientFileContent *params);

void construct_put_header(char *filename, char *filesize, struct ClientFileContent *params, char *header);
void construct_getpn_header(char *filename, struct ClientFileContent *params, char *header);
void construct_get_header(char *filename, struct ClientFileContent *params, char *header);
void construct_list_header(struct ClientFileContent *params, char *header);

void deleteSubstring(char *original_string,const char *sub_string);
void parse_client_conf_file(struct ClientFileContent *params, char *file_name);
int handle_get (char *get_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix, struct FilePortionLocations *locations);
int handle_put (char *put_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix);
int handle_list (char *list_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix);

