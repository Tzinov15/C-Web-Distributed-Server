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
    printf("  Header Request!!");
    *header_size = strlen(file_content);
    token = strtok_r(file_content, "\n", &header_left_over);
    if ( (strncmp(header_start_string, "&**&STXLIST", 11)) == 0) {
      printf("  LIST request\n");
      new_token = strtok(header_left_over, "\n");
      printf("poop\n");
      strcpy(username, new_token);
      printf("poop\n");
      new_token = strtok(NULL, "\n");
      printf("poop\n");
      strcpy(password, new_token);
      printf("poop\n");
      strcpy(method, "LIST");
      printf("Does with list header parsing\n");
      return 1;
    }
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
  // client messge is used to store the full original message from client, body is used to store just body. username, password, and file_name all store the respective information which is parsed from the header 
  char client_message[1280], body[1024], username[64], password[64], file_name[64];
  memset(&client_message, 0, sizeof(client_message));
  memset(&body, 0, sizeof(body));

  unsigned long header_size = 0, body_size = 0,  total_size = 1;

  // these are strings that we'll send in ACK messages to the client following certain messages received from the client
  char incorrect_password_message[] = "Invalid Username/Password. Please try again";
  char success_message[] = "File writing successful";
  char header_ack_message[] = "Recieved your header. User/Pass authorized";
  char request_method[6];
  memset(&request_method, 0, sizeof(request_method));

  // buffer and return ssize_t variables used to store acknowledgement from client
  ssize_t client_ack_size;
  char client_ack_message_buffer [64];
  memset(&client_ack_message_buffer, 0, sizeof(client_ack_message_buffer));

  printf("======================\n");
  read_size = recv(client, client_message, 1280, 0);
  // if this returns 1 (which it should), then the message is a header and we need to validate the user/pass combo and send a cofirmation or deny ACK message to the client
  if ((parse_message_header(client_message, username,  password, file_name, &header_size, &body_size, (char *)&request_method)) == 1) {
    if ( ((validate_user(username, password, name_password))) == 0) {
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
 
  if (strncmp(request_method, "LIST", 4) ==0)
  {
    printf("We have a list method to deal with\n");

    char client_ready_for_list_files_message_buffer[64];
    memset(&client_ready_for_list_files_message_buffer, sizeof(client_ready_for_list_files_message_buffer), 0);
    ssize_t client_ready_for_list_files_size;

    client_ready_for_list_files_size = recv(client, client_ready_for_list_files_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_list_files_message_buffer);

    char list_response_body[1024];
    memset(&list_response_body, sizeof(list_response_body), 0);
    construct_file_list_body(username, list_response_body, port_number);
    printf("This is the list response body that we have made so far: \n%s\n", list_response_body);
    send(client, list_response_body, strlen(list_response_body), 0);

  }
  // if the user has a sent a request to get the server portion numbers of a particular file
  if (strncmp(request_method, "GETPN", 5) == 0)
  {
    // buffer and return value ssize_t variables used to store thumbs up message from client telling the server to send the portion number message
    char client_ready_for_pns_message_buffer[64];
    memset(&client_ready_for_pns_message_buffer, sizeof(client_ready_for_pns_message_buffer), 0);
    ssize_t client_ready_for_pns_size;

    // block and wait for the client to receive the password/user confirming ack and send back a thumbs up saying they're ready for the PN message
    client_ready_for_pns_size = recv(client, client_ready_for_pns_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_pns_message_buffer);

    // create our server directory name by connecting "DFS" and our server numebr
    char server_name[8];
    memset(&server_name, 0, sizeof(server_name));
    strncpy(server_name, "DFS", 3);

    // convert the port number of the current runnign server instance to the respective server number, copy it into a string, append it to our server_name
    int server_number;
    server_number = port_number - 10000;
    char server_number_char[2];
    sprintf(server_number_char, "%d", server_number);
    strncat(server_name, server_number_char, 2);

    // getpn_response_message is the string that we are sending to the client as a response to their GETPN request
    char getpn_response_message[64];
    strcpy(getpn_response_message, "Portions: ");

    // this will be the array of ints that will store the two portion numbers that we extract from each of the server folders
    int *portion_numbers;

    // the following constructs the directory path that we search in for the file portion that the user specified
    // the directory name is made up of the folder that the current port is operating under ("DFS" + port_number-10000) + the name of the current user (sent in header from client)
    char generic_dfs_folder_name [] = "%s/%s/";
    char dfs_path[sizeof(generic_dfs_folder_name) + 64];
    memset(&dfs_path, 0, sizeof(dfs_path));
    snprintf(dfs_path, sizeof(dfs_path), generic_dfs_folder_name, server_name, username);

    // search the folder (specified by the above constructed file path), return the portion numebrs of the files parts found, append these poriton numbers to the message that we're sending to the client
    portion_numbers = find_file_portions(file_name, dfs_path, locations);
    int j = 0;
    for (j = 0; j < 2; j++)
    {
      sprintf(server_number_char, "%d", *(portion_numbers+j));
      strncat(getpn_response_message, server_number_char, 2);
      strncat(getpn_response_message, " ", 1);

    }
    // send our message body to the client
    send(client, getpn_response_message, sizeof(getpn_response_message), 0);

    // and wait for client to ackonowledge that they have received the message
    client_ack_size = recv(client, client_ack_message_buffer, 64, 0);
    printf("Client: %s\n", client_ack_message_buffer);
    return;
  }
  if (strncmp(request_method, "GET", 3) == 0)
  {
    // buffer and ssize_t return value variables used to store the thumbs up ack message from the client saying they're ready to receive the get header from the server
    char client_ready_for_get_header_message_buffer[64];
    memset(&client_ready_for_get_header_message_buffer, 0, sizeof(client_ready_for_get_header_message_buffer));
    ssize_t client_ready_for_get_header_size;

    // this is string that will be sent to the client as a preleminary header to the the actual body being sent from the server
    char get_response_header[32];
    memset(&get_response_header, 0, sizeof(get_response_header));
    construct_get_response_header(file_name, get_response_header);

    // buffer and ssize_t return value variables used to store the thumbs up ack message from the client saying they're ready to receive the actual portion body from the server
    char client_ready_for_get_body_message_buffer[64];
    memset(&client_ready_for_get_body_message_buffer, 0, sizeof(client_ready_for_get_body_message_buffer));
    ssize_t client_ready_for_get_body_size;

    // block and wait for the client to acknowledge the password confirmation packet from the server and give the thumbs up that they're ready to receive the header
    client_ready_for_get_header_size = recv(client, client_ready_for_get_header_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_get_header_message_buffer);

    // send the client the header for the get reeponse
    send(client, get_response_header, sizeof(get_response_header), 0);

    // block and wait for the client to acknowledge the header message from the server and give the thumbs up that they're ready to receive the actual body of the portion
    client_ready_for_get_body_size = recv(client, client_ready_for_get_body_message_buffer, 64, 0);
    printf("Client: %s\n", client_ready_for_get_body_message_buffer);

    // buffer and ssize_t return value variables used to store the ack message from the client saying that they have received the servers file portion and written it to their file
    char client_body_portion_ack_buffer[64];
    memset(&client_body_portion_ack_buffer, 0, sizeof(client_body_portion_ack_buffer));
    ssize_t client_body_portion_ack_size;

    // this is the buffer into which we read the file portion, and then from the buffer write it to the client socket
    // this buffer gets emptied at the end of each read in case we have to read a bigger portion and the buffer needs to be reused
    char file_portion_reading_buffer[1024];
    memset(&file_portion_reading_buffer, 0, sizeof(file_portion_reading_buffer));

    // open the file portion that we will be sending to the client
    // the name of this file portion was extracted when parsing the GET request that the user sent
    FILE *portion_file;
    portion_file = fopen(file_name, "r");
    if (portion_file == NULL) {
      printf("Oops, error opening the file\n");
      exit(1);
    }

    // variable that will keep track of how many bytes we read on each itertion
    ssize_t read_bytes;
    while (!feof(portion_file)) {
      // read from the portion file into the buffer
      read_bytes = fread(file_portion_reading_buffer, 1, 1024, portion_file);
      // send to the client the portion
      send(client, file_portion_reading_buffer, read_bytes, 0);
      // wait for a succesful acknowledgement from the client
      client_body_portion_ack_size = recv(client, client_body_portion_ack_buffer, 64, 0);
      printf("Client: %s\n", client_body_portion_ack_buffer);
      // clear the buffer for potential reuses
      memset(&file_portion_reading_buffer, 0, sizeof(file_portion_reading_buffer));
    }
    // close our file
    fclose(portion_file);
    printf("Server is done writting the file portion to the client\n");

  }
  if (strncmp(request_method, "PUT", 3) == 0)
  {
    // append to total bytes read the bytes that we already read in from the header parsing
    total_bytes_read += read_size;
    memset(&client_message, 0, sizeof(client_message));

    while (total_bytes_read != total_size) {
      // read in content from the client
      read_size = recv(client, client_message, 1280, 0);
      total_bytes_read += read_size;
      // create the file portion (or append to it) from the data that the client sent
      create_file_from_portion(file_name, client_message, port_number, username);
      total_size = header_size + body_size;
      // clear buffer
      memset(&client_message, 0, sizeof(client_message));
      // acknowledge the client that we have received the written the data
      send(client, success_message, sizeof(success_message), 0);
    }
  }
}

void construct_file_list_body(char *user_name, char *list_response_body, int port_number) {


  char server_number_char[2];
  int server_number;

  // Set up the filename by adding server number, a dot, and the original filename passed in
  server_number = port_number - 10000;

    
  strcpy(list_response_body, "File List: \n");

  char generic_dfs_folder_name [] = "DFS%d/%s/";
  char dfs_path[sizeof(generic_dfs_folder_name) + 64];
  memset(&dfs_path, 0, sizeof(dfs_path));
  snprintf(dfs_path, sizeof(dfs_path), generic_dfs_folder_name, server_number, user_name);
  printf("Inside of construct file_list_body on the server, this is the file name for the current dir: %s\n", dfs_path);

  DIR *dp;
  struct dirent *ep;
  char file_name[256];
  memset(&file_name, 0, sizeof(file_name));

  char truncated_file_name[256];
  memset(&truncated_file_name, 0, sizeof(truncated_file_name));

  int counter = 0;
  int current_file_count = 0;
  int file_already_exists_in_list = 0;


  char unique_file_list[10][128];

  dp = opendir (dfs_path);
  int i;
  if (dp != NULL)
  {
    while ( (ep = readdir (dp)) ) {
      if (counter < 2) {
        counter ++;
        continue;
      }
      strcpy(file_name, ep->d_name);
      strcpy(truncated_file_name, file_name+3);
      truncated_file_name[strlen(truncated_file_name)-1] = 0;
      truncated_file_name[strlen(truncated_file_name)-1] = 0;
      printf("File name: %s\n", truncated_file_name);

      for (i = 0; i < 10; i++) {
        file_already_exists_in_list = 0;
        if ( (strncmp(unique_file_list[i], truncated_file_name, strlen(truncated_file_name))) == 0) {
          file_already_exists_in_list = 1;
          break;
        }
        else {
        }
      }
      if (file_already_exists_in_list == 0) {
        strcpy(unique_file_list[current_file_count], truncated_file_name);
        current_file_count++;
        strcat(list_response_body, truncated_file_name);
        strcat(list_response_body, "\n");
      }
    }
    (void) closedir (dp);
  }
  else
    printf("Couldn't open the directory");
}

void construct_get_response_header(char *file_name, char *get_response_header) {
  struct stat buffer;

  if ( (stat(file_name, &buffer)) ==  0) {
    // convert the portion numeber into a char
    char body_size_char[65];
    memset(&body_size_char, 0, sizeof(body_size_char));
    sprintf(body_size_char, "%lld", buffer.st_size);
    strcpy(get_response_header, "BodySize: ");
    strcat(get_response_header, body_size_char);
  }
  else
    printf("uh oh, file does not exist\n");
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
