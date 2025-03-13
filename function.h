#ifndef FUNCTION_H__
#define FUNCTION_H__

#include <string>

using namespace std;

struct Function
{
  string name;
  int arity;

private:
  Function(string name, int arity)
  {
    this->name = name;
    this->arity = arity;
  }

  friend void createFunction(string, int);
};

#endif
