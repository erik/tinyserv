#include "server.h"

#include "htmlize/htmlize.h"
#include "htmlize/str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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
      puts("Bad request");
      bad_request(sockfd);
    } else {
      handle_request(server, sockfd, get);
    }
  }

  close(sockfd);
  
  /* exit child process */
  exit(0);
}

void send_client(int sockfd, int resp_num, char* resp_msg, char* type, int len, char* cont) {
  char msg[BUF_SIZE];

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

  send(sockfd, msg, strlen(msg) - 1, 0);
  send(sockfd, cont, len, 0);
}

void handle_request(server_t* serv, int sockfd, char* req) {
  printf("Client requested \"%s\"\n", req);
  /* return a directory listing */
  if(STREQ(req, "/")) {
    string_t* dir_list = string_new2("");

    DIR *dp;
    struct dirent *ep;     
    dp = opendir (serv->directory);
    
    if(dp != NULL) {
      while((ep = readdir(dp))) {
        if(STREQ(ep->d_name, "..") || STREQ(ep->d_name, ".")) {
          continue;
        }

        char html[BUF_SIZE];

        strcpy(html, "\t\t<a href=/");
        strcat(html, ep->d_name);
        strcat(html, ">");
        strcat(html, ep->d_name);
        strcat(html, "</a>\n\t\t<br />\n");
        
        dir_list = string_append_str(dir_list, html);
      }
      closedir (dp);
    }
    else
      perror ("Couldn't open the directory");

    string_t* html = htmlize(DOCTYPE_HTML5,
                             HEAD(
                                  html_tag("title",
                                           ATTRIBUTES(NULL),
                                           CONTENT("tinyserv listing"),
                                           0)),
                             BODY(dir_list));
    send_client(sockfd, 200, "OK", "text/html", html->size, html->str);

    string_del(html);
  } else {
    req += 1;
    if(strstr(req, "..") == req || req[0] == '\0') {
      bad_request(sockfd);
      return;
    }

    char* file = req;
    FILE* fp = fopen(file, "rb");
    
    if(fp == NULL) {
      string_t* str = string_new2("Not found: ");
      str = string_append_str(str, file);
      
      char* s = str->str;
      send_client(sockfd, 404, "Not Found", "text/plain", str->size, s);

      string_del(str);
    } else {
      fseek(fp, 0, SEEK_END);
      unsigned size = ftell(fp) + 1;
      rewind(fp);

      char* content = calloc(size, sizeof(char));
      fread(content, sizeof(char), size, fp);

      /* size - 1 because we don't want to send the trailing '\0' */ 
      send_client(sockfd, 200, "OK", "text/plain; charset=UTF-8", size - 1, content);

      free(content);
    }
  }
}
