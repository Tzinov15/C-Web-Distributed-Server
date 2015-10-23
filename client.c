#include "client.h"
int main(int argc, char ** argv) {

  struct ClientFileContent client_params;
  struct FilePortionLocations locations;
  int i;
  for (i = 0; i < 4; i++)
  {
    locations.portion_locations[i][0] = 111;
    locations.portion_locations[i][1] = 111;
  }

  if (argc < 2) {
    printf("Please specify a dfc.conf file\n");
    exit(1);
  }

  parse_client_conf_file(&client_params, argv[1]);

  printf("*-----------------------------------------------*\n");
  printf("|  To see of a list of your files, enter LIST   |\n");
  printf("|  To download a file, enter GET <name_of_file> |\n");
  printf("|   To upload a file, enter PUT <name_of_file>  |\n");
  printf("|     To exit the DFC, enter Exit               |\n");
  printf("*-----------------------------------------------*\n");
  char user_input[100];

  while (fgets(user_input, 100, stdin)) {
    deleteSubstring(user_input, "\n");
    if (strncmp(user_input, "Exit", strlen("Exit")) == 0) {
      printf("Exiting DFC client...\n");
      break;
    }
    if (strncmp(user_input, "LIST", strlen("LIST")) == 0) {
      handle_list(user_input, &client_params, &destination_matrix);
    }
    if (strncmp(user_input, "PUT", strlen("PUT")) == 0) {
      handle_put(user_input, &client_params, &destination_matrix);
    }

    if (strncmp(user_input, "GET", strlen("GET")) == 0) {
      handle_get(user_input, &client_params, &destination_matrix, &locations);
    }

  }

  // Free the memory allocated for the two dynamic strings within the ClientFileContent struct 
  free (client_params.username);
  free (client_params.password);
}


/*-------------------------------------------------------------------------------------------------------
 * handle_get - this function will be responsible for downloading the selected file from the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_get (char *get_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix, struct FilePortionLocations *locations) {

  /* Structs needed to be able to call the stat function which checks for file presence */
  struct stat buffer;
  /* file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to */
  char *file_name, read_line[200], *extra_args;

  /*-----------------------------------
   * Error Checking / Input Validation
   *----------------------------------- */
  /* Extract the second word from the GET command (the filename) and strip it of newline character */
  strtok_r(get_command, " ", &file_name);
  if (file_name == NULL) {
    printf("You have not specified a file to download. Please try again...\n");
    return 1;
  }

  /* Extract any potential arguments following our file name which should not be there */
  strtok_r(file_name, " ", &extra_args);
  if (extra_args != NULL) {
    printf("You have specified too many arguments. Please try again...\n");
    return 1;
  }

  /*------------------------
   * File Download Operations
   *------------------------*/
  ssize_t server_header_ack_size;
  char server_header_ack_buffer[64];

  char message_pn_header[256];
  char recv_pn_response_ack[] = "Received the PNs!";
  char thumbs_up_ack[] = "I'm ready for the PNs!";
  char server_message_buffer[1024];
  ssize_t server_message_size;
  construct_getpn_header(file_name, params, message_pn_header);
  printf("This is our get request so far: \n%s\n",message_pn_header );

  int i;
  int server;
  for (i = 1; i < 5; i++)
  {
    server = create_socket_to_server(i, params);

    if ( (send(server, message_pn_header, strlen(message_pn_header), 0)) == -1)
      printf("Error with sending the header to the first server");

    server_header_ack_size = recv(server, server_header_ack_buffer, 1024, 0);
    printf("Server: %s\n",server_header_ack_buffer );

    if ( (send(server, thumbs_up_ack, strlen(thumbs_up_ack), 0)) == -1)
      printf("Error with sending the thumbs up ack to the server");

    server_message_size = recv(server, server_message_buffer, 1024, 0);
    printf("Server: %s\n",server_message_buffer );
    if ( (send(server, recv_pn_response_ack, strlen(recv_pn_response_ack), 0)) == -1)
      printf("Error with sending the ack to the server");
    update_locations_array(server_message_buffer, locations,i);
    printf("Portion 1: (portion number) %d (server number) %d\n", locations->portion_locations[0][0], locations->portion_locations[0][1]);
    printf("Portion 2: (portion number) %d (server number) %d\n", locations->portion_locations[1][0], locations->portion_locations[1][1]);
    printf("Portion 3: (portion number) %d (server number) %d\n", locations->portion_locations[2][0], locations->portion_locations[2][1]);
    printf("Portion 4: (portion number) %d (server number) %d\n", locations->portion_locations[3][0], locations->portion_locations[3][1]);
    if ( (check_locations_array(locations)) == 0) {
      printf("We have enough portions!! Time to start retreving actual file content!\n");
    }
    else
      printf("Not enough portions yet...\n\n\n");
    memset(&server_message_buffer, 0, sizeof(server_message_buffer));
    sleep(2);
    close(server);
  }
  return 0;
}


int check_locations_array(struct FilePortionLocations *locations) {

  int i;
  for (i = 0; i < 4; i++)
  {
    if ( (locations->portion_locations[i][0] == 111) && (locations->portion_locations[i][1] == 111) )
      return 1;
  }
  return 0;
}
void update_locations_array(char *server_message, struct FilePortionLocations *locations, int port_number) {

  char *token;
  int first_portion_number, second_portion_number;

  token = strtok(server_message, " ");
  token = strtok(NULL, " ");
  //printf("And this should be our first portion number : %s\n", token);
  first_portion_number = atoi(token);
  token = strtok(NULL, " ");
  //printf("And this should be our second portion number : %s\n", token);
  second_portion_number = atoi(token);
  locations->portion_locations[first_portion_number-1][0] = first_portion_number;
  locations->portion_locations[first_portion_number-1][1] = port_number;

  locations->portion_locations[second_portion_number-1][0] = second_portion_number;
  locations->portion_locations[second_portion_number-1][1] = port_number;

}
/*-------------------------------------------------------------------------------------------------------
 * COMPLETE - calculate_hash_modulo_value - this function will take in the file name, open the file, calculate hash, extract the last byte, perform a modulo 4 operation on the decimal value of the last byte of the hash, return val
 *------------------------------------------------------------------------------------------------------- */
int calculate_hash_modulo_value(char * file_name)
{
  //printf("||>> Hello from calculate_hash_modulo_value\n");

  FILE *users_file;
  users_file = fopen(file_name, "r");

  unsigned char c[MD5_DIGEST_LENGTH], data[1024];
  int i, bytes, last_hex_byte, hash_modulo_4;
  long ret;
  char *ptr, hex_byte[4];
  MD5_CTX mdContext;
  memset(&data, 0, sizeof(data));

  MD5_Init(&mdContext);
  while ((bytes = fread (data, 1, 1024, users_file)) !=0) {
    MD5_Update (&mdContext, data, bytes);
  }
  MD5_Final (c, &mdContext);
  //printf("    This is the complete hash value of our file in hex: ");
  for (i = 0; i < MD5_DIGEST_LENGTH; i+=1)  {
    //printf("%02x", c[i]);
  }

  snprintf(hex_byte, 4, "%d", c[15]);
  last_hex_byte = atoi(hex_byte);
  hash_modulo_4 = last_hex_byte % 4;
  //printf("    This is the last byte of the hash value in hex: %02x and in decimal: %d and in modulo 4: %d\n", c[15], last_hex_byte, hash_modulo_4);
  fclose(users_file);
  return hash_modulo_4;
}
/*-------------------------------------------------------------------------------------------------------
 * COMPLETE - handle_put - this function will be responsible for uploading all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_put (char *put_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix){
  // Structs needed to be able to call the stat function for file information
  struct stat buffer;
  // file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to, extra_args is for any additional parameters that the user wrongly supplied */
  char *file_name, read_line[200], *extra_args;

  /* Extract the second word from the PUT command (the filename) and strip it of newline character */
  strtok_r(put_command, " ", &file_name);
  if (file_name == NULL) {
    printf("You have not specified a file to upload. Please try again...\n");
    return 1;
  }

  /* Extract any potential arguments following our file name which should not be there */
  strtok_r(file_name, " ", &extra_args);
  if (extra_args != NULL) {
    printf("You have specified too many arguments. Please try again...\n");
    return 1;
  }

  /* Check for file precense  */
  if (stat (file_name, &buffer) != 0) {
    printf("The file you have tried to upload does exist, please try again\n");
    return 1;
  }
  /* Actually open the file that the user specified from the command line */
  FILE *users_file;
  users_file = fopen(file_name, "r");
  if (users_file == NULL) { 
    perror("Opening user file: ");
    return 1;
  }
  char generic_filename [] = "%s.%d";

  // Construct filename for first portion of original file 
  char portion_one_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_one_filename, 0, sizeof(portion_one_filename));
  snprintf(portion_one_filename, sizeof(portion_one_filename), generic_filename, file_name, 1);

  // Construct filename for second portion of original file 
  char portion_two_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_two_filename, 0, sizeof(portion_two_filename));
  snprintf(portion_two_filename, sizeof(portion_two_filename), generic_filename, file_name, 2);

  // Construct filename for third portion of original file 
  char portion_three_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_three_filename, 0, sizeof(portion_three_filename));
  snprintf(portion_three_filename, sizeof(portion_three_filename), generic_filename, file_name, 3);

  // Construct filename for fourth portion of original file 
  char portion_four_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_four_filename, 0, sizeof(portion_four_filename));
  snprintf(portion_four_filename, sizeof(portion_four_filename), generic_filename, file_name, 4);

  ssize_t file_size, file_size_copy;
  ssize_t portion_one_size, portion_two_size, portion_three_size, portion_four_size;

  // Get size of file that user specified 
  file_size = buffer.st_size; 
  file_size_copy = file_size;

  // Set the file sizes of each portion based on the original size of the complete file 
  portion_one_size = file_size / 4;
  file_size_copy -= portion_one_size;

  portion_two_size = file_size / 4;
  file_size_copy -= portion_two_size;

  portion_three_size = file_size / 4;
  file_size_copy -= portion_one_size;

  portion_four_size = file_size_copy;

  // Array that will store all the mappings of portion number -> server number based on the modulo value of the hash
  int server_location_array[8];
  int hash_value = calculate_hash_modulo_value(file_name);

  switch (hash_value)
  {
    case 0 :
      memcpy(&server_location_array, matrix->hash_combo_zero_servers, sizeof(server_location_array));
      break;
    case 1 :
      memcpy(&server_location_array, matrix->hash_combo_one_servers, sizeof(server_location_array));
      break;
    case 2 :
      memcpy(&server_location_array, matrix->hash_combo_two_servers, sizeof(server_location_array));
      break;
    case 3 :
      memcpy(&server_location_array, matrix->hash_combo_three_servers, sizeof(server_location_array));
      break;
    default :
      printf("Invalid hash value, there must be an issue in the hash calc function\n" );
  }

  /*
     printf("    These are the values of the server_location\n");
     printf("      File Portion 1 going to server #%d and server #%d\n", server_location_array[0], server_location_array[1]);
     printf("      File Portion 2 going to server #%d and server #%d\n", server_location_array[2], server_location_array[3]);
     printf("      File Portion 3 going to server #%d and server#%d\n", server_location_array[4], server_location_array[5]);
     printf("      File Portion 4 going to server #%d and server #%d\n", server_location_array[6], server_location_array[7]);
     printf("==============================================================================================\n");
     printf("==============================================================================================\n\n");
     */
  // This will call the send_file command which will send portion one to the servers designated to receive portion 1
  send_file(server_location_array[0], server_location_array[1], 1, portion_one_size, users_file, params, portion_one_filename);

  // This will call the send_file command which will send portion two to the servers designated to receive portion two
  send_file(server_location_array[2], server_location_array[3], 2, portion_two_size, users_file, params, portion_two_filename);

  // This will call the send_file command which will send portion three to the servers designated to receive portion three
  send_file(server_location_array[4], server_location_array[5], 3, portion_three_size, users_file, params, portion_three_filename);

  // This will call the send_file command which will send portion four to the servers designated to receive portion four
  send_file(server_location_array[6], server_location_array[7], 4, portion_four_size, users_file, params, portion_four_filename);

  return 0;
}
/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * COMPLETE - send_file - this function will actually open up the correct sockets, send the header, and implement the send, receive ack, send again setup with the server
 *---------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void send_file (int first_server_number, int second_server_number, int portion_number, ssize_t portion_size, FILE *user_file, struct ClientFileContent *params, char *portion_file_name) {

  // set up the size variables that will hold the return values of all the calls to fread and send
  ssize_t bytes_read_from_file, total_bytes_read_from_file, bytes_written_to_first_server,total_bytes_written_to_first_server, bytes_written_to_second_server, total_bytes_written_to_second_server;

  // set up the two sockets that will connect to the two servers where this file portion will go
  int server_one, server_two;
  server_one = create_socket_to_server(first_server_number, params);
  server_two = create_socket_to_server(second_server_number, params);

  // convert the portion_size from a ssize_t type to a char so that we can include it into our header as text
  char portion_size_char[16];
  snprintf(portion_size_char, sizeof(portion_size_char), "%zu", portion_size);

  // setup and populate the header for the put request
  char message_header[256];
  construct_put_header(portion_file_name, portion_size_char, params, message_header);

  // the two buffers which will keep the data that we incrementally send to the servers
  char data_buffer[1024], data_buffer_server_two[1024];
  ssize_t portion_size_copy;
  portion_size_copy = portion_size;
  total_bytes_read_from_file = 0;
  total_bytes_written_to_first_server = 0;
  total_bytes_written_to_second_server = 0;
  //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  //printf("This is the portion size of the file that we are about to read %zu\n", portion_size);
  // Send the file message header to both servers, THEN start sending the body in chunks of 1024 bytes 
  if ( (send(server_one, message_header, strlen(message_header), 0)) == -1)
    printf("Error with sending the header to the first server");
  if ( (send(server_two, message_header, strlen(message_header), 0)) == -1)
    printf("Error with sending the header to the second server");

  ssize_t first_server_message_size, second_server_message_size;
  char first_server_message_buffer [1024], second_server_message_buffer [1024];
  first_server_message_size = recv(server_one, first_server_message_buffer, 1024, 0);
  second_server_message_size = recv(server_two, second_server_message_buffer, 1024, 0);
  if (first_server_message_size != 0)
    printf("Server #%d: %s\n",first_server_number,first_server_message_buffer );
  if (second_server_message_size != 0)
    printf("Server #%d: %s\n",second_server_number,second_server_message_buffer );
  memset(&first_server_message_buffer, 0, sizeof(first_server_message_buffer));
  memset(&second_server_message_buffer, 0, sizeof(second_server_message_buffer));
  while (total_bytes_read_from_file != portion_size)
  {
    if (portion_size_copy > 1024) {
      //printf("&& Portion still too big to fit into buffer...\n");
      memset(&data_buffer, 0, sizeof(data_buffer));
      memset(&data_buffer_server_two, 0, sizeof(data_buffer_server_two));
      bytes_read_from_file = fread(data_buffer, 1, 1024, user_file);
      memcpy(&data_buffer_server_two, &data_buffer, sizeof(data_buffer_server_two));
      printf("This is how many bytes were read from the file (should be 1024):%zu\n", bytes_read_from_file);
      total_bytes_read_from_file += bytes_read_from_file;
      bytes_written_to_first_server = send(server_one, data_buffer, bytes_read_from_file, 0);
      bytes_written_to_second_server = send(server_two, data_buffer_server_two, bytes_read_from_file, 0);

      if (bytes_written_to_first_server == -1)
        printf("Error with writing bytes to server %d\n", first_server_number);
      if (bytes_written_to_second_server == -1)
        printf("Error with writing bytes to server %d\n", second_server_number);

      total_bytes_written_to_first_server += bytes_written_to_first_server;
      total_bytes_written_to_second_server += bytes_written_to_second_server;
      printf("This is how many bytes were just written to server %d:%zu bytes and to server %d:%zubytes\n",first_server_number, bytes_written_to_first_server, second_server_number, bytes_written_to_second_server);
      portion_size_copy -= bytes_written_to_first_server;
      printf("This is how many bytes have been read total: %zu\n", total_bytes_read_from_file);
      printf("This is how many bytes have been written total to server %d: %zu and to server %d:%zu\n", first_server_number, total_bytes_written_to_first_server, second_server_number, total_bytes_written_to_second_server);
      printf("This is how many bytes we have left to read/write from the portion: %zu\n", portion_size_copy);
    }
    else {
      //printf("&& Portion should now fit into buffer...\n");
      memset(&data_buffer, 0, sizeof(data_buffer));
      memset(&data_buffer_server_two, 0, sizeof(data_buffer_server_two));
      bytes_read_from_file = fread(data_buffer, 1, portion_size_copy, user_file);
      memcpy(&data_buffer_server_two, &data_buffer, sizeof(data_buffer_server_two));
      printf("This is how many bytes were read from the file %zu bytes\n", bytes_read_from_file);
      total_bytes_read_from_file += bytes_read_from_file;
      bytes_written_to_first_server = send(server_one, data_buffer, bytes_read_from_file, 0);
      bytes_written_to_second_server = send(server_two, data_buffer_server_two, bytes_read_from_file, 0);
      if (bytes_written_to_first_server == -1)
        printf("Error with writing bytes to server %d\n", first_server_number);
      if (bytes_written_to_second_server == -1)
        printf("Error with writing bytes to server %d\n", second_server_number);
      total_bytes_written_to_first_server += bytes_written_to_first_server;
      total_bytes_written_to_second_server += bytes_written_to_second_server;
      printf("This is how many bytes were just written to server %d:%zu bytes and to server %d:%zubytes\n",first_server_number, bytes_written_to_first_server, second_server_number, bytes_written_to_second_server);
      printf("This is how many bytes have been read total: %zu\n", total_bytes_read_from_file);
      printf("This is how many bytes have been written total to server %d: %zu and to server %d:%zu\n", first_server_number, total_bytes_written_to_first_server, second_server_number, total_bytes_written_to_second_server);
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
    first_server_message_size = recv(server_one, first_server_message_buffer, 1024, 0);
    second_server_message_size = recv(server_two, second_server_message_buffer, 1024, 0);
    printf("Server #%d: %s\n",first_server_number,first_server_message_buffer );
    printf("Server #%d: %s\n",second_server_number,second_server_message_buffer );

  }
  printf("All done with sending from client\n");
  close(server_one);
  close(server_two);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------
 * COMPLTE - create_socket_to_server - this function is responsible for creating a socket connection using the passed in server number and conf sturct
 *---------------------------------------------------------------------------------------------------------------------------------------------------------- */
int create_socket_to_server(int server_number, struct ClientFileContent *params) {
  //printf("||>> Hello from create_socket_to_server\n");
  int sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
    printf("Could not create socket\n");

  struct sockaddr_in server;
  int port_number;
  printf("    Creating socket to server #%d which has port_number %s", server_number, params->ports[server_number-1]);
  port_number = atoi(params->ports[server_number-1]);

  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_family = AF_INET;
  server.sin_port = htons(port_number);

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
    perror("    connect failed. Error\n");
  }
  else
    printf("    connected\n");
  return sock;
}
/*-------------------------------------------------------------------------------------------
 * COMPLETE - construct_put_header - this function is responsible for assembling the put request header
 *-------------------------------------------------------------------------------------------*/
void construct_put_header(char *filename, char *filesize, struct ClientFileContent *params, char *header) 
{
  strcpy(header, "&**&STXPUT ");
  strcat(header, filename);
  strcat(header, " ");
  strcat(header, filesize);
  strcat(header, "\n");
  strcat(header, params->username);
  strcat(header, "\n");
  strcat(header, params->password);
  strcat(header, "\n");
}
/*-------------------------------------------------------------------------------------------
 * construct_getpn_header - this function is responsible for assembling the put request header
 *-------------------------------------------------------------------------------------------*/
void construct_getpn_header(char *filename, struct ClientFileContent *params, char *header) 
{
  strcpy(header, "&**&STXGETPN ");
  strcat(header, filename);
  strcat(header, "\n");
  strcat(header, params->username);
  strcat(header, "\n");
  strcat(header, params->password);
  strcat(header, "\n");
}
/*-------------------------------------------------------------------------------------------
 * construct_get_header - this function is responsible for assembling the put request header
 *-------------------------------------------------------------------------------------------*/
void construct_get_header(char *filename, struct ClientFileContent *params, char *header) 
{
  strcpy(header, "&**&STXGET ");
  strcat(header, filename);
  strcat(header, "\n");
  strcat(header, params->username);
  strcat(header, "\n");
  strcat(header, params->password);
  strcat(header, "\n");
}
/*-------------------------------------------------------------------------------------------
 * construct_list_header - this function is responsible for assembling the put request header
 *-------------------------------------------------------------------------------------------*/
void construct_list_header(struct ClientFileContent *params, char *header) 
{
  strcpy(header, "&**&STXLIST ");
  strcat(header, "\n");
  strcat(header, params->username);
  strcat(header, "\n");
  strcat(header, params->password);
  strcat(header, "\n");
}
/*-------------------------------------------------------------------------------------------------------
 * handle_list - this function will be responsible for listing all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_list (char *get_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix){
  printf("Hello form handle_list\n");
  printf("1.txt \n 2.txt \n 3.txt");
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * COMPLETE - parse_client_conf_file - this function takes in a file name and the client params struct, and will popuate the struct after parsing and extracting info from the dfc.conf file 
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void parse_client_conf_file(struct ClientFileContent *params, char *file_name) {
  // Structs needed to be able to call the stat function which checks for file presence 
  struct stat buffer;

  // leftover used in strtok_r to capture content after space, read_line is buffer that fgets writes to
  char *leftover, read_line[200];

  // file pointer for our dfc.conf file 
  FILE *config_file;

  // check if conf file exists 
  if (stat (file_name, &buffer) != 0) {
    perror("Conf file doesn't exist: ");
    exit(-1);
  }

  // check if conf file can be opened 
  config_file = fopen(file_name, "r");
  if (config_file == NULL) {
    perror("Opening conf file: ");
    exit(-1);
  }

  // iterator value to keep track of what index to update in conf struct 
  int current_server;
  current_server = 0;

  // another char pointer for strtok usage
  char *token;
  while (fgets (read_line,200, config_file) != NULL) {
    // Extract the address, port, server number information
    if(strstr(read_line, "Server") != NULL) {
      token = strtok(read_line, " ");
      token = strtok(NULL, " ");
      token = strtok(NULL, ":");
      // zero out buffer where we copy string to prevent weird ending chars resulting from uninitialized mem
      memset(params->addresses[current_server], 0, sizeof(params->addresses[current_server]));
      // remove potential newline character at end of extracted field
      deleteSubstring(token, "\n");
      strcpy(params->addresses[current_server], token);
      token = strtok(NULL, ":");
      memset(params->ports[current_server], 0, sizeof(params->ports[current_server]));
      deleteSubstring(token, "\n");
      strcpy(params->ports[current_server], token);
      current_server++;
    }

    // Extract the user name field
    if(strstr(read_line, "Username") != NULL) {
      strtok_r(read_line, " ", &leftover);
      deleteSubstring(leftover, "\n");
      params->username = malloc(strlen(leftover)+1);
      strcpy(params->username,leftover);
    }
    // Extract the password name field
    if(strstr(read_line, "Password") != NULL) {
      strtok_r(read_line, " ", &leftover);
      deleteSubstring(leftover, "\n");
      params->password= malloc(strlen(leftover)+1);
      strcpy(params->password,leftover);
    }
  }
  fclose(config_file);
}
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * COMPLETE - deleteSubstring - this function is a helper function that is used to remove newline characters from the end of extracted strings
 *------------------------------------------------------------------------------------------------------------------------------------------- */
void deleteSubstring(char *original_string,const char *sub_string) {
  while( (original_string=strstr(original_string,sub_string)) )
    memmove(original_string,original_string+strlen(sub_string),1+strlen(original_string+strlen(sub_string)));
}
