#pragma once

#ifndef _STR_H_
#define _STR_H_

/* simple dynamic string */
typedef struct string {
  unsigned size;      /* how long is the string */
  unsigned allocated; /* how much space is allocated */
  char str[];         /* pointer to string */
} string_t;

string_t* string_new(char* str, unsigned len);
string_t* string_new2(char* str);
void      string_del(string_t* string);
string_t* string_resize(string_t* str, unsigned size);

string_t* string_concat(string_t* a, string_t* b);
string_t* string_concat_str(string_t* a, char* str);

string_t* string_append(string_t* base, string_t* str);
string_t* string_append_str(string_t* base, char* str);

#endif /* _STR_H_ */
