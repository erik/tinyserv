#pragma once

#ifndef _HTML_H_
#define _HTML_H_

#include "str.h"

extern char html_auto_free;

#define ATTRIBUTES(...) (char*[]){__VA_ARGS__, NULL}
#define CONTENT(...)    (char*[]){__VA_ARGS__, NULL}

#define HEAD(...)       (string_t*[]){__VA_ARGS__, NULL}
#define BODY(...)       (string_t*[]){__VA_ARGS__, NULL}

#define ID(name) ("id=" #name )
#define CLASS(name) ("class=" #name)

#define HTML_BREAK "<br />"

#define DOCTYPE_HTML5 "<!DOCTYPE html>\n<html>"
#define DOCTYPE_XHTML "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" \
  "\n\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"          \
  "\n<html xmlns=\"http://www.w3.org/1999/xhtml\">"


/**
 * Converts a series of tags to HTML. First argument is the DOCTYPE of the
 * HTML, second element is a NULL terminated string_t*[] containing all the tags
 * in <head>, and body is a NULL terminated string_t*[] containing all the tags
 * in <body>.
 *
 * If html_auto_free is set to 1, then each of the tags passes to htmlize will
 * automatically be freed, so temporary variables do not need to be used to
 * avoid leaking memory
 */
string_t* htmlize(char* doctype, string_t* head[], string_t* body[]);

/**
 * Generates a string representation of an HTML tag, with the given tagname.
 * Attrs is a NULL terminated char*[] containing all the attributes of the
 * tag (id, class, and whatever other attributes are necessary). Content is
 * a NULL terminated char*[] that contains all the content that will be placed
 * inside of the tag. <tag>CONTENT</tag>. Self close determines whether the tag
 * will close itself: <tag />, or not: <tag></tag>
 */
string_t* html_tag(char* tagname, char* attrs[], char* content[], int selfClose);

/**
 * Returns an HTML-escaped representation of a given char* of text.
 */
string_t* html_escape(char* text);

#endif /* _HTML_H_ */
