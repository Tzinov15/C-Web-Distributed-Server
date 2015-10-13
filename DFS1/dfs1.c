#include "server.h"

int main(int argc, char ** argv)
{
  printf("| Welcome to DFS1! |\n\n");

  int main_socket, cli, pid, port_number;
  struct TextfileData system_config_data;
  struct sockaddr_in client;
  unsigned int sockaddr_len = sizeof(struct sockaddr_in);

  /* Read in conf file and populate struct */
  setup_server(&system_config_data); 
  port_number = 10001;

  printf("---------------------------------------\n");
  printf("|CURRENT SERVER SETUP: \n");
  printf("|Port Number: %d\n", port_number);
  printf("---------------------------------------\n\n");

  /* Create main socket, bind, have it listen */
  main_socket = setup_socket(port_number, MAX_CLIENTS);
  int client_socket, read_size, c;
  char client_message[2000];
  client_socket = accept(main_socket, (struct sockaddr *)&client, (socklen_t*)&c);

  while( (read_size = recv(client_socket, client_message, 2000, 0)) > 0)
  {
    printf("%s\n", client_message);
  }
}
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * deleteSubstring - this function is a helper function that is used when extracting the path that the client sends a GET request on
 *------------------------------------------------------------------------------------------------------------------------------------------- */
void deleteSubstring(char *original_string,const char *sub_string) {
  while( (original_string=strstr(original_string,sub_string)) )
    memmove(original_string,original_string+strlen(sub_string),1+strlen(original_string+strlen(sub_string)));
}
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * client_handler - this is the function that gets first called by the child (client) process. It receives the initial request and proceeds onward with error handling, parsing, and file serving
 *----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void client_handler(int client, struct TextfileData *config_data) {
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * send_response - this function will be the function that actually sends a message to the client. This message will contain the proper headers and the respective body content
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void send_response(int client, int status_code, struct HTTP_RequestParams *params, char *full_path) {
}
/* TODO -> refactor so that there isn't so much hardcoding / repitition */
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * construct_file_response - this function is responsible for reading, constructing, and sending files that the user requested. This function is only invoked on a succesful validation of the file path 
 *----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void construct_file_response(char *full_path, int client) {
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * handle_file_serving - this function will take in a file path, and either construct the correct response body to serve up that file or it will return false if the file does not exist
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
int handle_file_serving(char *path, char *body, struct TextfileData *config_data, int *result_status) {
  return 0;
}
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * validate_request_headers - this function will be take the users path and decide what to do based on the result
 *------------------------------------------------------------------------------------------------------------------------------------------- */
int validate_request_headers(struct HTTP_RequestParams *params, int *decision) {
  return 0;
}
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * extract_request_parameters - this function will be mainly responsible for parsing and extracting the path from the HTTP request from the client
 *------------------------------------------------------------------------------------------------------------------------------------------- */
void extract_request_parameters(char *response, struct HTTP_RequestParams *params) {
}

/*----------------------------------------------------------------------------------------------
 * setup_server - first function called on entry, prints information and reads in config file 
 *---------------------------------------------------------------------------------------------- */
void setup_server(struct TextfileData *config_data) {
}

/*----------------------------------------------------------------------------------------------
 * setup_socket - allocate and bind a server socket using TCP, then have it listen on the port
 *---------------------------------------------------------------------------------------------- */
int setup_socket(int port_number, int max_clients)
{
  /* The data structure used to hold the address/port information of the server-side socket */
  struct sockaddr_in server;

  /* This will be the socket descriptor that will be returned from the socket() call */
  int sock;

  /* Socket family is INET (used for Internet sockets) */
  server.sin_family = AF_INET;
  /* Apply the htons command to convert byte ordering of port number into Network Byte Ordering (Big Endian) */
  server.sin_port = htons(port_number);
  /* Allow any IP address within the local configuration of the server to have access */
  server.sin_addr.s_addr = INADDR_ANY;
  /* Zero off remaining sockaddr_in structure so that it is the right size */
  memset(server.sin_zero, '\0', sizeof(server.sin_zero));

  /* Allocate the socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
    perror("server socket: ");
    exit(-1);
  }


  /* Bind it to the right port and IP using the sockaddr_in structuer pointed to by &server */
  if ((bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == ERROR) {
    perror("bind : ");
    exit(-1);
  }

  /* Have the socket listen to a max number of max_clients connections on the given port */
  if ((listen(sock, max_clients)) == ERROR) {
    perror("Listen");
    exit(-1);
  }
  return sock;
}
