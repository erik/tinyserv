#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include "server.h"

server_t* server;

void sig_int_callback(int signal) {
  printf("\nForcing server to shut down...\n");
  deinit_server(server);
}

static struct option args[]  = {
  {"directory", required_argument, 0, 'd'},
  {"port", required_argument, 0, 'p'},
  {"ignore-dot", no_argument, 0, 'i'},
  {"help", no_argument, 0, '?'},
  {0, 0, 0, 0}
};

static int usage() {
  printf("tinyserv [options]\n"
         "  --directory\t-d [directory]\tdirectory to run from\n"
         "  --port\t-p [port]\tport to run on\n"
         "  --ignore-dot\t-i\tignore dot (hidden) files\n"
         "  --help\t-?\t\tshow this text\n");
  return 0;
}

int main(int argc, char** argv) {
  char* directory = ".";
  char* port = "8080";

  int option_index = 0;
  int ign_dot = 0;

  while(1) {
    int c = getopt_long(argc, argv, "d:p:i?:", 
                        args, &option_index);
    if(c == -1) {
      break;
    }

    switch(c) {
    case 'd':
      directory = optarg;
      break;
    case 'p':
      port = optarg;
      break;
    case 'i':
      ign_dot = 1;
      break;
    case '?':
    default:
      return usage();
    }
  }

  int port_num = atoi(port);

  if(port_num == 0) {
    printf("Invalid port number, '%s'\n", port);
    return usage();
  }
  
  /* strip trailing / from directory if it exists */
  int len = strlen(directory);
  if(directory[len - 1] == '/') {
    directory[len - 1] = '\0';
  }
  
  server = init_server(directory, port_num, ign_dot);

  if(server) {
    signal(SIGINT, sig_int_callback);
    run_server(server);
  } else {
    puts("ERROR: Aborting\n");
    return 1;
  }

  return 0;
}
