#include "htmlize/str.h"

#include <stdlib.h>
#include <string.h>

string_t* string_new(char* str, unsigned length) {
  string_t* string = malloc(sizeof(string_t));
  string->str = calloc(length + 1, sizeof(char));
  
  string->allocated = length + 1;

  unsigned size = str == NULL ? 0 : strlen(str);

  memcpy(string->str, str, size >= length ? length : size);
  string->size = size;

  return string;  
}

string_t* string_new2(char* str) {
  return string_new(str, strlen(str));
}

void string_del(string_t* string) {
  free(string->str);
  free(string);
}

string_t* string_resize(string_t* string, unsigned size) {
  string->str = realloc(string->str, size);
  string->allocated = size;
  return string;
}

string_t* string_append(string_t* base, string_t* string) {
  
  if(string->size + base->size >= base->allocated) {
    unsigned size = base->size;
    base = string_resize(base, string->size + size);
  }

  unsigned size = strlen(string->str);
  memcpy(base->str + base->size, string->str, size);

  return base;
}

string_t* string_append_str(string_t* base, char* string) {
  unsigned len = strlen(string);

  if(len + base->size >= base->allocated) {
    unsigned size = base->size;
    base = string_resize(base, len + size + 10);
  }

  strcpy(base->str + base->size, string);
  base->size = base->size + len;

  return base;
}
