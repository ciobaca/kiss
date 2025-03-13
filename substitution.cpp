#include "substitution.h"
#include "helper.h"
#include <cassert>

void Substitution::add(Variable *v, Term *t)
{
  (*this)[v] = t;
}

void Substitution::apply(Substitution &s)
{
  for (Substitution::iterator it = this->begin(); it != this->end(); ++it) {
    it->second = it->second->substitute(s);
  }
}

void Substitution::force(Variable *v, Term *t)
{
  Substitution temp;
  temp.add(v, t);
  apply(temp);
  add(v, t);
}

bool Substitution::inDomain(Variable *v)
{
  return contains(*this, v);
}

Term *Substitution::image(Variable *v)
{
  assert(contains(*this, v));
  return (*this)[v];
}
