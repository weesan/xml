%{

/*
 * To compile, do the following:
 *   $ flex -t scanner.l > scanner.cc
 *   $ g++ -g -std=c++11 scanner.cc -o scanner -lfl
 *
 * To run, do the following:
 *   $ scanner < foo.xml
 *
 * An example of an XML file:
 *
 * <?xml version="1.0" encoding="UTF-8"?>
 * <mediawiki xmlns="http://www.mediawiki.org/xml/export-0.10/" ...>
 *   <siteinfo>
 *     <sitename>Wikipedia</sitename>
 *     ...
 *   </siteinfo>
 *   <page>
 *     ...
 *   </page>
 * </mediawiki>
 */

#include <stdio.h>
#include <assert.h>
#include <unordered_map>
#include "xml.h"

using namespace std;

#define YY_DECL int parseXml(Xml &xml)

static unsigned long long offset = 0;

static void
handle_start_tag (Xml &xml, char *tag, unsigned long long offset)
{
    if (xml.debug()) {
        fprintf(stderr, "Handled start tag: %s\n", tag);
    }

    int count = 0;

    char *attr = strchr(tag, ' ');
    if (attr) {
	*attr++ = 0;
    }

    // Create and add a new child tag to the current element.
    Element &e = xml.addChild(tag, offset - strlen(tag) - 2);

    // Make the callback if provided.
    xml.startTag();

    if (!attr) {
        return;
    }

    // Parse the attributes if any.
    char key[256], value[8192];
    while (attr) {
        if (sscanf(attr, "%[^=]=\"%[^\"]", key, value) == 2) {
            // Skip after the first "
            attr = strchr(attr, '"');
            attr++;
            // Skip after the second "
            attr = strchr(attr, '"');
            attr++;
            // Skip the spaces.
            while (attr && *attr == ' ') attr++;
        } else {
            if (sscanf(attr, "%[^=]=%s", key, value) == 2) {
                // Skip after the space.
                attr = strchr(attr, ' ');
                // Skip the spaces.
                while (attr && *attr == ' ') attr++;
            } else {
                // Malform attribute?
                break;
            }
        }
        e.attrs().push_back(make_pair(key, value));
        if (xml.debug()) {
            fprintf(stderr, "Attr: %s=%s\n", key, value);
        }
    }
}

static void
handle_end_tag (Xml &xml, char *tag, unsigned long long offset) {
    if (xml.debug()) {
        fprintf(stderr, "Handled close tag: %s\n", tag);
    }

    xml.updateLength(offset);
    xml.endTag();
    xml.up();
}

%}

/*
%option prefix="xml"
%option yyclass="Xml"
*/

%x  TAG
%x  CONTENT
%x  CDATA
ws  [ \t]+

%%

"<" {
    offset++;
    BEGIN TAG;
}

<TAG>[^>]+> {
    int len = strlen(yytext);
    offset += len;

    // Get rid of the last char, ie. '>'
    yytext[--len] = 0;

    // Check for <?, </, etc.
    switch (yytext[0]) {
    case '?':
        if (xml.debug()) {
            fprintf(stderr, "Meta tag: [%s]\n", yytext);
        }
	xml.reset();
	BEGIN INITIAL;
	break;
    case '/': {
        if (xml.debug()) {
            fprintf(stderr, "End tag: [%s]\n", yytext);
        }

        handle_end_tag(xml, yytext + 1, offset);
	BEGIN INITIAL;
	break;
    }
    default:
	if (yytext[len - 1] == '/') {
            if (xml.debug()) {
                fprintf(stderr, "Empty-element tag  : [%s]\n", yytext);
            }
            yytext[--len] = 0;

            // Since an empty-element tag is both a start and end tag,
            // we will call both handlers.

	    handle_start_tag(xml, yytext, offset);
            handle_end_tag(xml, yytext, offset);
	    BEGIN INITIAL;
	} else {
	    handle_start_tag(xml, yytext, offset);
	    BEGIN CONTENT;
	}
	break;
    }
}

<CONTENT>"<" {
    offset++;
    BEGIN TAG;
}

<CONTENT>[^<]+ {
    offset += strlen(yytext);

    char *s = yytext;
    while (*s && (*s == ' ' || *s == '\n')) s++;
    if (*s) {
        if (xml.debug()) {
            fprintf(stderr, "Content:     [%s]\n", yytext);
        }
	xml.addContent(yytext);
    }
    BEGIN INITIAL;
}

<CONTENT>"<![CDATA[" {
    offset += strlen(yytext);

    xml.addContent(yytext);
    BEGIN CDATA;
}

<CDATA>.|\n {
    offset++;
    xml.appendContent(yytext);
}

<CDATA>"]]>" {
    offset += strlen(yytext);

    xml.appendContent(yytext);
    BEGIN INITIAL;
}

{ws} {
    // Ignore white spaces
    offset += strlen(yytext);
}

.|\n {
    offset++;
}

%%

int yywrap(void) {
  return 1;
}
