#include "client.h"

struct ClientFileContent {
  char *method;
  char *URI;
  char *httpversion;
};


void parse_client_conf_file(struct ClientFileContent *params, char *file_name);

int main(int argc, char ** argv)
{

  struct stat buffer;
  struct ClientFileContent client_params;

  printf("| Welcome to this wonderful C client! |\n\n");
  if (argc < 2) {
    printf("Please specify a dfc.conf file\n");
    exit(1); }
  if (stat (argv[1], &buffer) != 0) {
    printf("File doesn't seem to exist...\n");
    exit(1); }
  else
    printf("Yay file exists \n");

  parse_client_conf_file(&client_params, argv[1]);

}



void parse_client_conf_file(struct ClientFileContent *params, char *file_name) {
  
  printf("Hello from parse_client_conf_file\n");
  printf("This is the path of the file that was passed in:  %s\n", file_name);
  FILE *config_file;
  char *leftover;
  config_file = fopen(file_name, "r");
  char read_line[200];

  while (fgets (read_line,200, config_file) != NULL) {
    printf("%s \n", read_line);
    if(strstr(read_line, "Username") != NULL) {
      strtok_r(read_line, " ", &leftover);
      printf("%s", leftover);
    }
  }
}
