#include "htmlize/htmlize.h"
#include "htmlize/str.h"

#include <stdlib.h>
#include <string.h>

/* HTMLDocument functions */
HTMLDocument html_doc_new(doctype type) {
  HTMLDocument doc;
  doc.type = type;
  
  doc.num_head = doc.num_body = 0;

  doc.head_elems = malloc(0 * sizeof(HTMLElement));
  doc.body_elems = malloc(0 * sizeof(HTMLElement));

  return doc;
}

void html_doc_destroy(HTMLDocument* doc) {
  free(doc->head_elems);
  free(doc->body_elems);
}

void html_doc_set_doctype(HTMLDocument* doc, doctype t) {
  doc->type = t;
}

void html_doc_set_title(HTMLDocument* doc, char* title) {
  HTMLElement e = html_elem_new("title", ELEMENT_AUTO_FREE);
  html_elem_set_content(&e, title);

  html_doc_add_head_elem(doc, e);  
}

void html_doc_add_head_elem(HTMLDocument* doc, HTMLElement elem)  {
  doc->head_elems = realloc(doc->head_elems, ++doc->num_head * sizeof(HTMLElement));
  doc->head_elems[doc->num_head - 1] = elem;
}

void html_doc_add_body_elem(HTMLDocument* doc, HTMLElement elem)  {
  doc->body_elems = realloc(doc->body_elems, ++doc->num_body * sizeof(HTMLElement));
  doc->body_elems[doc->num_body - 1] = elem;
}

char* html_doc_create(HTMLDocument* doc, unsigned* sizeptr) {
  string_t* string = string_new(NULL, 2000);

  switch(doc->type) {
  case DOC_HTML4:
    string = string_append_str(string, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"" \
                               "\n\t\"http://www.w3.org/TR/html4/strict.dtd\">");
    break;
  case DOC_HTML5:
    string = string_append_str(string, "<!doctype html>\n<html>\n");
    break;
  case DOC_XHTML:
    string = string_append_str(string, "!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" \
                               "\n\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" \
                               "\n<html xmlns=\"http://www.w3.org/1999/xhtml\">");
    break;
  }

  /* head */

  string = string_append_str(string, "\t<head>\n");
  unsigned i;
  for(i = 0; i < doc->num_head; ++i) {
    HTMLElement e = doc->head_elems[i];
    unsigned size;
    
    char* str = html_elem_create(&e, &size);
    string = string_append_str(string, "\t\t");
    string = string_append_str(string, str);
    string = string_append_str(string, "\n");
    free(str);    
  }

  string = string_append_str(string, "\t</head>\n");

  /* body */
  string = string_append_str(string, "\t<body>\n");
  for(i = 0; i < doc->num_body; ++i) {
    HTMLElement e = doc->body_elems[i];
    unsigned size;
    
    char* str = html_elem_create(&e, &size);
    string = string_append_str(string, "\t\t");
    string = string_append_str(string, str);
    string = string_append_str(string, "\n");
    free(str);    
  }
  string = string_append_str(string, "\t</body>\n");


  string = string_append_str(string, "</html>");

  char* strptr = string->str;
  *sizeptr = string->size;
  free(string);
    
  return strptr;
}

/* HTMLElement functions */
HTMLElement html_elem_new(char* tag, element_flags flags) {
  HTMLElement elem;

  if(flags & ELEMENT_NULL_TAG) {
    elem.tag = NULL;
  } else {
    elem.tag = tag;
  }

  elem.flags = flags;

  elem.content_str = NULL;

  elem.num_attrs = 0;
  elem.attributes = malloc(0 * sizeof(char*));

  elem.num_content = 0;
  elem.content = malloc(0 * sizeof(HTMLElement));

  return elem;
}

void html_elem_destroy(HTMLElement* elem) {
  elem->num_attrs = elem->num_content = 0;

  free(elem->attributes);
  free(elem->content);

  if(elem->content_str) {
    free(elem->content_str);
  }

}

void html_elem_add_attr(HTMLElement* elem, char* attr) {
  elem->attributes = realloc(elem->attributes, (++elem->num_attrs * sizeof(char*)));
  elem->attributes[elem->num_attrs - 1] = attr;
}

void html_elem_add_elem(HTMLElement* elem, HTMLElement inner) {
  elem->content = realloc(elem->content, (++elem->num_content * sizeof(HTMLElement)));
  elem->content[elem->num_content - 1] = inner;
}

void html_elem_add_content(HTMLElement* elem, char* content) {
  HTMLElement e = html_elem_new(NULL, ELEMENT_NULL_TAG | ELEMENT_AUTO_FREE);

  html_elem_set_content(&e, content);
  html_elem_add_elem(elem, e);
}

void html_elem_set_content(HTMLElement* elem, char* content) {
  unsigned size = strlen(content);
  elem->content_str = malloc(size + 1);
  strcpy(elem->content_str, content);
}

char* html_elem_create(HTMLElement* elem, unsigned* sizeptr) {
  string_t* string = string_new(NULL, 100 * elem->num_content);

  if(!(elem->flags & ELEMENT_NULL_TAG)) {
    string = string_append_str(string, "<");
    string = string_append_str(string, elem->tag);

    unsigned i;
    for(i = 0; i < elem->num_attrs; ++i) {
      char* attr = elem->attributes[i];

      string = string_append_str(string, " ");
      string = string_append_str(string, attr);
    }
    
    if(elem->flags & ELEMENT_SELF_CLOSE) {
      string = string_append_str(string, " />");
    } else {
      string = string_append_str(string, ">");
    }
  }


  if(!(elem->flags & ELEMENT_SELF_CLOSE)) {
    if(elem->content_str != NULL) {
      string = string_append_str(string, elem->content_str);
    }

    unsigned i;
    for(i = 0; i < elem->num_content; ++i) {
      HTMLElement e = elem->content[i];
      unsigned size;
      char* inner = html_elem_create(&e, &size);
      string = string_append_str(string, inner);
      free(inner);
    }
    
    if(!(elem->flags & ELEMENT_NULL_TAG)) {
      string = string_append_str(string, "</");
      string = string_append_str(string, elem->tag);
      string = string_append_str(string, ">");
    }
  }

  char* strptr = string->str;
  *sizeptr = string->size;
  free(string);
  
  if(elem->flags & ELEMENT_AUTO_FREE) {
    html_elem_destroy(elem);
  }

  return strptr;
}


char* html_escape(char* text, unsigned* sizeptr) {
  string_t* string = string_new(NULL, strlen(text));

  unsigned i = 0;
  char c;
  while((c = text[i++])) {
    switch(c) {
    case '"':
      string = string_append_str(string, "&quot;");
      break;
    case '<':
      string = string_append_str(string, "&lt;");
      break;
    case '>':
      string = string_append_str(string, "&gt;");
      break;
    case '&':
      string = string_append_str(string, "&amp;");
      break;
    default: {
      char tmp[] = { c, '\0' };
      string = string_append_str(string, tmp);
      break;
    }
    }
  }

  char* strptr = string->str;
  *sizeptr = string->size;
  free(string);

  return strptr;
}
