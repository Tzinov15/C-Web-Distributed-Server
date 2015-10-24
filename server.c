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

  struct FilePortionLocations portion_locations;
  struct Username_Passwords user_pass;
  int i;
  for (i = 0; i < 4; i++)
  {
    strcpy(portion_locations.portion_locations[i], "EMPTY");
  }


  parse_server_conf_file(&user_pass);

  int main_socket, pid, port_number;
  port_number = atoi(argv[2]);
  struct sockaddr_in client;

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
      client_handler(client_socket, port_number,&user_pass, &portion_locations);
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

void parse_server_conf_file(struct Username_Passwords *name_password) {

  struct stat buffer;

  /* This will always be our server conf file */
  char filename[9] = "dfs.conf";
  filename[8] = '\0';

  /* file pointer for our dfs.conf file */
  FILE *config_file;
  char read_line[200];

  /* check if file exists */
  if (stat (filename, &buffer) != 0) {
    perror("Conf file doesn't exist: ");
    exit(-1);
  }

  /* check if file can be opened */
  config_file = fopen(filename, "r");
  if (config_file == NULL) {
    perror("Opening conf file did not work: ");
    exit(-1);
  }

  //printf("Server conf file succesfully opened!!\n");

  char *token;
  int counter = 0;

  /* read file and parse out usernames and passwords, popualte struct */
  while (fgets (read_line,200, config_file) != NULL) {
    /* cap the number of users we read in and thus accept */
    if (counter == MAX_USERS) {
      //printf("Max number of users parsed, ignoring the rest\n");
      return;
    }
    token = strtok(read_line, " ");
    //printf("This should be our username: %s\n", token);
    strcpy(name_password->username[counter], token);
    token = strtok(NULL, " ");

    deleteSubstring(token, "\n");
    //printf("This should be our password: %s\n", token);
    strcpy(name_password->password[counter], token);
    counter++;
  }

}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * validate_user - this function will take the username and passed into it, validate that the two match with the dfs.conf record, and return a 0 or a 1 based on the result
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
int validate_user(char *username, char *password, struct Username_Passwords *name_password) {
  //printf("||>> Hello from validate_user\n");
  //printf("    This is the username that was passed to me%s\n", username);
  //printf("    This is the password that was passed to me%s\n", password);


  int i;
  for (i = 0; i < MAX_USERS; i++)
  {
    if (strncmp(name_password->username[i], username, strlen(username)) == 0)
    {
      //printf("We have a matching username!...\n");
      if (strncmp(name_password->password[i], password, strlen(password)) == 0) {
        //printf("We have a matching password!...\n");
        return 0;
      }
    }
  }
  //printf("User and password did not match\n");
  return 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * parse_request - this function will be responsible for taking in the request from the client, parsing out the elements of the body and header, then populating the respective stirngs that were passed in
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
int parse_message_header(char *file_content, char *username, char *password, char *file_name, unsigned long *header_size, unsigned long *body_size, char *method) {
  //printf("||>> Hello from parse_request\n");
  char header_start_string[13];
  char *ptr;

  char *backup_first_line_left_over;
  char backup_first_line[256];
  memset(&backup_first_line, 0, sizeof(backup_first_line));
  char *file_size_token;
  char *token;
  char *new_token;
  char *header_left_over;
  char *second_token;
  strncpy(header_start_string, file_content, 12);
  header_start_string[12] = '\0';
  if ( (strncmp(header_start_string, "&**&STX", 7)) == 0) {
    //printf("  Header Request!!\n\n");
    *header_size = strlen(file_content);
    token = strtok_r(file_content, "\n", &header_left_over);
    strncpy(backup_first_line, token, strlen(token));
    second_token = strtok(token, " ");
    second_token = strtok(NULL, " ");
    strcpy(file_name, second_token);
    new_token = strtok(header_left_over, "\n");
    strcpy(username, new_token);
    new_token = strtok(NULL, "\n");
    strcpy(password, new_token);
    if ( (strncmp(header_start_string, "&**&STXPUT", 10)) == 0) {
      printf("  PUT request\n");
      file_size_token = strtok_r(backup_first_line, " ", &backup_first_line_left_over);
      file_size_token = strtok(backup_first_line_left_over, " ");
      file_size_token = strtok(NULL, " ");
      *body_size = strtoul(file_size_token, &ptr, 10);
      strcpy(method, "PUT");
      return 1;
    }
    if ( (strncmp(header_start_string, "&**&STXGETPN", 12)) == 0) {
      printf("  GETPN request\n");
      strcpy(method, "GETPN");
      return 1;
    }
    if ( (strncmp(header_start_string, "&**&STXGET", 10)) == 0) {
      printf("  GET request\n");
      strcpy(method, "GET");
      return 1;
    }
    if ( (strncmp(header_start_string, "&**&STXLIST", 10)) == 0) {
      printf("  LIST request\n");
      strcpy(method, "LIST");
      return 1;
    }
    else
      return 0;
  }
  else {
    printf("  Regular Body\n");
    return 0;
  }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * create_file_from_portion - this function will take in the name of the file, and its body, and create a new file under the correct folder for the respective server number and user
 *--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void create_file_from_portion(char *file_name, char *body, int port_number, char *user_name) {
  //printf("||>> Hello from create_file_from_portion\n");
  //printf("    This is the file_name that was passed to me: %s\n", file_name);
  //printf("    This is the body that was passed to me: \n%s\n", body);
  char new_file_name[strlen(file_name) + 16];
  memset(&new_file_name, 0, sizeof(new_file_name));
  char directory_name[16];
  memset(&directory_name, 0, sizeof(directory_name));
  char server_number_char[2];
  int server_number;

  // Set up the filename by adding server number, a dot, and the original filename passed in
  server_number = port_number - 10000;
  sprintf(server_number_char, "%d", server_number);
  strncpy(new_file_name, ".", 1);
  strncat(new_file_name, server_number_char, 2);
  strncat(new_file_name, ".", 1);
  strncat(new_file_name, file_name, strlen(file_name));

  // Set up the directory by adding current directory, a slash, "DFS", server number, a slash, username
  strncpy(directory_name, "DFS", 3);
  strncat(directory_name, server_number_char, 2);

  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    printf("Error with getcwd()\n");
  char full_dir_path[strlen(cwd)+strlen(directory_name) + strlen(user_name) + 16];
  memset(&full_dir_path, 0, sizeof(full_dir_path));
  strncpy(full_dir_path, cwd, strlen(cwd));
  strncat(full_dir_path, "/", 1);
  strncat(full_dir_path, directory_name, strlen(directory_name));
  strncat(full_dir_path, "/", 1);
  strncat(full_dir_path, user_name, strlen(user_name));

  // check for directory precesne
  struct stat st;

  if (stat(full_dir_path, &st) == -1) {
    mkdir(full_dir_path, 0700);
  }

  DIR* dir = opendir(full_dir_path);
  if (dir) {}
  else if (ENOENT == errno)
    printf("Users directory does not exist (shouldn't happen at this point...)\n");
  else
    printf("opendir() failed for some other reason\n");

  char full_file_path[strlen(full_dir_path) + strlen(new_file_name) + 16];
  memset(&full_file_path, 0, sizeof(full_file_path));
  strncpy(full_file_path, full_dir_path, strlen(full_dir_path));
  strncat(full_file_path, "/", 1);
  strncat(full_file_path, new_file_name, strlen(new_file_name));

  FILE *file_portion;
  file_portion=fopen(full_file_path, "a");

  fwrite(body, 1, strlen(body), file_portion);
}
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * client_handler - this is the function that gets first called by the child (client) process. It receives the initial request and proceeds onward with error handling, parsing, and file serving
 *----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void client_handler(int client, int port_number, struct Username_Passwords *name_password, struct FilePortionLocations *locations) {
  ssize_t read_size, total_bytes_read;
  total_bytes_read = 0;
  char client_message[1280], body[1024], username[64], password[64], file_name[64];
  unsigned long header_size = 0, body_size = 0,  total_size = 1;

  char incorrect_password_message[] = "Invalid Username/Password. Please try again";
  char success_message[] = "File writing successful";
  char header_ack_message[] = "Recieved your header. User/Pass authorized";
  //char get_response_message[] = "Your file has been retreived";
  char request_method[6];
  memset(&request_method, 0, sizeof(request_method));
  // this array of strings wilh hold the path locations of file poritons 1-4

  ssize_t client_ack_size;
  char client_ack_message_buffer [64];
  memset(&client_ack_message_buffer, 0, sizeof(client_ack_message_buffer));
  memset(&client_message, 0, sizeof(client_message));
  memset(&body, 0, sizeof(body));

  printf("======================\n");
  read_size = recv(client, client_message, 1280, 0);
  if ((parse_message_header(client_message, username,  password, file_name, &header_size, &body_size, (char *)&request_method)) == 1) {
    //printf("Just a header, no need to write to any file\n");
    //printf("This was the method extracted from the header: %s\n", request_method);
    //printf("This was the username extracted from the header: %s\n", username);
    //printf("This was the password extracted from the header: %s\n", password);
    //printf("This was the filename extracted from the header: %s\n", file_name);
    if ( ((validate_user(username, password, name_password))) == 0) {
      //printf("Username and password match!!\n");
      send(client, header_ack_message, sizeof(header_ack_message), 0);
    }
    else {
      printf("Incorrect username name pass\n");
      send(client, incorrect_password_message, sizeof(incorrect_password_message), 0);
    }
  }
  else {
    printf("Not the header, which is odd because it should be the header at this point \n");
  }
  if (strncmp(request_method, "GETPN", 5) == 0)
  {
    //printf("Hello from the getpn handler from the server. Waiting for ack from client so that we can start sendig...\n");

    char client_ready_for_pns_message_buffer[64];
    ssize_t client_ready_for_pns_size;
    client_ready_for_pns_size = recv(client, client_ready_for_pns_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_pns_message_buffer);
    char server_name[8];
    memset(&server_name, 0, sizeof(server_name));
    strncpy(server_name, "DFS", 3);
    int server_number;
    server_number = port_number - 10000;
    char server_number_char[2];
    sprintf(server_number_char, "%d", server_number);
    strncat(server_name, server_number_char, 2);

    char getpn_response_message[64];
    strcpy(getpn_response_message, "Portions: ");

    int *portion_numbers;
    char generic_dfs_folder_name [] = "%s/%s/";
    char dfs_path[sizeof(generic_dfs_folder_name) + 64];
    memset(&dfs_path, 0, sizeof(dfs_path));
    snprintf(dfs_path, sizeof(dfs_path), generic_dfs_folder_name, server_name, username);
    portion_numbers = find_file_portions(file_name, dfs_path, locations);
    int j = 0;
    for (j = 0; j < 2; j++)
    {
      //printf("These are the portion numbers extract from server %d: %d\n", port_number, *(portion_numbers+j));
      sprintf(server_number_char, "%d", *(portion_numbers+j));
      strncat(getpn_response_message, server_number_char, 2);
      strncat(getpn_response_message, " ", 1);

    }
    send(client, getpn_response_message, sizeof(getpn_response_message), 0);
    client_ack_size = recv(client, client_ack_message_buffer, 64, 0);
    printf("Client: %s\n", client_ack_message_buffer);
    return;
  }
  if (strncmp(request_method, "GET", 3) == 0)
  {
    printf("This is the parsed filename from the header, no idea if its going to work: %s\n", file_name);
    char client_ready_for_get_body_message_buffer[64];
    ssize_t client_ready_for_get_body_size;
    client_ready_for_get_body_size = recv(client, client_ready_for_get_body_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_get_body_message_buffer);

    // start sending data to client

    /*
    char server_name[8];
    char generic_dfs_folder_name [] = "%s/%s/";
    char dfs_path[sizeof(generic_dfs_folder_name) + 64];
    memset(&dfs_path, 0, sizeof(dfs_path));
    snprintf(dfs_path, sizeof(dfs_path), generic_dfs_folder_name, server_name, username);
    int  server_number;
    server_number = port_number - 10000;
    char server_number_char[2];
    sprintf(server_number_char, "%d", server_number);

    memset(&server_name, 0, sizeof(server_name));
    strncpy(server_name, "DFS", 3);
    strncat(server_name, server_number_char, 2);
    //printf("We have a GET request that we need to deal with!!\n");

    find_file_portions(file_name, dfs_path, locations);
    if ( (are_file_portions_ready) == 0) {
      printf("Yay! Accumulated enough file portions to reconstruct file\n");
      send_back_file(locations, client);
      return;
      }
      send(client, get_response_message, sizeof(get_response_message), 0);
      else
      printf("Not enough parts have been accumulated, need to collect more servers...\n");
      */
  }
  if (strncmp(request_method, "PUT", 3) == 0)
  {
    //printf("We have a PUT request that we need to deal with!!\n");
    total_bytes_read += read_size;
    //printf("Just read this many bytes: %zu\n", read_size);
    //printf("Total read bytes: %zu\n", total_bytes_read);
    memset(&client_message, 0, sizeof(client_message));

    while (total_bytes_read != total_size) {
      read_size = recv(client, client_message, 1280, 0);
     // printf("Just read this many bytes: %zu\n", read_size);
      total_bytes_read += read_size;
      //printf("This is what I just read from the client: \n%s\n", client_message);
      create_file_from_portion(file_name, client_message, port_number, username);

      total_size = header_size + body_size;
      memset(&client_message, 0, sizeof(client_message));
      //printf("This is the total bytes read: %zu\n", total_bytes_read);
      //printf("This is the total size of the portion %zu\n", total_size);
      send(client, success_message, sizeof(success_message), 0);
    }

  }
}

void send_back_file(locations, client) {
}

int are_file_portions_ready (struct FilePortionLocations *locations) {
  int i;
  for (i = 0; i < 4; i++)
  {
    if ( (strncmp(locations->portion_locations[i], "EMPTY", 5)) == 0 )
      return 1;
  }
  return 0;
}

int * find_file_portions(char *file_name,  char *directory_path, struct FilePortionLocations *locations)
{
  DIR *dp;
  struct dirent *ep;
  char complete_portion_file_path[strlen(directory_path) + 128];
  int file_portion_number;
  static int pns[2];
  memset(&complete_portion_file_path, 0, sizeof(complete_portion_file_path));
  int i = 0;

  dp = opendir (directory_path);
  if (dp != NULL)
  {
    while ( (ep = readdir (dp)) ) {
      if ( (strstr(ep->d_name, file_name)) != NULL)
      {
        //printf("We have a match on the files!!\n");
        //printf("This is the interger version of the file portion (extracted from the last character of the file name%d\n", atoi(&ep->d_name[strlen(ep->d_name)-1]));
        file_portion_number = atoi(&ep->d_name[strlen(ep->d_name)-1]);
        pns[i] = file_portion_number;
        i++;
        strncpy(complete_portion_file_path, directory_path, strlen(directory_path));
        strncat(complete_portion_file_path, ep->d_name, strlen(ep->d_name));
        //printf("This should be our complete file path for the portion: %s\n", complete_portion_file_path);
        strcpy(locations->portion_locations[file_portion_number-1], complete_portion_file_path);
        memset(&complete_portion_file_path, 0, sizeof(complete_portion_file_path));
      }
    } 
    (void) closedir (dp);
  }
  else
    perror ("Couldn't open the directory");
  //printf("done with parsign the directory...\n");

  return pns;
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
