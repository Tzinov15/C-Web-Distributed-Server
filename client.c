#include "client.h"

struct ClientFileContent {
  char *username;
  char *password;
};


void deleteSubstring(char *original_string,const char *sub_string);
void parse_client_conf_file(struct ClientFileContent *params, char *file_name);
int handle_put (char *put_command);
void handle_list ();
//void handle_list (char *list_of_files);

int main(int argc, char ** argv) {
  printf("| Welcome to this wonderful C client! |\n\n");

  struct ClientFileContent client_params;


  if (argc < 2) {
    printf("Please specify a dfc.conf file\n");
    exit(1); }

  parse_client_conf_file(&client_params, argv[1]);

  /* Free the memory allocated for the strings within the ClientFileContent struct */

printf("*-----------------------------------------------*\n");
printf("|  To see of a list of your files, enter LIST   |\n");
printf("|  To download a file, enter GET <name_of_file> |\n");
printf("|   To upload a file, enter PUT <name_of_file>  |\n");
printf("|     To exit the DFC, enter Exit               |\n");
printf("*-----------------------------------------------*\n");
  char user_input[100];
  char *server_file_list;

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
        handle_put(user_input);
      }

    }
  }
  free (client_params.username);
  free (client_params.password);
}


/*-------------------------------------------------------------------------------------------------------
 * handle_put - this function will be responsible for uploading all the files available on the DFS servers
 *------------------------------------------------------------------------------------------------------- */
int handle_put (char *put_command) {
  /* Structs needed to be able to call the stat function which checks for file presence */
  struct stat buffer;

  /* file_name used in strtok_r to capture content after space, read_line is buffer that fgets writes to */
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
  FILE *file_to_upload;
  file_to_upload = fopen(file_name, "r");
  if (file_to_upload == NULL) {
    perror("Opening user file: ");
    exit(-1);
  }
  printf("here are the contents of the file you wish to upload\n");
  while (fgets (read_line,200, file_to_upload) != NULL) {
    printf("%s\n", read_line);
  }
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
