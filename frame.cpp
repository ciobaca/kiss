#include "frame.h"
#include "helper.h"
#include "term.h"
#include "name.h"
#include "knowledgebase.h"
#include "factories.h"

#include <iostream>

void Frame::add(Name *w, Term *t)
{
  push_back(make_pair(w, t));
}

vector<Name *> Frame::names()
{
  vector<Name *> result = this->bound;
  for (int i = 0; i < int(this->size()); ++i) {
    vector<Name *> temp = this->at(i).second->names();
    append(result, temp);
  }
  return result;
}

KnowledgeBase Frame::initial(vector<Function *> functions)
{
  KnowledgeBase knowledgeBase;

  for (int i = 0; i < len(functions); ++i) {
    knowledgeBase.addContextFact(functions[i]);
  }

  vector<Name *> freeNames;
  vector<Name *> names = this->names();
  for (int i = 0; i < len(names); ++i) {
    if (!contains(bound, names[i]) && !contains(freeNames, names[i])) {
      freeNames.push_back(names[i]);
    }
  }

  for (int i = 0; i < len(freeNames); ++i) {
    knowledgeBase.addClosedFact(getNamTerm(freeNames[i]), getNamTerm(freeNames[i]));
  }
  for (int i = 0; i < int(this->size()); ++i) {
    knowledgeBase.addClosedFact(getNamTerm(this->at(i).first), this->at(i).second);
  }

  //  cout << "Initial knowledge base: " << knowledgeBase << endl;

  return knowledgeBase;
}
