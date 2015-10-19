#include "server.h"

int main(int argc, char ** argv)
{
  printf("| Welcome to the Distributed Server! |\n");

  if (argc < 2) {
    printf("Please specify a directory and port number file\n");
    exit(1); }
  if (argc < 3) {
    printf("Please specify a port number \n");
    exit(1); }
  int main_socket, cli, pid, port_number;
  port_number = atoi(argv[2]);
  struct sockaddr_in client;
  unsigned int sockaddr_len = sizeof(struct sockaddr_in);

  /* Read in conf file and populate struct */

  printf("|Port Number: %d\n", port_number);

  /* Create main socket, bind, have it listen */
  main_socket = setup_socket(port_number, MAX_CLIENTS);
  int client_socket, c;
  while (1) {

    if ( (client_socket = accept(main_socket, (struct sockaddr *)&client, (socklen_t*)&c)) < 0) {
      perror("ERROR on accept");
      exit(1);
    }

    pid = fork();
    if (pid < 0){
      perror("ERROR on fork");
      exit(1);
    }

    /* The child process will handle individual clients, therefore we can 
     * close the master socket for the childs (clients) address space */
    if (pid == 0) {
      close(main_socket);
      client_handler(client_socket);
      //close(cli);
      exit(0);
    }

    /* The parent will simply sit in this while look acceptting new clients,
     * it has no need to maintain active sockets with all clients */
    if (pid > 0) {
      close(client_socket);
      waitpid(0, NULL, WNOHANG);
    }
  }
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * validate_user - this function will take the username and passed into it, validate that the two match with the dfs.conf record, and return a 0 or a 1 based on the result
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
int validate_user(char *username, char *password) {
  printf("||>> Hello from validate_user\n");
  printf("    This is the username that was passed to me%s\n", username);
  printf("    This is the password that was passed to me%s\n", password);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * parse_request - this function will be responsible for taking in the request from the client, parsing out the elements of the body and header, then populating the respective stirngs that were passed in
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
int parse_message_header(char *file_content, char *username, char *password, char *file_name, unsigned long *header_size, unsigned long *body_size) {
  printf("||>> Hello from parse_request\n");
  printf("    This is the full file_content that was passed to me: \n%s\n", file_content);
  char header_start_string[8];
  char *ptr;
  strncpy(header_start_string, file_content, 7);
  header_start_string[7] = '\0';
  printf("  This is the start string for the file header: %s\n", header_start_string);
  if ( (strncmp(header_start_string, "&**&STX", 7)) == 0) {
    char *token;
    char *second_token;
    printf("  We have a header request!!\n");
    printf("  This is the header: \n%s\n", file_content);
    printf("  And this is its length: %zu\n", strlen(file_content));
    *header_size = strlen(file_content);
    token = strtok(file_content, "\n");
    printf("  This should be the first line of our header: %s\n", token);
    second_token = strtok(token, " ");
    second_token = strtok(NULL, " ");
    second_token = strtok(NULL, " ");
    printf("  This should be the size of the body!!! %s\n", second_token);
    *body_size = strtoul(second_token, &ptr, 10);
    return 0;

  }
  else {
    printf("  looks like a regular body\n");
    return 1;
  }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * create_file_from_portion - this function will take in the name of the file, and its body, and create a new file under the correct folder for the respective server number and user
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void create_file_from_portion(char *file_name, char *body) {
  printf("||>> Hello from create_file_from_portion\n");
  printf("    This is the file_name that was passed to me%s\n", file_name);
  printf("    This is the body that was passed to me%s\n", body);
}
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * client_handler - this is the function that gets first called by the child (client) process. It receives the initial request and proceeds onward with error handling, parsing, and file serving
 *----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void client_handler(int client) {
  ssize_t read_size;
  ssize_t total_bytes_read;
  total_bytes_read = 0;
  char client_message[1024];
  char body[1024];
  char username[64];
  char password[64];
  char file_name[64];
  unsigned long header_size = 0;
  unsigned long body_size = 0;
  unsigned long  total_size = 1;
  memset(&client_message, 0, sizeof(client_message));

  while (total_bytes_read != total_size)
  {
    sleep(1);
    read_size = recv(client, client_message, 1024, 0);
    total_bytes_read += read_size;
    // if our message received on the socket is just the header...no need to write anyting to a file 
    if ((parse_message_header(client_message, username,  password, file_name, &header_size, &body_size)) == 0) {
      printf("Just a header, no need to write to any file\n");
    }
    // the message received on the socket contains no header
    else
    printf("Just read this many bytes: %zu\n", read_size);
    printf("Total read bytes: %zu\n", total_bytes_read);
    total_size = header_size + body_size;
    printf("And this is the total of the body and the header %lu\n", total_size);

    /*
    printf("This is the username from the header: %s\n", username);
    printf("This is the password from the header: %s\n", password);
    printf("This is the body size from the header: %zd\n", body_size);
    */
    /*
    if (validate_user(username, password))
      printf("Username and password do not match!!\n");
    else
      create_file_from_portion (file_name, body);
    sleep(1);
    */
    memset(&client_message, 0, sizeof(client_message));
  }
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
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * deleteSubstring - this function is a helper function that is used when extracting the path that the client sends a GET request on
 *------------------------------------------------------------------------------------------------------------------------------------------- */
void deleteSubstring(char *original_string,const char *sub_string) {
  while( (original_string=strstr(original_string,sub_string)) )
    memmove(original_string,original_string+strlen(sub_string),1+strlen(original_string+strlen(sub_string)));
}
