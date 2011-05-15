#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STREQ(s1, s2) (!strcmp(s1, s2))

server_t* init_server(char* directory, int port) {
  int sockfd;
  struct sockaddr_in addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if(sockfd == 0) {
    perror("Socket creation failed");
    return NULL;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("Socket binding failed");
    return NULL;
  }

  if(listen(sockfd, MAX_CONNECTIONS) < 0) {
    perror("Listen failed");
    return NULL;
  }

  server_t* serv = malloc(sizeof(server_t));
  serv->sockfd = sockfd;
  serv->addr = addr;
  serv->port = port;
  serv->directory = directory;

  return serv;
}

void deinit_server(server_t* serv) {
  close(serv->sockfd);
  free(serv);
}

int run_server(server_t* server) {
  while(1) {
    unsigned addrlen = sizeof(server->addr);
    int new_sock = accept(server->sockfd, (struct sockaddr*)&server->addr, &addrlen);

    if(new_sock < 0) {
      perror("Accept failed");
      return -1;
    }

    /* jump to a child process */
    if(fork() == 0) {
      handle_client(server, new_sock);
    }
  }
}

#define LINE_IS(x) ((strstr(ptr, x)) == ptr)

void handle_client(server_t* server, int sockfd) {
  char buffer[BUF_SIZE];
  
  int size = recv(sockfd, buffer, BUF_SIZE, 0);

  if(size > 0) {
    char* get  = NULL;
    char* host = NULL;

    char* ptr = strtok(buffer, "\r\n");
    while (ptr != NULL) {
      printf ("%s\n", ptr);

      /* only pieces of the header we're concerned with */
      if(LINE_IS("GET ")) {
        get = ptr;
        get += 4;
        int ind = strcspn(get, " ");
        get[ind] = '\0';;
      } else if(LINE_IS("Host: ")) {
        host = ptr;
      }

      ptr = strtok(NULL, "\r\n");
    }    

    if(host == NULL || get == NULL) {
      puts("bad req\n");
      bad_request(sockfd);
    } else {
      puts("good req\n");
      send_client(sockfd, 200, "OK", "text/html", 5, "BALLS");        
    }
  }

  close(sockfd);
  printf("closed\n");
  
  /* exit child process */
  exit(0);
}

void send_client(int sockfd, int resp_num, char* resp_msg, char* type, int len, char* cont) {
  unsigned size = len + strlen(resp_msg) + 100;
  char* msg = calloc(size, 0);

  char resp_num_buf[10];
  snprintf(resp_num_buf, 5, "%d ", resp_num);

  char len_buf[15];
  snprintf(len_buf, 14, "%d\n\n", len);

  strcpy(msg, "HTTP/1.1 ");
  strcat(msg, resp_num_buf);
  strcat(msg, resp_msg);
  strcat(msg, "\n" SERVER_FIELD);
  strcat(msg, "Connection: close\n");
  strcat(msg, "Content-type: ");
  strcat(msg, type);
  strcat(msg, "\nContent-length: ");
  strcat(msg, len_buf);
  strcat(msg, "\n");
  memcpy(msg + strlen(msg) - 1, cont, len);

  send(sockfd, msg, strlen(msg), 0);
}
