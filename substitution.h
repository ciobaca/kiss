#ifndef SUBSTITUTION_H__
#define SUBSTITUTION_H__

#include <map>
#include "term.h"

using namespace std;

struct Substitution : public map<Variable *, Term *>
{
  void apply(Substitution &);
  void force(Variable *v, Term *t);
  void add(Variable *v, Term *t);
  bool inDomain(Variable *v);
  Term *image(Variable *v);
};

#endif
