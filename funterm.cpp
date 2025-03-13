#include "funterm.h"
#include "varterm.h"
#include "namterm.h"
#include "factories.h"
#include "knowledgebase.h"
#include "helper.h"
#include <sstream>
#include <cassert>
#include <iostream>

using namespace std;

FunTerm::FunTerm(Function *function, vector<Term *> arguments)
{
  this->function = function;
  this->arguments = arguments;
}

vector<Variable *> FunTerm::computeVars()
{
  vector<Variable *> result;
  for (int i = 0; i < len(arguments); ++i) {
    vector<Variable *> temp = arguments[i]->vars();
    append(result, temp);
  }
  return result;
}

vector<Name *> FunTerm::names()
{
  vector<Name *> result;
  for (int i = 0; i < len(arguments); ++i) {
    vector<Name *> temp = arguments[i]->names();
    append(result, temp);
  }
  return result;
}

string FunTerm::toString()
{
  assert(function->arity == len(arguments));
  int n = function->arity;

  ostringstream oss;
  oss << function->name;
  if (n) {
    oss << "(";
  }
  for (int i = 0; i < n; ++i) {
    oss << arguments[i]->toString() << (i == n - 1 ? ")" : ",");
  }
  return oss.str();
}

bool FunTerm::computeIsNormalized(RewriteSystem &rewriteSystem, map<Term *, bool> &cache)
{
  if (!contains(cache, (Term *)this)) {
    for (int i = 0; i < len(rewriteSystem); ++i) {
      Term *l = rewriteSystem[i].first;
      Substitution subst;
      if (this->isInstanceOf(l, subst)) {
	return cache[this] = false;
      }
    }
    for (int i = 0; i < len(arguments); ++i) {
      if (!arguments[i]->isNormalized(rewriteSystem)) {
	return cache[this] = false;
      }
    }
    cache[this] = true;
  }
  return cache[this];
}

Term *FunTerm::computeSubstitution(Substitution &subst, map<Term *, Term *> &cache)
{
  if (!contains(cache, (Term *)this)) {
    vector<Term *> newargs;
    for (int i = 0; i < len(arguments); ++i) {
      newargs.push_back(arguments[i]->computeSubstitution(subst, cache));
    }
    cache[this] = getFunTerm(function, newargs);
  }
  return cache[this];
}

Term *FunTerm::computeNormalize(RewriteSystem &rewriteSystem, map<Term *, Term *> &cache)
{
  if (!contains(cache, (Term *)this)) {
    vector<Term *> subterms;
    for (int i = 0; i < function->arity; ++i) {
      subterms.push_back(arguments[i]->computeNormalize(rewriteSystem, cache));
    }
    Term *result = getFunTerm(function, subterms);
    bool done = false;
    while (!done) {
      done = true;
      for (int i = 0; i < len(rewriteSystem); ++i) {
	pair<Term *, Term *> rewriteRule = rewriteSystem[i];
	Term *l = rewriteRule.first;
	Term *r = rewriteRule.second;

	Substitution subst;
	if (result->isInstanceOf(l, subst)) {
	  result = r->substitute(subst);
	  done = false;
	}
      }
    }
    cache[this] = result;
    assert(result->isNormalized(rewriteSystem));
  }
  return cache[this];
}

bool FunTerm::unifyWith(Term *t, Substitution &subst)
{
  logmgu("FunTerm::unifyWith", this, t, subst);
  return t->unifyWithFunTerm(this, subst);
}

bool FunTerm::unifyWithVarTerm(VarTerm *t, Substitution &subst)
{
  logmgu("FunTerm::unifyWithVarTerm", this, t, subst);
  return t->unifyWithFunTerm(this, subst);
}

bool FunTerm::unifyWithFunTerm(FunTerm *t, Substitution &subst)
{
  logmgu("FunTerm::unifyWithFunTerm", this, t, subst);
  if (this->function == t->function) {
    for (int i = 0; i < len(arguments); ++i) {
      if (!this->arguments[i]->unifyWith(t->arguments[i], subst)) {
	return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

bool FunTerm::unifyWithNamTerm(NamTerm *t, Substitution &subst)
{
  logmgu("FunTerm::unifyWithNamTerm", this, t, subst);
  return false;
}

bool FunTerm::isVariable()
{
  return false;
}

vector<pair<Term *, Term *> > FunTerm::split()
{
  vector<pair<Term *, Term *> > result;

  result.push_back(make_pair(getVarTerm(getVariable("\\_")), this));
  for (int i = 0; i < len(arguments); ++i) {
    vector<pair<Term *, Term *> > temp = arguments[i]->split();
    for (int j = 0; j < len(temp); ++j) {
      Term *context = temp[j].first;
      Term *hole = temp[j].second;

      vector<Term *> newArguments;
      for (int k = 0; k < i; ++k) {
	newArguments.push_back(arguments[k]);
      }
      newArguments.push_back(context);
      for (int k = i + 1; k < len(arguments); ++k) {
	newArguments.push_back(arguments[k]);
      }
      result.push_back(make_pair(getFunTerm(function, newArguments), hole));
    }
  }
  return result;
}

Term *FunTerm::generatedBy(KnowledgeBase &knowledgeBase, map<Term *, Term *> &cache)
{
  if (!contains(cache, dynamic_cast<Term *>(this))) {
    Term *result = 0;
    for (int i = 0; i < len(knowledgeBase.deductionFacts); ++i) {
      DeductionFact fact = knowledgeBase.deductionFacts[i];
      assert(fact.isSolved());
      assert(fact.isCanonical());

      vector<Term *> recipes;
      Substitution sigma;
      if (this->isInstanceOf(fact.T, sigma)) {
	assert(fact.T->substitute(sigma) == this);
	bool ok = true;
	for (int i = 0; i < len(fact.sideConditions); ++i) {
	  Term *what = fact.sideConditions[i].t->substitute(sigma);
	  Term *recipe = what->generatedBy(knowledgeBase, cache);
	  if (!recipe) {
	    ok = false;
	    break;
	  }
	  recipes.push_back(recipe);
	}
	if (ok) {
	  Substitution substRecipe;
	  for (int i = 0; i < len(fact.sideConditions); ++i) {
	    substRecipe.add(fact.sideConditions[i].X, recipes[i]);
	  }
	  result = fact.R->substitute(substRecipe);
	  break;
	}
      }
    }
    cache[this] = result;
  }
  return cache[this];
}

bool FunTerm::computeIsInstanceOf(Term *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  return t->computeIsGeneralizationOf(this, s, cache);
}

bool FunTerm::computeIsGeneralizationOf(NamTerm *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  return false;
}

bool FunTerm::computeIsGeneralizationOf(VarTerm *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  return false;
}

bool FunTerm::computeIsGeneralizationOf(FunTerm *t, Substitution &s, map<pair<Term *, Term *>, bool> &cache)
{
  if (!contains(cache, make_pair((Term *)t, (Term *)this))) {
    if (this->function != t->function) {
      cache[make_pair(t, this)] = false;
    } else {
      bool result = true;
      for (int i = 0; i < len(t->arguments); ++i) {
	if (!t->arguments[i]->computeIsInstanceOf(arguments[i], s, cache)) {
	  result = false;
	  break;
	}
      }
      cache[make_pair(t, this)] = result;
      if (result) {
	assert(this->substitute(s) == t);
      }
    }
  }
  return cache[make_pair(t, this)];
}

Term *FunTerm::computeApplyFrame(Frame *phi, map<Term *, Term *> &cache)
{
  if (!contains(cache, (Term *)this)) {
    vector<Term *> newargs;
    
    for (int i = 0; i < len(arguments); ++i) {
      newargs.push_back(arguments[i]->computeApplyFrame(phi, cache));
    }
    cache[this] = getFunTerm(function, newargs);
  }
  return cache[this];
}

int FunTerm::computeDagSize(map<Term *, int> &cache)
{
  if (contains(cache, (Term *)this)) {
    return 0;
  } else {
    int result = 1;
    for (int i = 0; i < len(arguments); ++i) {
      result += arguments[i]->computeDagSize(cache);
    }
    cache[this] = result;
    return result;
  }
}
