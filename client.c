#include "client.h"
int main(int argc, char ** argv) {

  struct FileCombos destination_matrix;
  setup_file_matrix(&destination_matrix);

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
        handle_put(user_input, &client_params);
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


void setup_file_matrix(struct FileCombos *matrix) {

  struct FileDistributionCombination combo0;
  combo0.hash_modulo_value = 0;

  combo0.server1_files[0] = 1;
  combo0.server1_files[1] = 2;

  combo0.server2_files[0] = 2;
  combo0.server2_files[1] = 3;

  combo0.server3_files[0] = 3;
  combo0.server3_files[1] = 4;

  combo0.server4_files[0] = 4;
  combo0.server4_files[1] = 1;

  
  struct FileDistributionCombination combo1;
  combo1.hash_modulo_value = 1;
  combo1.server1_files[0] = 4;
  combo1.server1_files[1] = 1;

  combo1.server2_files[0] = 1;
  combo1.server2_files[1] = 2;

  combo1.server3_files[0] = 2;
  combo1.server3_files[1] = 3;

  combo1.server4_files[0] = 3;
  combo1.server4_files[1] = 4;

  struct FileDistributionCombination combo2;
  combo2.hash_modulo_value = 2;
  combo2.server1_files[0] = 3;
  combo2.server1_files[1] = 4;

  combo2.server2_files[0] = 4;
  combo2.server2_files[1] = 1;

  combo2.server3_files[0] = 1;
  combo2.server3_files[1] = 2;

  combo2.server4_files[0] = 2;
  combo2.server4_files[1] = 3;
  
  struct FileDistributionCombination combo3;
  combo3.hash_modulo_value = 3;
  combo3.server1_files[0] = 2;
  combo3.server1_files[1] = 3;

  combo3.server2_files[0] = 3;
  combo3.server2_files[1] = 4;

  combo3.server3_files[0] = 4;
  combo3.server3_files[1] = 1;

  combo3.server4_files[0] = 1;
  combo3.server4_files[1] = 2;

  matrix->x0 = combo0;
  matrix->x1 = combo1;
  matrix->x2 = combo2;
  matrix->x3 = combo3;
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
int calculate_hash_modulo_value(char * file_name)
{
printf("Hello from calculate_hash_modulo_value\n");

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
  printf("This is the complete hash value of our file in hex: ");
  for (i = 0; i < MD5_DIGEST_LENGTH; i+=1)  {
    printf("%02x", c[i]);
  }
  printf("\n");

  printf("This is the last byte of the hash value in hex: %02x\n", c[15]);
  snprintf(hex_byte, 4, "%d", c[15]);
  last_hex_byte = atoi(hex_byte);
  printf("This is the decimal value of the last byte of the hash: %d\n", last_hex_byte);
  hash_modulo_4 = last_hex_byte % 4;
  printf("This is the modulus 4 value of the hash %d\n", hash_modulo_4);
  fclose(users_file);
  return hash_modulo_4;

}

/*-------------------------------------------------------------------------------------------------------
 * handle_put - this function will be responsible for uploading all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_put (char *put_command, struct ClientFileContent *params) {

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

  FILE *users_file;
  users_file = fopen(file_name, "r");
  /* Actually open the file that the user specified from the command line */

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

  calculate_hash_modulo_value(file_name);


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
  printf("The size of the original file is %zd\n", file_size); 

  /* Set the file sizes of each portion based on the original size of the complete file */
  portion_one_size = file_size / 4;
  file_size_copy -= portion_one_size;

  portion_two_size = file_size / 4;
  file_size_copy -= portion_two_size;

  portion_three_size = file_size / 4;
  file_size_copy -= portion_one_size;

  portion_four_size = file_size_copy;


  /* The following code will create a seperate buffer for each portion of the file, read in the correct number of bytes for that portion from the users file, then add the string-terminating character '\0' */
  /* This is the code that breaks up the users file into four seperate chunks */
  char portion_one_buffer [portion_one_size];
  fread(portion_one_buffer, 1, portion_one_size, users_file);
  portion_one_buffer[portion_one_size] = '\0';

  char portion_two_buffer [portion_two_size];
  fread(portion_two_buffer, 1, portion_two_size, users_file);
  portion_two_buffer[portion_two_size] = '\0';

  char portion_three_buffer [portion_three_size];
  fread(portion_three_buffer, 1, portion_three_size, users_file);
  portion_three_buffer[portion_three_size] = '\0';

  char portion_four_buffer [portion_four_size];
  fread(portion_four_buffer, 1, portion_four_size, users_file);
  portion_four_buffer[portion_four_size] = '\0';

  /*------------------------
   * Socket Sending 
   *------------------------*/

  /* These strings declared below will be used to store the string version of the portion sizes so that we can include them as part of the header string that we send to the DFS */
  /* We convert the ssize_t type that the portion_size variables are made up of and convert them to strings using the snprintf command */
  char portion_one_size_char[16], portion_two_size_char[16], portion_three_size_char[16], portion_four_size_char[16];
  snprintf(portion_one_size_char, sizeof(portion_one_size_char), "%zu", portion_one_size);
  snprintf(portion_two_size_char, sizeof(portion_two_size_char), "%zu", portion_two_size);
  snprintf(portion_three_size_char, sizeof(portion_three_size_char), "%zu", portion_three_size);
  snprintf(portion_four_size_char, sizeof(portion_four_size_char), "%zu", portion_four_size);


  /* The declarations below are creating the arrays of chars that will ultimately hold the final messages with headers that we send to the server */
  char portion_one_message[sizeof("PUT ") + sizeof(portion_one_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_one_size];
  char portion_two_message[sizeof("PUT ") + sizeof(portion_two_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_two_size];
  char portion_three_message[sizeof("PUT ") + sizeof(portion_three_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_three_size];
  char portion_four_message[sizeof("PUT ") + sizeof(portion_four_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_four_size];

  /* The calls below are actually populating the arrays that we instantiated above by creating the headers, appending the body, and writing this to the passed in array of chars */
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


  return 0;

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
