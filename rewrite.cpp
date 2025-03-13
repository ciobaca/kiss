#include "rewrite.h"
#include "term.h"
#include "substitution.h"
#include "variable.h"
#include <cassert>
#include <string>

using namespace std;

void RewriteSystem::addRule(Term *l, Term *r)
{
  assert(subseteq(r->vars(), l->vars()));
  this->push_back(make_pair(l, r));
}

RewriteSystem RewriteSystem::rename(string s)
{
  vector<Variable *> vars;
  for (int i = 0; i < (int)this->size(); ++i) {
    append(vars, (*this)[i].first->vars());
    append(vars, (*this)[i].second->vars());
  }

  map<Variable *, Variable *> r = createRenaming(vars, s);
  Substitution subst = createSubstitution(r);

  RewriteSystem result;
  for (int i = 0; i < (int)this->size(); ++i) {
    Term *l = (*this)[i].first;
    Term *r = (*this)[i].second;
    result.push_back(make_pair(l->substitute(subst), r->substitute(subst)));
  }

  return result;
}
