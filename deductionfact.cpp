#include "deductionfact.h"
#include "knowledgebase.h"
#include "varterm.h"
#include "term.h"
#include "factories.h"
#include "helper.h"
#include <cassert>
#include <sstream>

void DeductionFact::apply(Substitution &substTerms, Substitution &substRecipes)
{
  T = T->substitute(substTerms);
  for (int i = 0; i < len(sideConditions); ++i) {
    Variable *X = sideConditions[i].X;
    Term *t = sideConditions[i].t;
    sideConditions[i] = SideCondition(X, t->substitute(substTerms));
  }

  R = R->substitute(substRecipes);
}

vector<Variable *> DeductionFact::vars()
{
  vector<Variable *> result;

  append(result, R->vars());
  append(result, T->vars());
  for (int i = 0; i < len(sideConditions); ++i) {
    append(result, sideConditions[i].vars());
  }
  return unique(result);
}

#include <iostream>

void DeductionFact::combineInto(KnowledgeBase &knowledgeBase)
{
  //  cout << *this << endl;
  assert(this->isSolved());

  //  cout << "Combining with " << (*this) << endl;
  DeductionFact canonical = this->canonicalize();
  //  cout << "Canonical form " << canonical << endl;
  assert(canonical.isCanonical());

  Term *recipe = knowledgeBase.generates(canonical.T);
  if (recipe) {
    LOG(LOG_COMBINE, "Unuseful " + this->toString() + ";; term " + canonical.T->toString() + " has recipe " + recipe->toString());
    //    cout << "Is unuseful, other recipe is " << recipe->toString() << endl;
    Substitution subst;

    for (int i = 0; i < len(sideConditions); ++i) {
      subst.add(sideConditions[i].X, sideConditions[i].t);
    }
    EquationalFact equationalFact;
    equationalFact.U = R->substitute(subst);
    equationalFact.V = recipe;
    //    cout << "Adding equational fact " << equationalFact << endl;
    EquationalFact clashFree = equationalFact.rename(SMALL_SOLVED, BIG_SOLVED);
    LOG(LOG_COMBINE, "Adding equation " + clashFree.toString());
    //    cout << "Renamed to " << clashFree << endl;
    knowledgeBase.equationalFacts.push_back(clashFree);
    //    cout << "Done add" << endl;
  } else {
    LOG(LOG_COMBINE, "Useful " + this->toString());
    DeductionFact newFact = canonical.rename(SMALL_SOLVED, BIG_SOLVED);
    knowledgeBase.deductionFacts.push_back(newFact);
    //    cout << "Useful, added: " << newFact << endl;
  }
}

bool DeductionFact::isCanonical()
{
  assert(isSolved());
  for (int i = 0; i < len(sideConditions) - 1; ++i) {
    for (int j = i + 1; j < len(sideConditions); ++j) {
      if (sideConditions[i].t == sideConditions[j].t) {
	return false;
      }
    }
  }
  vector<Variable *> tvars = T->vars();
  vector<Variable *> svars;
  for (int i = 0; i < len(sideConditions); ++i) {
    append(svars, sideConditions[i].t->vars());
  }
  svars = unique(svars);
  return subseteq(tvars, svars) && subseteq(svars, tvars);
}

DeductionFact DeductionFact::canonicalize()
{
  vector<Variable *> vars = T->vars();

  Substitution substRecipe;
  vector<Variable *> big;
  vector<Variable *> small;
  for (int i = 0; i < len(sideConditions); ++i) {
    Variable *X = sideConditions[i].X;
    VarTerm *v = dynamic_cast<VarTerm *>(sideConditions[i].t);
    big.push_back(X);
    small.push_back(v->variable);
  }

  for (int i = 0; i < len(big) - 1; ++i) {
    for (int j = i + 1; j < len(big); ++j) {
      assert(big[i] != big[j]);
    }
  }

  Substitution subst;
  vector<SideCondition> newSideConditions;
  for (int i = 0; i < len(small); ++i) {
    if (!contains(vars, small[i])) {
      subst.add(big[i], getVarTerm(small[i]));
    } else  {
      int pos = -1;
      for (int j = 0; j < i; ++j) {
	if (small[j] == small[i]) {
	  pos = j;
	  break;
	}
      }
      if (pos != -1) {
	subst.add(big[i], getVarTerm(big[pos]));
      } else {
	newSideConditions.push_back(SideCondition(big[i], getVarTerm(small[i])));
      }
    }
  }

  DeductionFact result = *this;
  result.R = R->substitute(subst);
  result.sideConditions = newSideConditions;
  return result;
}

void DeductionFact::applyClashAvoider(Substitution &subst)
{
  R = R->substitute(subst);
  T = T->substitute(subst);

  for (int i = 0; i < len(sideConditions); ++i) {
    Variable *X = sideConditions[i].X->rename(subst);
    Term *t = sideConditions[i].t->substitute(subst);

    sideConditions[i] = SideCondition(X, t);
  }
}

void DeductionFact::solve(KnowledgeBase &knowledgeBase, int n)
{
  LOG(LOG_SOLVING, "Solving " + this->toString());

  if (this->isSolved()) {
    this->combineInto(knowledgeBase);
    return;
  }

  vector<Variable *> vars = this->vars();

  int pos = firstUnsolved();

  assert(pos < len(sideConditions));

  vector<Variable *> varspos = sideConditions[pos].t->vars();
  if (len(varspos) == 0) {
    Term *r = knowledgeBase.generates(sideConditions[pos].t);
    if (r) {
      Substitution substRecipes;
      substRecipes.add(sideConditions[pos].X, r);

      DeductionFact newFact;
      newFact.R = R;
      newFact.T = T;
      for (int i = 0; i < pos; ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
      }
      for (int i = pos + 1; i < len(sideConditions); ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
      }
      Substitution sigma;
      newFact.apply(sigma, substRecipes);
      DeductionFact clashfreeFact = newFact.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
      clashfreeFact.solve(knowledgeBase, n);
    }
    return;
  }

  for (int j = 0; j < n; ++j) {
    DeductionFact deductionFact = knowledgeBase.deductionFacts[j];
    assert(deductionFact.isSolved());
    assert(deductionFact.isCanonical());

    Substitution sigma;
    if (sideConditions[pos].t->unifyWith(deductionFact.T, sigma)) {
      Substitution substRecipes;
      substRecipes.add(sideConditions[pos].X, deductionFact.R);

      DeductionFact newFact;
      newFact.R = R;
      newFact.T = T;
      for (int i = 0; i < pos; ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
      }
      for (int i = 0; i < len(deductionFact.sideConditions); ++i) {
	newFact.sideConditions.push_back(deductionFact.sideConditions[i]);
      }
      for (int i = pos + 1; i < len(sideConditions); ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
      }
      newFact.apply(sigma, substRecipes);
      DeductionFact clashfreeFact = newFact.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
      clashfreeFact.solve(knowledgeBase, n);
    }
  }
}

DeductionFact DeductionFact::rename(string small, string big)
{
  assert(small != big);

  vector<Variable *> smallVars = this->T->vars();
  vector<Variable *> bigVars = this->R->vars();
  for (int i = 0; i < len(sideConditions); ++i) {
    append(smallVars, sideConditions[i].t->vars());
    bigVars.push_back(sideConditions[i].X);
  }

  map<Variable *, Variable *> rs = createRenaming(smallVars, small);
  map<Variable *, Variable *> rb = createRenaming(bigVars, big);

  Substitution smallSubst = createSubstitution(rs);
  Substitution bigSubst = createSubstitution(rb);

  DeductionFact result;
  result.T = T->substitute(smallSubst);
  result.R = R->substitute(bigSubst);
  for (int i = 0; i < len(sideConditions); ++i) {
    SideCondition sideCondition(rb[sideConditions[i].X],
				sideConditions[i].t->substitute(smallSubst));

    result.sideConditions.push_back(sideCondition);
  }

  return result;
}

ostream &DeductionFact::output(ostream &o)
{
  o << *this;
  return o;
}

ostream &operator<<(ostream &os, DeductionFact &deductionFact)
{
  os << "[ ";
  os << deductionFact.R->toString() << " > ";
  os << deductionFact.T->toString() << " | ";
  for (int i = 0; i < len(deductionFact.sideConditions); ++i) {
    os << deductionFact.sideConditions[i].X->name << " > ";
    os << deductionFact.sideConditions[i].t->toString();
    if (i != len(deductionFact.sideConditions) - 1) {
      os << ", ";
    }
  }
  os << " ]";
  return os;
}

string DeductionFact::toString()
{
  ostringstream oss;
  oss << (*this);
  return oss.str();
}
