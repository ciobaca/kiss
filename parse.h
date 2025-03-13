#ifndef PARSE_H__
#define PARSE_H__

#include <string>
#include "term.h"

using namespace std;

Term *parseTerm(string &s);
Term *parseTerm(const char *s);
void matchString(string &s, int &pos, string what);
void match(string &s, int &pos, char c);
void skipWhiteSpace(string &s, int &pos);
string getIdentifier(string &s, int &pos);
int getNumber(string &s, int &pos);
Term *parseTerm(string &s, int &pos);
void expected(string what, int &where);
void expected(string what);

#endif
