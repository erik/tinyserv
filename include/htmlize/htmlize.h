#pragma once

#ifndef _HTML_H_
#define _HTML_H_

typedef enum doctype { 
  DOC_HTML4,
  DOC_HTML5,
  DOC_XHTML
} doctype;

typedef enum element_flags {
  ELEMENT_SELF_CLOSE = 1 << 0,
  ELEMENT_NULL_TAG   = 1 << 1,
  ELEMENT_AUTO_FREE  = 1 << 2
} element_flags;

typedef struct HTMLElement {
  char* tag;

  char** attributes;
  unsigned num_attrs;

  char* content_str;

  struct HTMLElement* content;
  unsigned num_content;

  element_flags flags;
} HTMLElement;

typedef struct HTMLDocument {
  doctype type;
  
  unsigned num_head;
  HTMLElement* head_elems;

  unsigned num_body;
  HTMLElement* body_elems;

} HTMLDocument;


/**************************
 * HTMLDocument functions *
 *************************/

/**
 * Creates a new HTMLDocument, with the doctype set 
 * to doc. Nothing special, just allocates and intializes
 * everything to sane values.
 */
HTMLDocument html_doc_new(doctype doc);

/**
 * Cleans up memory associated with doc. Note that if
 * doc is malloc()'d, html_doc_destroy will not free()
 * it.
 */
void html_doc_destroy(HTMLDocument* doc);

/**
 * Worthless function to change the doctype of doc
 */
void html_doc_set_doctype(HTMLDocument* doc, doctype type);

/**
 * Convenience function to set the title of doc to title.
 * Note that as this is a naive function, it simply calls
 * html_doc_add_head_elem, and as such, multiple calls
 * will create multiple titles
 */
void html_doc_set_title(HTMLDocument* doc, char* title);

/**
 * Appends an HTMLElement to the document, to be generated
 * in the <head> section
 */
void html_doc_add_head_elem(HTMLDocument* doc, HTMLElement elem);

/**
 * Appends an HTMLElement to the document, to be generated 
 * in the <body> section
 */
void html_doc_add_body_elem(HTMLDocument* doc, HTMLElement elem);

/**
 * Generates an HTML representation of the document, the 
 * value of sizeptr set to the length of the generated
 * string
 */
char* html_doc_create(HTMLDocument* doc, unsigned* sizeptr);


/*************************
 * HTMLElement functions *
 ************************/

/**
 * Creates a new HTMLElement with everything intialized and 
 * allocated
 */
HTMLElement html_elem_new(char* tag, element_flags flags);

/**
 * Cleans up memory associated with the element. Note that 
 * as with html_doc_destroy(), this will not free malloc()'d
 * memory
 */
void html_elem_destroy(HTMLElement* elem);

/**
 * Adds an attribute to the HTMLElement, such as "href=/.."
 * or "class=myclass".
 */
void html_elem_add_attr(HTMLElement* elem, char* attr);

/**
 * Adds an embedded element to the HTMLElement.
 */
void html_elem_add_elem(HTMLElement* elem, HTMLElement inner);

/**
 * Convenience function to create an element with the flags
 * ELMEMENT_AUTO_FREE | ELEMENT_NULL_TAG and adds it to the 
 * element. Note that since the element is auto-free'd, you 
 * need to be careful about how the parent HTMLElement will
 * be freed
 */
void html_elem_add_content(HTMLElement* elem, char* content);

/**
 * You should probably call html_elem_add_content() instead.
 */
void html_elem_set_content(HTMLElement* elem, char* content);

/**
 * Generates the HTML representation of the tag, with the value
 * of sizeptr set to the length of the string returned
 */
char* html_elem_create(HTMLElement* elem, unsigned* sizeptr);


/*********************
 * Utility functions *
 ********************/

/**
 * Returns an HTML-escaped representation of a given char* of raw input.
 * The value of *sizeptr after this function will be the length of the returned
 * string
 */
char* html_escape(char* raw, unsigned* sizeptr);

#endif /* _HTML_H_ */
