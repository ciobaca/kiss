#ifndef REWRITE_H__
#define REWRITE_H__

#include <vector>
#include <string>

using namespace std;

struct Term;

struct RewriteSystem : public vector<pair<Term *, Term *> >
{
  void addRule(Term *l, Term *r);
  RewriteSystem rename(string);
};

#endif
