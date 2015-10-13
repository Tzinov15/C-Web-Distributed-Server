#include "client.h"

struct ClientFileContent {
  char *username;
  char *password;
};


void construct_put_message(char *filename, char *filesize, char *filecontent, struct ClientFileContent *params, char *final_message);
void deleteSubstring(char *original_string,const char *sub_string);
void parse_client_conf_file(struct ClientFileContent *params, char *file_name);
int handle_get (char *get_command);
int handle_put (char *put_command, struct ClientFileContent *params);
void handle_list ();
//void handle_list (char *list_of_files);

int main(int argc, char ** argv) {
  printf("| Welcome to this wonderful C client! |\n\n");

  struct ClientFileContent client_params;


  if (argc < 2) {
    printf("Please specify a dfc.conf file\n");
    exit(1); }

  parse_client_conf_file(&client_params, argv[1]);


printf("*-----------------------------------------------*\n");
printf("|  To see of a list of your files, enter LIST   |\n");
printf("|  To download a file, enter GET <name_of_file> |\n");
printf("|   To upload a file, enter PUT <name_of_file>  |\n");
printf("|     To exit the DFC, enter Exit               |\n");
printf("*-----------------------------------------------*\n");
  char user_input[100];
  //char *server_file_list;

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
}
/*-------------------------------------------------------------------------------------------------------
 * handle_put - this function will be responsible for uploading all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_put (char *put_command, struct ClientFileContent *params) {
  /* Structs needed to be able to call the stat function which checks for file presence */
  struct stat buffer;
  /* file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to */
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
  FILE *file_to_upload;

  char generic_filename [] = "%d.%s";

  /* Construct filename for first portion of original file */
  char portion_one_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_one_filename, 0, sizeof(portion_one_filename));
  snprintf(portion_one_filename, sizeof(portion_one_filename), generic_filename, 1, file_name);
  printf("This is the filename of the first portion: %s\n", portion_one_filename);

  /* Construct filename for second portion of original file */
  char portion_two_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_two_filename, 0, sizeof(portion_two_filename));
  snprintf(portion_two_filename, sizeof(portion_two_filename), generic_filename, 2, file_name);
  printf("This is the filename of the second portion: %s\n", portion_two_filename);

  /* Construct filename for third portion of original file */
  char portion_three_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_three_filename, 0, sizeof(portion_three_filename));
  snprintf(portion_three_filename, sizeof(portion_three_filename), generic_filename, 3, file_name);
  printf("This is the filename of the third portion: %s\n", portion_three_filename);

  /* Construct filename for fourth portion of original file */
  char portion_four_filename[sizeof(generic_filename) + sizeof(file_name) + 1];
  memset(&portion_four_filename, 0, sizeof(portion_four_filename));
  snprintf(portion_four_filename, sizeof(portion_four_filename), generic_filename, 4, file_name);
  printf("This is the filename of the fourth portion: %s\n\n", portion_four_filename);



  ssize_t file_size;
  ssize_t file_size_copy;
  ssize_t portion_one_size, portion_two_size, portion_three_size, portion_four_size;

  file_size = buffer.st_size; 
  file_size_copy = file_size;
  printf("The size of the original file is %zd\n", file_size); 

  portion_one_size = file_size / 4;
  printf("The size of the first portion is %zd\n", portion_one_size); 
  file_size_copy -= portion_one_size;

  portion_two_size = file_size / 4;
  printf("The size of the second portion is %zd\n", portion_two_size); 
  file_size_copy -= portion_two_size;

  portion_three_size = file_size / 4;
  file_size_copy -= portion_one_size;
  printf("The size of the third portion is %zd\n", portion_three_size); 

  portion_four_size = file_size_copy;
  printf("The size of the fourth portion is %zd\n", portion_four_size); 

  file_to_upload = fopen(file_name, "r");
  if (file_to_upload == NULL) { perror("Opening user file: ");
    exit(-1); }

  FILE *file_portion_one = fopen("tzinov.txt", "w");

  char portion_one_buffer [portion_one_size];
  fread(portion_one_buffer, 1, portion_one_size, file_to_upload);
  portion_one_buffer[portion_one_size] = '\0';
  printf("this is the result of reading the first fourth of the file into the buffer: %s\n", portion_one_buffer);
  fwrite(portion_one_buffer, 1, portion_one_size, file_portion_one);

  char portion_two_buffer [portion_two_size];
  fread(portion_two_buffer, 1, portion_two_size, file_to_upload);
  portion_two_buffer[portion_two_size] = '\0';
  printf("this is the result of reading the third fourth of the file into the buffer: %s\n", portion_two_buffer);
  fwrite(portion_two_buffer, 1, portion_two_size, file_portion_one);

  char portion_three_buffer [portion_three_size];
  fread(portion_three_buffer, 1, portion_three_size, file_to_upload);
  portion_three_buffer[portion_three_size] = '\0';
  printf("this is the result of reading the second fourth of the file into the buffer: %s\n", portion_three_buffer);
  fwrite(portion_three_buffer, 1, portion_three_size, file_portion_one);

  char portion_four_buffer [portion_four_size];
  fread(portion_four_buffer, 1, portion_four_size, file_to_upload);
  portion_four_buffer[portion_four_size] = '\0';
  printf("this is the result of reading the fourth fourth of the file into the buffer: %s\n", portion_four_buffer);
  fwrite(portion_four_buffer, 1, portion_one_size, file_portion_one);

  /*------------------------
     * Socket Sending 
   *------------------------*/
    int sock1, sock2, sock3, sock4;
    struct sockaddr_in server1;
    struct sockaddr_in server2;
    struct sockaddr_in server3;
    struct sockaddr_in server4;

    char message[100];
    char portion_one_size_char[16], portion_two_size_char[16], portion_three_size_char[16], portion_four_size_char[16];
    snprintf(portion_one_size_char, sizeof(portion_one_size_char), "%zu", portion_one_size);
    snprintf(portion_two_size_char, sizeof(portion_two_size_char), "%zu", portion_two_size);
    snprintf(portion_three_size_char, sizeof(portion_three_size_char), "%zu", portion_three_size);
    snprintf(portion_four_size_char, sizeof(portion_four_size_char), "%zu", portion_four_size);


    char portion_one_message[sizeof("PUT ") + sizeof(portion_one_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_one_size];
    char portion_two_message[sizeof("PUT ") + sizeof(portion_two_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_two_size];
    char portion_three_message[sizeof("PUT ") + sizeof(portion_three_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_three_size];
    char portion_four_message[sizeof("PUT ") + sizeof(portion_four_filename) + 8 + sizeof(params->username) + sizeof(params->password) + portion_four_size];

    construct_put_message((char *)&portion_one_filename, (char *)&portion_one_size_char, (char *)&portion_one_buffer, params, (char *)&portion_one_message);
    construct_put_message((char *)&portion_two_filename, (char *)&portion_two_size_char, (char *)&portion_two_buffer, params, (char *)&portion_two_message);
    construct_put_message((char *)&portion_three_filename, (char *)&portion_three_size_char, (char *)&portion_three_buffer, params, (char *)&portion_three_message);
    construct_put_message((char *)&portion_four_filename, (char *)&portion_four_size_char, (char *)&portion_four_buffer, params, (char *)&portion_four_message);

    printf("!!!!!!!!!!!!!!!!!\n\n");
    printf("The size of portion one message is %zu\n", sizeof(portion_one_message));
    printf("The length of portion one message is %zu\n", strlen(portion_one_message));
    printf("This is our portion one message so far: \n%s\n", portion_one_message);



    /*
    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock1 == -1)
      printf("Could not create socket\n");

    server1.sin_addr.s_addr = INADDR_ANY;
    server1.sin_family = AF_INET;
    server1.sin_port = htons( 10001);

    if (connect(sock1, (struct sockaddr *)&server1, sizeof(server1)) < 0){
      perror("connect failed. Error\n");
      return 1;
    }
    printf("connected\n");

    while (1) {
      printf("Enter message : \n");
      scanf("%s" , message);

      if ( send(sock1, message, strlen(message) , 0) < 0) {
          puts ("send failed");
          return 1;
        }
    }
    */
    return 0;

}

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
//void handle_list (char *list_of_files) {
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

  while (fgets (read_line,200, config_file) != NULL) {
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
