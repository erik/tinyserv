#include "server.h"

#include "htmlize/htmlize.h"
#include "htmlize/str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

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

  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("Socket binding failed");
    return NULL;
  }

  if(listen(sockfd, MAX_CONNECTIONS) < 0) {
    perror("Listen failed");
    return NULL;
  }

  int status = chdir(directory);
  if(status < 0) {
    perror("Changing directory failed");
    return NULL;
  }

  printf("Starting tinyserv on port %d, serving up \"%s\"\n", port, directory);

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
  _exit(0);
}

void send_file_chunked(int sockfd, char* type, FILE* fp) {
  char msg[BUF_SIZE];

  snprintf(msg, BUF_SIZE, 
           "HTTP/1.1 200 OK\n"                   \
           "Content-type: %s\n"                  \
           "Connection: close\r\n"               \
           "Transfer-Encoding: chunked\r\n"      \
           "%s\n\n",
           type,
           SERVER_FIELD);

  send(sockfd, msg, strlen(msg) - 1, 0);

  char chunk[CHUNK_SIZE];

  for(;;) {
    char chunk_sz[10];

    unsigned len = fread(chunk, sizeof(char), CHUNK_SIZE, fp);

    snprintf(chunk_sz, 10, "%X\r\n", len);
    
    send(sockfd, chunk_sz, strlen(chunk_sz), 0);
    send(sockfd, chunk, len, 0);
    send(sockfd, "\r\n", 2, 0);

    if(len != CHUNK_SIZE) {
      break;
    }
  }

  send(sockfd, "0\r\n\r\n", 5, 0);

}

void send_client(int sockfd, int resp_num, char* resp_msg, char* type, int len, char* cont) {
  char msg[BUF_SIZE];

  snprintf(msg, BUF_SIZE, 
           "HTTP/1.1 %d %s\n"                   \
           "Connection: close\n"                \
           "Content-type: %s\n"                 \
           "Content-length: %d\n"               \
           "%s\n\n",
           resp_num, resp_msg,
           type,
           len,
           SERVER_FIELD);

  send(sockfd, msg, strlen(msg) - 1, 0);
  send(sockfd, cont, len, 0);
}


static void get_time(char* buf, int size, struct stat st) {
  struct tm tm_mod;
  time_t mod = st.st_mtime;
  localtime_r(&mod, &tm_mod);
  strftime(buf, size, "%a %b %d %H:%M:%S", &tm_mod);
}

static char *units[] = {
  "bytes", "KiB", "MiB", "GiB"
};

static void list_directory(server_t* serv, int sockfd, char* dir) {
  HTMLElement table = html_elem_new("table", ELEMENT_AUTO_FREE);
  html_elem_add_content(&table, "<tr><th class=links>Files</th><th class=sizes>Size</th>" \
                        "<th class=times>Modified</th></tr>\n");

  int num;
  int i;

  struct dirent **namelist;
  num = scandir(dir, &namelist, 0, alphasort);

  if(num >= 0) {
    for(i = 0; i < num; ++i) {
      if(STREQ(namelist[i]->d_name, "..") || STREQ(namelist[i]->d_name, ".")) {
        continue;
      }

      HTMLElement row = html_elem_new("tr", ELEMENT_AUTO_FREE);

      struct stat buffer;
      {
        char tmp[BUF_SIZE];
        strcpy(tmp, dir);
        strcat(tmp, "/");
        strcat(tmp, namelist[i]->d_name);
        if(lstat(tmp, &buffer) < 0) {
          perror("stat");
          return;
        }
      }

      int is_dir = S_ISDIR(buffer.st_mode);

      char html[BUF_SIZE];
      {
        memset(html, '\0', BUF_SIZE);               
        snprintf(html, BUF_SIZE,
                 "<a href=\"/%s/%s\">%s%c</a><br />",
                 dir,
                 namelist[i]->d_name,
                 namelist[i]->d_name,
                 is_dir ? '/' : ' ');

        HTMLElement cell = html_elem_new("td", ELEMENT_AUTO_FREE);
        html_elem_add_content(&cell, html);
        html_elem_add_elem(&row, cell);
      }

      {
        memset(html, '\0', BUF_SIZE);
        unsigned file_size = buffer.st_size;
        unsigned i;
        for(i = 0; i < 4; ++i) {
          unsigned tmp = file_size / 1024;
          if(tmp <= 0) {
            break;
          } else {
            file_size = tmp;
          }
        }

        snprintf(html, BUF_SIZE, "%u %s<br />", file_size, units[i]);
        
        HTMLElement cell = html_elem_new("td", ELEMENT_AUTO_FREE);
        html_elem_add_content(&cell, is_dir ? "---<br />" : html);
        html_elem_add_elem(&row, cell);
      }


      {
        char t_buf[50];
        get_time(t_buf, 50, buffer);
        snprintf(html, BUF_SIZE, "%s<br />", t_buf);
        HTMLElement cell = html_elem_new("td", ELEMENT_AUTO_FREE);
        html_elem_add_content(&cell, html);
        html_elem_add_elem(&row, cell);
      }

      html_elem_add_elem(&table, row);

      free(namelist[i]);         
    }
    free(namelist);
  }
  else {
    perror ("Couldn't open the directory");
    return;
  }
   
  HTMLElement div   = html_elem_new("div", ELEMENT_AUTO_FREE);
  html_elem_add_attr(&div, "class=container");
  html_elem_add_elem(&div, table);

  HTMLElement css = html_elem_new("style", ELEMENT_AUTO_FREE);
  html_elem_add_attr(&css, "type=\"text/css\"");
  html_elem_add_content(&css, PAGE_CSS);

  HTMLElement back = html_elem_new("a", ELEMENT_AUTO_FREE);
  html_elem_add_attr(&back, "class=title href=/");
  html_elem_add_content(&back, dir);

  HTMLDocument doc = html_doc_new(DOC_HTML5);
  html_doc_set_title(&doc, dir);
  html_doc_add_head_elem(&doc, css);
  html_doc_add_body_elem(&doc, back);
  html_doc_add_body_elem(&doc, div);

  unsigned size;
  char* html = html_doc_create(&doc, &size);

  send_client(sockfd, 200, "OK", "text/html", size, html);
  
  html_doc_destroy(&doc);
  free(html);
}

void handle_request(server_t* serv, int sockfd, char* encdir) {
  // to shut valgrind up
  char *decbuf = calloc(BUF_SIZE, sizeof(char));
  decode_url(encdir, decbuf, BUF_SIZE);
  printf("client requested \"%s\"\n", decbuf);

  char* dir = decbuf;

  if(STREQ(dir, "/")) {
    list_directory(serv, sockfd, ".");
  } else {

    /* skip over '/' */
    dir += 1;

    if(strstr(dir, "..") || dir[0] == '/' || dir[0] == '\0') {
      bad_request(sockfd);
      return;
    }

    struct stat buffer;
    int status = stat(dir, &buffer);

    /* not found */
    if(status < 0) {
      string_t* str = string_new2("Not found: ");
      str = string_append_str(str, dir);
      
      char* s = str->str;
      send_client(sockfd, 404, "Not Found", "text/html", str->size, s);
      
      string_del(str);
    } else if(S_ISDIR(buffer.st_mode)) {
      list_directory(serv, sockfd, dir);
    } else {

      char* file = dir;
      FILE* fp = fopen(file, "rb");

      if(ferror(fp)) {
        printf("IO error on \"%s\"\n", file);
        bad_request(sockfd);
      }    

      char* mime = get_mime_type(file);

      send_file_chunked(sockfd, mime, fp);

      fclose(fp);
    }
  }
  free(decbuf);
}

// TODO: write this
char* get_mime_type(char* filename) {
  char *buf = NULL;
  buf = "text/plain";
  return buf;  
}

// TODO: encode more things
char* encode_url(char* dec, char* buf, int len) {
  int bufptr = 0;
  int i;

  for(i = 0; i < len && dec[i]; ++i) {
    if(dec[i] == ' ') {
      strcpy(buf + bufptr, "%20");
      bufptr += 3;
    } else {
      buf[bufptr++] = dec[i];
    }
  }
  return buf;
}

char* decode_url(char* encoded, char* buf, int len) {
  int bufptr = 0;
  int i;
  for(i = 0; i < len - 2 && encoded[i]; ++i) {
    if(encoded[i] == '%') {
      char code[] = {encoded[i+1], encoded[i+2], '\0'};      
      unsigned c;
      sscanf(code, "%x", &c); 

      buf[bufptr++] = (char)c;
      i += 2;
    } else {
      buf[bufptr++] = encoded[i];
    }
  }
  return buf;
}
