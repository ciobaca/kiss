#ifndef FRAME_H__
#define FRAME_H__

#include <vector>

struct Term;
struct Name;
struct Function;
struct KnowledgeBase;

using namespace std;

struct Frame : vector<pair<Name *, Term *> >
{
  vector<Name *> bound;

  void add(Name *, Term *);
  vector<Name *> names();
  KnowledgeBase initial(vector<Function *>);
};

#endif
