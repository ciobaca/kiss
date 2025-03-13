#include "equationalfact.h"
#include "helper.h"
#include "knowledgebase.h"
#include <cassert>
#include <ostream>
#include <iostream>
#include <sstream>

void EquationalFact::apply(Substitution &substTerms, Substitution &substRecipes)
{
  for (int i = 0; i < len(sideConditions); ++i) {
    Variable *X = sideConditions[i].X;
    Term *t = sideConditions[i].t;
    sideConditions[i] = SideCondition(X, t->substitute(substTerms));
  }

  U = U->substitute(substRecipes);
  V = V->substitute(substRecipes);
}

vector<Variable *> EquationalFact::vars()
{
  vector<Variable *> result;

  append(result, U->vars());
  append(result, V->vars());
  for (int i = 0; i < len(sideConditions); ++i) {
    append(result, sideConditions[i].vars());
  }
  return unique(result);
}

void EquationalFact::combineInto(KnowledgeBase &knowledgeBase)
{
  assert(this->isSolved());

  knowledgeBase.equationalFacts.push_back(*this);
}

void EquationalFact::solve(KnowledgeBase &knowledgeBase)
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

      EquationalFact newFact;
      newFact.U = U;
      newFact.V = V;
      for (int i = 0; i < pos; ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
     }
      for (int i = pos + 1; i < len(sideConditions); ++i) {
	newFact.sideConditions.push_back(sideConditions[i]);
      }
      Substitution sigma;
      newFact.apply(sigma, substRecipes);
      EquationalFact clashfreeFact = newFact.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
      clashfreeFact.solve(knowledgeBase);
    } 
    return;
  }

  for (int j = 0; j < len(knowledgeBase.deductionFacts); ++j) {
    DeductionFact deductionFact = knowledgeBase.deductionFacts[j];
    assert(deductionFact.isSolved());
    assert(deductionFact.isCanonical());

    Substitution sigma;
    if (sideConditions[pos].t->unifyWith(deductionFact.T, sigma)) {
      Substitution substRecipes;
      substRecipes.add(sideConditions[pos].X, deductionFact.R);

      EquationalFact newFact;
      newFact.U = U;
      newFact.V = V;
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
      //      cout << "Solving: " << (*this) << endl;
      //      cout << "With: " << deductionFact << endl;
      EquationalFact clashfreeFact = newFact.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
      //      cout << "Yields: " << clashfreeFact << endl;
      clashfreeFact.solve(knowledgeBase);
    }
  }
}

EquationalFact EquationalFact::rename(string small, string big)
{
  assert(small != big);

  vector<Variable *> smallVars;
  vector<Variable *> bigVars = this->U->vars();
  append(bigVars, this->V->vars());
  for (int i = 0; i < len(sideConditions); ++i) {
    append(smallVars, sideConditions[i].t->vars());
    bigVars.push_back(sideConditions[i].X);
  }

  map<Variable *, Variable *> rs = createRenaming(smallVars, small);
  map<Variable *, Variable *> rb = createRenaming(bigVars, big);

  Substitution smallSubst = createSubstitution(rs);
  Substitution bigSubst = createSubstitution(rb);

  EquationalFact result;
  result.U = U->substitute(bigSubst);
  result.V = V->substitute(bigSubst);
  for (int i = 0; i < len(sideConditions); ++i) {
    SideCondition sideCondition(rb[sideConditions[i].X],
				sideConditions[i].t->substitute(smallSubst));

    result.sideConditions.push_back(sideCondition);
  }

  return result;
}

ostream &EquationalFact::output(ostream &os)
{
  os << *this;
  return os;
}

ostream &operator<<(ostream &os, EquationalFact &e)
{
  os << "[ ";
  os << e.U->toString() << " ~ ";
  os << e.V->toString() << " | ";
  for (int i = 0; i < len(e.sideConditions); ++i) {
    os << e.sideConditions[i].X->name << " > ";
    os << e.sideConditions[i].t->toString();
    if (i != len(e.sideConditions) - 1) {
      os << ", ";
    }
  }
  os << " ]";
  return os;
}


string EquationalFact::toString()
{
  ostringstream oss;
  oss << (*this);
  return oss.str();
}

bool EquationalFact::holdsIn(Frame *phi, RewriteSystem &rewrite)
{
  assert(this->isSolved());

  Substitution sigma;
  for (int i = 0; i < len(sideConditions); ++i) {
    sigma.add(sideConditions[i].X, sideConditions[i].t);
  }
  Term *A = U->substitute(sigma);
  Term *B = V->substitute(sigma);
  Term *Aphi = A->applyFrame(phi);
  Term *Bphi = B->applyFrame(phi);
//   int as = A->dagSize();
//   int bs = B->dagSize();
//   int aphis = Aphi->dagSize();
//   int bphis = Bphi->dagSize();
  //  cout << "As = " << as << " Bs = " << bs << " Aphis = " << aphis << " Bphis = " << bphis << endl;
//   cout << "testing equation " << (*this) << endl;
//   cout << "A = " << Aphi->normalize(rewrite)->toString() << endl;
//   cout << "B = " << Bphi->normalize(rewrite)->toString() << endl;

  Term *Anorm = Aphi->normalize(rewrite);
  Term *Bnorm = Bphi->normalize(rewrite);

  return Anorm == Bnorm;
}
