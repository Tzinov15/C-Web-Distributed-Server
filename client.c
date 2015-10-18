#include "client.h"
int main(int argc, char ** argv) {
  printf("| Welcome to this wonderful C client! |\n\n");

  struct ClientFileContent client_params;

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

  while (1) {
    while (fgets(user_input, 100, stdin)) {
      deleteSubstring(user_input, "\n");
      if (strncmp(user_input, "Exit", strlen("Exit")) == 0) {
        printf("Exiting DFC client...\n");
        break;
      }
      if (strncmp(user_input, "LIST", strlen("LIST")) == 0) {
        handle_list();
      }
      if (strncmp(user_input, "PUT", strlen("PUT")) == 0) {
        handle_put(user_input, &client_params, &destination_matrix);
      }

      if (strncmp(user_input, "GET", strlen("GET")) == 0) {
        handle_get(user_input);
      }

    }
  }
  /* Free the memory allocated for the strings within the ClientFileContent struct */
  free (client_params.username);
  free (client_params.password);
}


/*-------------------------------------------------------------------------------------------------------
 * handle_get - this function will be responsible for downloading the selected file from the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_get (char *get_command) {

  /* Structs needed to be able to call the stat function which checks for file presence */
  struct stat buffer;
  /* file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to */
  char *file_name, read_line[200], *extra_args;

  /*-----------------------------------
   * Error Checking / Input Validation
   *----------------------------------- */
  /* Extract the second word from the PUT command (the filename) and strip it of newline character */
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
  return 0;
}
/*-------------------------------------------------------------------------------------------------------
 * calculate_hash_modulo_value - this function will take in the file name, open the file, calculate the hash, extract the last byte, perform a modulo 4 operation on the decimal value of the last byte of the hash, return val
 *------------------------------------------------------------------------------------------------------- */
int calculate_hash_modulo_value(char * file_name)
{
  printf("||>> Hello from calculate_hash_modulo_value\n");

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
  printf("    This is the complete hash value of our file in hex: ");
  for (i = 0; i < MD5_DIGEST_LENGTH; i+=1)  {
    printf("%02x", c[i]);
  }
  printf("\n");

  snprintf(hex_byte, 4, "%d", c[15]);
  last_hex_byte = atoi(hex_byte);
  hash_modulo_4 = last_hex_byte % 4;
  printf("    This is the last byte of the hash value in hex: %02x and in decimal: %d and in modulo 4: %d\n", c[15], last_hex_byte, hash_modulo_4);
  fclose(users_file);
  return hash_modulo_4;

}

/*-------------------------------------------------------------------------------------------------------
 * handle_put - this function will be responsible for uploading all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_put (char *put_command, struct ClientFileContent *params, struct FileDistributionCombination *matrix){

  printf("||>> Hi from handle_put\n");
  // Structs needed to be able to call the stat function for file information
  struct stat buffer;
  /* file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to, extra_args is for any additional parameters that the user wrongly supplied */
  char *file_name, read_line[200], *extra_args;


  /*-----------------------------------
   * Error Checking / Input Validation
   *----------------------------------- */
  /* Extract the second word from the PUT command (the filename) and strip it of newline character */
  strtok_r(put_command, " ", &file_name);
  if (file_name == NULL) {
    printf("You have not specified a file to upload. Please try again...\n");
    return 1;
  }

  /* Actually open the file that the user specified from the command line */
  FILE *users_file;
  users_file = fopen(file_name, "r");
  if (users_file == NULL) { 
    perror("Opening user file: ");
    exit(-1); 
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
  /*------------------------
   * File Consruction Operations
   *------------------------*/



  char generic_filename [] = "%s.%d";

  /* Construct filename for first portion of original file */
  char portion_one_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_one_filename, 0, sizeof(portion_one_filename));
  snprintf(portion_one_filename, sizeof(portion_one_filename), generic_filename, file_name, 1);

  /* Construct filename for second portion of original file */
  char portion_two_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_two_filename, 0, sizeof(portion_two_filename));
  snprintf(portion_two_filename, sizeof(portion_two_filename), generic_filename, file_name, 2);

  /* Construct filename for third portion of original file */
  char portion_three_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_three_filename, 0, sizeof(portion_three_filename));
  snprintf(portion_three_filename, sizeof(portion_three_filename), generic_filename, file_name, 3);

  /* Construct filename for fourth portion of original file */
  char portion_four_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_four_filename, 0, sizeof(portion_four_filename));
  snprintf(portion_four_filename, sizeof(portion_four_filename), generic_filename, file_name, 4);



  ssize_t file_size, file_size_copy;
  ssize_t portion_one_size, portion_two_size, portion_three_size, portion_four_size;

  /* Get size of file that user specified */
  file_size = buffer.st_size; 
  file_size_copy = file_size;
  printf("    The size of the original file is %zd\n", file_size); 

  /* Set the file sizes of each portion based on the original size of the complete file */
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

  printf("||>> Back to handle_put\n");
  printf("    These are the values of the server_location\n");
  printf("      File Portion 1 going to server #%d and server #%d\n", server_location_array[0], server_location_array[1]);
  printf("      File Portion 2 going to server #%d and server #%d\n", server_location_array[2], server_location_array[3]);
  printf("      File Portion 3 going to server #%d and server#%d\n", server_location_array[4], server_location_array[5]);
  printf("      File Portion 4 going to server #%d and server #%d\n", server_location_array[6], server_location_array[7]);
  // This will call the send_file command which will send portion one to the servers designated to receive portion 1
  send_file(server_location_array[0], server_location_array[1], 1, portion_one_size, users_file, params, portion_one_filename);

  // This will call the send_file command which will send portion two to the servers designated to receive portion two
  send_file(server_location_array[2], server_location_array[3], 2, portion_two_size, users_file, params, portion_two_filename);
 
  // This will call the send_file command which will send portion three to the servers designated to receive portion three
  send_file(server_location_array[4], server_location_array[5], 3, portion_three_size, users_file, params, portion_three_filename);
  
  // This will call the send_file command which will send portion four to the servers designated to receive portion four
  send_file(server_location_array[6], server_location_array[7], 4, portion_four_size, users_file, params, portion_two_filename);


  /*------------------------
   * Socket Sending 
   *------------------------*/
  /*
  // These strings declared below will be used to store the string version of the portion sizes so that we can include them as part of the header string that we send to the DFS 
  // We convert the ssize_t type that the portion_size variables are made up of and convert them to strings using the snprintf command 
  char portion_one_size_char[16], portion_two_size_char[16], portion_three_size_char[16], portion_four_size_char[16];
  snprintf(portion_one_size_char, sizeof(portion_one_size_char), "%zu", portion_one_size);
  snprintf(portion_two_size_char, sizeof(portion_two_size_char), "%zu", portion_two_size);
  snprintf(portion_three_size_char, sizeof(portion_three_size_char), "%zu", portion_three_size);
  snprintf(portion_four_size_char, sizeof(portion_four_size_char), "%zu", portion_four_size);


  // The declarations below are creating the arrays of chars that will ultimately hold the final messages with headers that we send to the server 
  char portion_one_message[sizeof("PUT ") + sizeof(portion_one_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_one_size];
  char portion_two_message[sizeof("PUT ") + sizeof(portion_two_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_two_size];
  char portion_three_message[sizeof("PUT ") + sizeof(portion_three_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_three_size];
  char portion_four_message[sizeof("PUT ") + sizeof(portion_four_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_four_size];

  / The calls below are actually populating the arrays that we instantiated above by creating the headers, appending the body, and writing this to the passed in array of chars 
  construct_put_message((char *)&portion_one_filename, (char *)&portion_one_size_char, (char *)&portion_one_buffer, params, (char *)&portion_one_message);
  construct_put_message((char *)&portion_two_filename, (char *)&portion_two_size_char, (char *)&portion_two_buffer, params, (char *)&portion_two_message);
  construct_put_message((char *)&portion_three_filename, (char *)&portion_three_size_char, (char *)&portion_three_buffer, params, (char *)&portion_three_message);
  construct_put_message((char *)&portion_four_filename, (char *)&portion_four_size_char, (char *)&portion_four_buffer, params, (char *)&portion_four_message);

  //printf("Portion one: \n%s\n", portion_one_message);
  //printf("\nPortion two: \n%s\n", portion_two_message);
  //printf("\nPortion three: \n%s\n", portion_three_message);
  //printf("\nPortion four: \n%s\n", portion_four_message);

  send_to_server((char *)&portion_one_message, 1, params);
  send_to_server((char *)&portion_two_message, 2, params);
  send_to_server((char *)&portion_three_message, 3, params);
  send_to_server((char *)&portion_four_message, 4, params);

*/
  return 0;

}




void send_file (int first_server_number, int second_server_number, int portion_number, ssize_t portion_size, FILE *user_file, struct ClientFileContent *params, char *portion_file_name) {

printf("||>> Hello from send_file\n");
  ssize_t read_bytes, written_bytes, total_read_bytes, total_written_bytes;
  ssize_t written_bytes_server_two, total_written_bytes_server_two;
  int server_one, server_two;
  server_one = create_socket_to_server(first_server_number, params);
  server_two = create_socket_to_server(second_server_number, params);

  char data_buffer[1024];
  char data_buffer_server_two[1024];
  ssize_t portion_size_copy;
  portion_size_copy = portion_size;
  total_read_bytes = 0;
  total_written_bytes = 0;
  total_written_bytes_server_two = 0;
  printf("This is the portion size of the file that we are about to read %zu\n", portion_size);
  while (total_read_bytes != portion_size) {
    sleep(1);
    if (portion_size_copy > 1024) {
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      read_bytes = fread(data_buffer, 1, 1024, user_file);
      memcpy(&data_buffer_server_two, &data_buffer, sizeof(data_buffer_server_two));
      printf("This is how many bytes were read from the file (should be 1024):%zu\n", read_bytes);
      printf("This is what was read from the file:\n%s\n", data_buffer);
      printf("And this is what was copied to the second buffer:\n%s\n", data_buffer_server_two);
      total_read_bytes += read_bytes;
      written_bytes = send(server_one, data_buffer, read_bytes, 0);
      written_bytes_server_two = send(server_two, data_buffer_server_two, read_bytes, 0);
      if (written_bytes == -1)
        printf("Error with writing bytes to server %d\n", first_server_number);
      if (written_bytes_server_two == -1)
        printf("Error with writing bytes to server %d\n", second_server_number);
      total_written_bytes += written_bytes;
      total_written_bytes_server_two += written_bytes_server_two;
      printf("This is how many bytes were written to server %d (should be 1024): %zu\n",first_server_number, written_bytes);
      printf("This is how many bytes were written to server %d (should be 1024): %zu\n", second_server_number, written_bytes);
      portion_size_copy -= written_bytes;
      printf("This is how many bytes have been read total: %zu\n", total_read_bytes);
      printf("This is how many bytes have been written total to server %d: %zu\n", first_server_number, total_written_bytes);
      printf("This is how many bytes have been written total to server %d: %zu\n", second_server_number, total_written_bytes_server_two);
      printf("This is how many bytes we have left to read/write from the portion: %zu\n", portion_size_copy);
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      memset(&data_buffer, 0, sizeof(data_buffer));
    }
    else {
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      memset(&data_buffer, 0, sizeof(data_buffer));
      memset(&data_buffer_server_two, 0, sizeof(data_buffer));
      read_bytes = fread(data_buffer, 1, portion_size, user_file);
      memcpy(&data_buffer_server_two, &data_buffer, sizeof(data_buffer_server_two));
      printf("This is how many bytes were read from the file %zu\n", read_bytes);
      printf("This is what was read from the file:\n%s\n", data_buffer);
      printf("And this is what was copied to the second buffer:\n%s\n", data_buffer_server_two);
      total_read_bytes += read_bytes;
      written_bytes = send(server_one, data_buffer, read_bytes, 0);
      written_bytes_server_two = send(server_two, data_buffer_server_two, read_bytes, 0);
      if (written_bytes == -1)
        printf("Error with writing bytes to server %d\n", first_server_number);
      if (written_bytes_server_two == -1)
        printf("Error with writing bytes to server %d\n", second_server_number);
      total_written_bytes += written_bytes;
      total_written_bytes_server_two += written_bytes_server_two;
      printf("This is how many bytes were just written to server %d : %zu and to server %d : %zu\n",first_server_number, written_bytes, second_server_number, written_bytes_server_two);
      printf("This is how many bytes have been read total: %zu\n", total_read_bytes);
      printf("This is how many bytes have been written total to server %d: %zu and to server %d : %zu\n", first_server_number, total_written_bytes, second_server_number, total_written_bytes_server_two);
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      memset(&data_buffer, 0, sizeof(data_buffer));
    }

  }
  //close(server_one);
  //close(server_two);

}
int create_socket_to_server(int server_number, struct ClientFileContent *params) {
  printf("||>> Hello from create_socket_to_server\n");
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


/*-----------------------------------------------------------------------------------------------------------------------------------------------------------
 * send_to_server - this function takes in a message body, a server number, and our client param struct and sends the message to the correct server
 *---------------------------------------------------------------------------------------------------------------------------------------------------------- */
void send_to_server(char *message, int server_number, struct ClientFileContent *params) {
  printf("Server #%s Address: %s\n", params->servers[server_number-1], params->addresses[server_number-1]);
  printf("Server #%s Port: %s\n", params->servers[server_number-1], params->ports[server_number-1]);
  printf("Message contents \n%s\n", message);
  printf("Message size %zu\n", strlen(message));

  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
    printf("Could not create socket\n");

  struct sockaddr_in server;
  int port_number;
  port_number = atoi(params->ports[server_number-1]);

  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_family = AF_INET;
  server.sin_port = htons(port_number);

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
    perror("connect failed. Error\n");
    exit(1);
  }
  printf("connected\n");

  ssize_t total_read_bytes;
  total_read_bytes = 0;
  while (total_read_bytes != strlen(message)) {
    total_read_bytes = send(sock, message, strlen(message), 0);
    printf("I just read %zu bytes\n", total_read_bytes);
  }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------
 * construct_put_message - this function is responsible for assembling the full message that we send to the server which includes header fields and the body 
 *---------------------------------------------------------------------------------------------------------------------------------------------------------- */
void construct_put_message(char *filename, char *filesize, char *filecontent, struct ClientFileContent *params, char *final_message) {
  strcpy(final_message, "PUT ");
  strcat(final_message, filename);
  strcat(final_message, " ");
  strcat(final_message, filesize);
  strcat(final_message, "\n");
  strcat(final_message, params->username);
  strcat(final_message, "\n");
  strcat(final_message, params->password);
  strcat(final_message, "\n");
  strcat(final_message, filecontent);
}

/*-------------------------------------------------------------------------------------------------------
 * handle_list - this function will be responsible for listing all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
void handle_list () {
  printf("Hello form handle_list\n");
  printf("1.txt \n 2.txt \n 3.txt");
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * parse_client_conf_file - this function takes in a file name and the client params struct, and will popuate the struct after parsing and extracting info from the dfc.conf file 
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void parse_client_conf_file(struct ClientFileContent *params, char *file_name) {

  /* Structs needed to be able to call the stat function which checks for file presence */
  struct stat buffer;
  /* leftover used in strtok_r to capture content after space, read_line is buffer that fgets writes to */
  char *leftover, read_line[200];

  /* file pointer for our dfc.conf file */
  FILE *config_file;

  if (stat (file_name, &buffer) != 0) {
    perror("Conf file doesn't exist: ");
    exit(-1);
  }

  config_file = fopen(file_name, "r");
  if (config_file == NULL) {
    perror("Opening conf file: ");
    exit(-1);
  }


  int current_server;
  current_server = 0;
  char *token;
  while (fgets (read_line,200, config_file) != NULL) {
    /* Extract the address, port, server number information */
    if(strstr(read_line, "Server") != NULL) {
      token = strtok(read_line, " ");
      token = strtok(NULL, " ");
      strcpy(params->servers[current_server], token);
      token = strtok(NULL, ":");
      strcpy(params->addresses[current_server], token);
      token = strtok(NULL, ":");
      strcpy(params->ports[current_server], token);
      current_server++;
    }

    /* Extract the user name field */
    if(strstr(read_line, "Username") != NULL) {
      strtok_r(read_line, " ", &leftover);
      deleteSubstring(leftover, "\n");
      params->username = malloc(strlen(leftover)+1);
      strcpy(params->username,leftover);
    }
    /* Extract the password name field */
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
 * deleteSubstring - this function is a helper function that is used to remove newline characters from the end of extracted strings
 *------------------------------------------------------------------------------------------------------------------------------------------- */
void deleteSubstring(char *original_string,const char *sub_string) {
  while( (original_string=strstr(original_string,sub_string)) )
    memmove(original_string,original_string+strlen(sub_string),1+strlen(original_string+strlen(sub_string)));
}
