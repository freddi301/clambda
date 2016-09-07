// Copyright 2016 <Frederik Batuna>

#include <iostream>
#include <string>
#include <map>

#include "gc.h"
#include "gc_cpp.h"
#include "gc_allocator.h"

class Term {
  public:
    virtual std::string pretty_print() const = 0;
};
std::ostream& operator<<(std::ostream &stream, const Term &t) {
  return stream << t.pretty_print();
}
std::ostream& operator<<(std::ostream &stream, const Term* t) {
  return stream << t->pretty_print();
}

class Identifier: public Term {
  public:
    const std::string name;
    Identifier(std::string name_): name(name_) {}
    std::string pretty_print() const { return name; }
};

class Application: public Term {
  public:
    Term* const left;
    Term* const right;
    Application(Term* left_, Term* right_): left(left_), right(right_) {}
    std::string pretty_print() const {
      return "(" + left->pretty_print() + " " + right->pretty_print() + ")";
    }
};

class Lambda: public Term {
  public:
    Identifier* const head;
    Term* const body;
    Lambda(Identifier* head_, Term* body_): head(head_), body(body_) {};
    std::string pretty_print() const {
      return "λ"+head->pretty_print()+"."+body->pretty_print();
    }
};

struct evaluated {
  Lambda* lambda;
  std::map<std::string, Lambda*> env;
};
std::ostream& operator<<(std::ostream &stream, const std::map<std::string, Lambda*> &env) {
  stream << "{";
  for (auto t : env) { stream << " " << t.first << " = " << t.second->pretty_print() << " "; };
  stream << "}";
  return stream;
}
std::ostream& operator<<(std::ostream &stream, const struct evaluated &e) {
  return stream << e.lambda << " " << e.env;
}

const struct evaluated evaluate(Term *ast, std::map<std::string, Lambda*> env) {
  if (Application* a = dynamic_cast<Application*>(ast)) {
    Lambda* const left = dynamic_cast<Lambda*>(a->left);
    Lambda* const right = dynamic_cast<Lambda*>(a->right);
    if (left && right) {
      env[left->head->name] = right;
      return evaluate(left->body, env);
    } else if (left) {
      return evaluate(new(GC) Application(left, evaluate(a->right, env).lambda), env);
    } else if (right) {
      auto r = evaluate(a->left, env);
      return evaluate(new(GC) Application(r.lambda, right), r.env);
    }
  }
  else if (Identifier* i = dynamic_cast<Identifier*>(ast)) {
    return {env.at(i->name), env};
  }
  else if (Lambda* l = dynamic_cast<Lambda*>(ast)) { return {l, env}; }
  throw std::invalid_argument( "invalid ast" );
}

Identifier* i(std::string name) { return new(GC) Identifier(name); }
Application* a(Term* left, Term* right) { return new(GC) Application(left, right); }
Lambda* l(Identifier* head, Term* body) { return new(GC) Lambda(head, body); }
const struct evaluated e(Term *ast) { return evaluate(ast, std::map<std::string, Lambda*>()); }

int main() {
  Lambda* Identity = l(i("x"), i("x"));
  std::cout << "Identity = " << Identity << std::endl;
  Term* test1 = a(Identity, Identity);
  std::cout << "I(I) = " << test1 << std::endl;
  std::cout << "I(I) => " << e(test1) << std::endl;
  Term* True = l(i("a"), l(i("b"), i("a")));
  Term* False = l(i("a"), l(i("b"), i("b")));
  std::cout << "True = " << True << std::endl;
  std::cout << "False = " << False << std::endl;
  std::cout << "(a=>b=>c=>a)(I)(T)(F) = " << e(a(a(a(l(i("a"), l(i("b"), l(i("c"), i("a")))), Identity), True), False)) << std::endl;
  std::cout << "(True(Identity))(False) => " << e(a(a(True, Identity), False)) << std::endl;
  return 0;
}
