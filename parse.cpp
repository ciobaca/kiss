#include "parse.h"
#include "factories.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

void expected(string what, int &where)
{
  cout << "expected " << what << " at position " << where << endl;
  abort();
}

void expected(string what)
{
  cout << "expected " << what << endl;
  abort();
}

bool isIdentifierChar(char c)
{
  return c == '_' || isalpha(c) || isdigit(c);
}

string getIdentifier(string &s, int &pos)
{
  string id = "";
  if (pos < len(s) && isIdentifierChar(s[pos])) {
    while (pos < len(s) && isIdentifierChar(s[pos])) {
      id += s[pos];
      pos++;
    }
  } else {
    expected("identifier", pos);
  }
  return id;
}

int getNumber(string &s, int &pos)
{
  int result;
  if (pos >= len(s) || !isdigit(s[pos])) {
    expected("number");
  }
  string str;
  while (pos < len(s) && isdigit(s[pos])) {
    str += s[pos];
    pos++;
  }
  istringstream iss(str);
  iss >> result;
  return result;
}

void skipWhiteSpace(string &s, int &pos)
{
  while (pos < len(s) && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) {
    pos++;
  }
}

void matchString(string &s, int &pos, string what)
{
  for (int i = 0; i < len(what); ++i) {
    match(s, pos, what[i]);
  }
}

void match(string &s, int &pos, char c)
{
  if (pos < len(s) && s[pos] == c) {
    pos++;
  } else {
    ostringstream oss;
    oss << c;
    expected(oss.str(), pos);
  }
}

Term *parseTerm(string &s, int &pos)
{
  skipWhiteSpace(s, pos);
  string id = getIdentifier(s, pos);
  Function *f = getFunction(id);
  Variable *v = getVariable(id);
  Name *n = getName(id);

  if (f) {
    skipWhiteSpace(s, pos);
    vector<Term *> arguments;
    if (f->arity) {
      match(s, pos, '(');
      for (int i = 0; i < f->arity; ++i) {
	Term *t = parseTerm(s, pos);
	skipWhiteSpace(s, pos);
	if (i == f->arity - 1) {
	  match(s, pos, ')');
	} else {
	  match(s, pos, ',');
	}
	arguments.push_back(t);
      }
    }
    return getFunTerm(f, arguments);
  } else if (v) {
    return getVarTerm(v);
  } else if (n) {
    return getNamTerm(n);
  }
  cout << "this id: " << id << " is not a name, not a var, not a function " << endl;
  assert(0);
  return 0;
}

Term *parseTerm(string &s)
{
  int position = 0;
  return parseTerm(s, position);
}

Term *parseTerm(const char *s)
{
  string str(s);
  return parseTerm(str);
}
