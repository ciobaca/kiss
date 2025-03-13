#include "term.h"

vector<Variable *> Term::vars() 
{
  if (computedVars) {
    return allVars;
  } else {
    computedVars = true;
    return allVars = computeVars();
  }
}

Term *Term::substitute(Substitution &subst)
{
  map<Term *, Term *> cache;
  return computeSubstitution(subst, cache);
}

bool Term::isNormalized(RewriteSystem &rewriteSystem)
{
  map<Term *, bool> cache;
  return computeIsNormalized(rewriteSystem, cache);
}

Term *Term::normalize(RewriteSystem &rewriteSystem)
{
  map<Term *, Term *> cache;
  return computeNormalize(rewriteSystem, cache);
}

bool Term::isInstanceOf(Term *t, Substitution &s)
{
  map<pair<Term *, Term *>, bool> cache;
  return computeIsInstanceOf(t, s, cache);
}

int Term::dagSize()
{
  map<Term *, int> cache;
  return computeDagSize(cache);
}

Term *Term::applyFrame(Frame *phi)
{
  map<Term *, Term *> cache;
  return computeApplyFrame(phi, cache);
}

