#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <vector>
#include <sstream>
#include <algorithm>
#include <getopt.h>

#include "helper.h"
#include "term.h"
#include "parse.h"
#include "knowledgebase.h"
#include "deductionfact.h"
#include "factories.h"
#include "frame.h"

static int sober_flag;

string sober(string s) 
{
  return sober_flag ? "" : s;
}

RewriteSystem rewrite;
map<string, Frame> frames;
map<string, KnowledgeBase> kbs;

void logmgu(string s, Term *a, Term *b, Substitution &subst)
{
#ifdef LOGMGU
  cout << s << " " << a->toString() << " and " << b->toString() << endl;
  cout << "subst = ";
  for (Substitution::iterator it = subst.begin(); it != subst.end(); ++it) {
    cout << it->first->name << "->" << it->second->toString() << "; ";
  }
  cout << endl;
#endif
}

void outputRewrite(RewriteSystem &rewriteSystem)
{
  for (int i = 0; i < len(rewriteSystem); ++i) {
    pair<Term *, Term *> rewriteRule = rewriteSystem[i];
    cout << rewriteRule.first->toString() << " " << rewriteRule.second->toString() << endl;
  }
}

void parseFunctions(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "signature");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string f = getIdentifier(s, w);
    skipWhiteSpace(s, w);
    match(s, w, '/');
    skipWhiteSpace(s, w);
    int arity = getNumber(s, w);
    createFunction(f, arity);
    skipWhiteSpace(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more function symbols");
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}

void parseNames(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "names");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string n = getIdentifier(s, w);
    createName(n);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more names");
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}

void parseVariables(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "variables");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string n = getIdentifier(s, w);
    createVariable(n);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more variables");
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}

void parseRewriteSystem(string &s, int &w, RewriteSystem &rewrite)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "rewrite");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    Term *t = parseTerm(s, w);
    skipWhiteSpace(s, w);
    matchString(s, w, "->");
    skipWhiteSpace(s, w);
    Term *tp = parseTerm(s, w);
    rewrite.addRule(t, tp);
    skipWhiteSpace(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more rewrite rules");
    }
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}


vector<Name *> parseNameList(string &s, int &w)
{
  vector<Name *> result;
  skipWhiteSpace(s, w);
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string n = getIdentifier(s, w);
    result.push_back(getName(n));
    if (w >= len(s) || (s[w] != ',' && s[w] != '.')) {
      expected("more names");
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, '.');
      break;
    }
  }
  return result;
}

void parseFrame(string &s, int &w)
{
  skipWhiteSpace(s, w);
  string id = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  matchString(s, w, "=");
  skipWhiteSpace(s, w);
  matchString(s, w, "new");
  skipWhiteSpace(s, w);
  vector<Name *> names = parseNameList(s, w);
  Frame frame;

  frame.bound = names;

  skipWhiteSpace(s, w);
  matchString(s, w, "{");
  if (w >= len(s)) {
    expected("frame content");
  }
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    string n = getIdentifier(s, w);
    Name *name = getName(n);
    assert(name);
    skipWhiteSpace(s, w);
    match(s, w, '=');
    skipWhiteSpace(s, w);
    Term *t = parseTerm(s, w);
    frame.add(name, t);
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, '}');
      break;
    }
  }
  frames[id] = frame;
  cout << "Saturating frame " << id << "..." << endl;
  vector<Function *> publicFunctions;
  extern map<string, Function *> functions;
  for (map<string, Function *>::iterator it =
	 functions.begin(); it != functions.end(); ++it) {
    publicFunctions.push_back(it->second);
  }
  kbs[id] = frame.initial(publicFunctions);
  kbs[id].saturate(rewrite);
  cout << "We have " << len(kbs[id].deductionFacts) << " deduction facts and " << len(kbs[id].equationalFacts) << " eq. facts." << endl;
}

void parseFrames(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "frames");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    parseFrame(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more frames");
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}

void parseEquivQuestion(string &s, int &w)
{
  matchString(s, w, "equiv");
  skipWhiteSpace(s, w);
  string id1 = getIdentifier(s, w);
  skipWhiteSpace(s, w);
  string id2 = getIdentifier(s, w);

  KnowledgeBase *kb1 = &kbs[id1];
  KnowledgeBase *kb2 = &kbs[id2];
  Frame *f1 = &frames[id1];
  Frame *f2 = &frames[id2];
  EquationalFact *counterExample1not2 = 0;
  EquationalFact *counterExample2not1 = 0;
//   for (int i = 0; i < len(kb1->equationalFacts); ++i) {
//     cout << "." << flush;
//     if (!kb1->equationalFacts[i].holdsIn(f1, rewrite)) {
//       cout << "Auch, unsound knowledge base for " << id1 << ": " << kb1->equationalFacts[i] << endl;
//     }
//   }
//   for (int i = 0; i < len(kb2->equationalFacts); ++i) {
//     cout << "," << flush;
//     if (!kb2->equationalFacts[i].holdsIn(f2, rewrite)) {
//       cout << "Auch, unsound knowledge base for " << id2 << ": " << kb2->equationalFacts[i] << endl;
//     }
//   }

  for (int i = 0; i < len(kb1->equationalFacts); ++i) {
    //    cout << ":" << flush;
    if (!kb1->equationalFacts[i].holdsIn(f2, rewrite)) {
      counterExample1not2 = &(kb1->equationalFacts[i]);
      break;
    }
  }
  for (int i = 0; i < len(kb2->equationalFacts); ++i) {
    //    cout << ";" << flush;
    if (!kb2->equationalFacts[i].holdsIn(f1, rewrite)) {
      counterExample2not1 = &(kb2->equationalFacts[i]);
      break;
    }
  }
  if (counterExample1not2 || counterExample2not1) {
    cout << sober("Sorry, ") << id1 << " is not statically equivalent to " << id2 << endl;
    if (counterExample1not2) {
      cout << "Here's an equation that holds in " << id1 << " but not in " << id2 << endl;
      cout << *counterExample1not2 << endl;
      if (!counterExample1not2->holdsIn(f1, rewrite)) {
	cout << "Auch, wrong example!" << endl;
      }
    }
    if (counterExample2not1) {
      cout << "Here's an equation that holds in " << id2 << " but not in " << id1 << endl;
      cout << *counterExample2not1 << endl;
      if (!counterExample2not1->holdsIn(f2, rewrite)) {
	cout << "Auch, wrong example!" << endl;
      }
    }
  } else {
    cout << sober("Yupii, ") << id1 << " is statically equivalent to " << id2 << endl;
  }
}

void parseDeductionQuestion(string &s, int &w)
{
  matchString(s, w, "deducible");
  skipWhiteSpace(s, w);
  Term *t = parseTerm(s, w)->normalize(rewrite);
  skipWhiteSpace(s, w);
  string id = getIdentifier(s, w);

  Frame f = frames[id];

  vector<Name *> names = t->names();
  vector<Name *> noneed = f.bound;
  append(noneed, f.names());
  vector<Name *> freeNames;
  for (int i = 0; i < len(names); ++i) {
    if (!contains(noneed, names[i])) {
      kbs[id].addClosedFact(getNamTerm(names[i]), getNamTerm(names[i]));
    }
  }
  Term *recipe = kbs[id].generates(t);
  if (recipe) {
    cout << sober("Yupii, ") << id << " |- " << t->toString() << " with recipe " << recipe->toString() << endl;
  } else {
    cout << sober("Sorry, ") << t->toString() << " is not deducible from " << id << endl;
  }
}

void parseKnowledgeQuestion(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "knowledgebase");
  skipWhiteSpace(s, w);
  string id = getIdentifier(s, w);
  cout << sober("Voila ") << "the saturated knowledge base of " << id << ": " << endl;
  cout << kbs[id] << endl << endl;
}

void parseQuestion(string &s, int &w)
{
  skipWhiteSpace(s, w);
  assert(w < len(s));
  if (s[w] == 'e') {
    parseEquivQuestion(s, w);
  } else if (s[w] == 'd') {
    parseDeductionQuestion(s, w);
  } else if (s[w] == 'k') {
    parseKnowledgeQuestion(s, w);
  } else {
    expected("a question");
  }
}

void parseQuestions(string &s, int &w)
{
  skipWhiteSpace(s, w);
  matchString(s, w, "questions");
  while (w < len(s)) {
    skipWhiteSpace(s, w);
    cout << endl;
    parseQuestion(s, w);
    skipWhiteSpace(s, w);
    if (w >= len(s) || (s[w] != ',' && s[w] != ';')) {
      expected("more questions");
    }
    skipWhiteSpace(s, w);
    if (s[w] == ',') {
      match(s, w, ',');
      continue;
    } else {
      match(s, w, ';');
      break;
    }
  }
}

int VERBOSITY;

int main(int argc, char **argv)
{
  static struct option long_options[] = {
    /* These options set a flag. */
    {"sober", no_argument,       &sober_flag, 1},
    /* These options don't set a flag.
       We distinguish them by their indices. */
    {"verbosity",  required_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  VERBOSITY = 0;
  while (1) {
    int option_index;
    int c = getopt_long(argc, argv, "v:", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 'v':
      VERBOSITY = atol(optarg);
      break;
    }
  }
  
  string s;
  int w = 0;

  while (!cin.eof()) {
    string tmp;
    getline(cin, tmp);
    s += tmp;
    s += " ";
  }

  parseFunctions(s, w);
  parseVariables(s, w);
  parseNames(s, w);
  parseRewriteSystem(s, w, rewrite);
  parseFrames(s, w);

  cout << endl << endl << endl;
  cout << "Answers to questions: " << endl << endl;
  parseQuestions(s, w);

  skipWhiteSpace(s, w);
  if (w < len(s)) {
    cout << "There is stuff after questions at the end, ignoring." << endl;
  }
}
