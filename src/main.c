#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "server.h"

server_t* server;

void sig_int_callback(int signal) {
  printf("Forcing server to shut down...\n");
  deinit_server(server);
}

int main(int argc, char** argv) {
  signal(SIGINT, sig_int_callback);

  server = init_server("", 8081);

  if(server) {
    run_server(server);
  } else {
    puts("ERROR: Aborting\n");
    return 1;
  }

  return 0;
}
