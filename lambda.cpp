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
std::ostream& operator<<(std::ostream &stream, const Term* const t) {
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
    const Term* const left;
    const Term* const right;
    Application(const Term* const left_, const Term* const right_): left(left_), right(right_) {}
    std::string pretty_print() const {
      return "(" + left->pretty_print() + " " + right->pretty_print() + ")";
    }
};

class Lambda: public Term {
  public:
    const Identifier* const head;
    const Term* const body;
    Lambda(const Identifier* const head_, const Term* const body_): head(head_), body(body_) {};
    std::string pretty_print() const {
      return "λ"+head->pretty_print()+"."+body->pretty_print();
    }
};

struct evaluated {
  const Lambda* const lambda;
  const std::map<std::string, const Lambda*> env;
};
std::ostream& operator<<(std::ostream &stream, const std::map<std::string, const Lambda*> &env) {
  stream << "{";
  for (auto t : env) { stream << " " << t.first << " = " << t.second->pretty_print() << " "; };
  stream << "}";
  return stream;
}
std::ostream& operator<<(std::ostream &stream, const struct evaluated &e) {
  return stream << e.lambda << " " << e.env;
}

const struct evaluated evaluate(const Term* const ast, std::map<std::string, const Lambda*> env) {
  if (const Application* a = dynamic_cast<const Application*>(ast)) {
    const Lambda* const left = dynamic_cast<const Lambda*>(a->left);
    const Lambda* const right = dynamic_cast<const Lambda*>(a->right);
    if (left && right) {
      env[left->head->name] = right;
      return evaluate(left->body, env);
    } else if (left) {
      return evaluate(new(GC) Application(left, evaluate(a->right, env).lambda), env);
    } else if (right) {
      auto r = evaluate(a->left, env);
      return evaluate(new(GC) Application(r.lambda, right), r.env);
    } else if (const Identifier* i = dynamic_cast<const Identifier*>(a->left)) {
      return evaluate(new(GC) Application(env.at(i->name), a->right), env);
    }
  }
  else if (const Identifier* i = dynamic_cast<const Identifier*>(ast)) {
    return {env.at(i->name), env};
  }
  else if (const Lambda* l = dynamic_cast<const Lambda*>(ast)) { return {l, env}; }
  throw std::invalid_argument( "invalid ast " + ast->pretty_print() );
}

Identifier* i(const std::string name) { return new(GC) Identifier(name); }
Application* a(const Term* const left, const Term* const right) { return new(GC) Application(left, right); }
Lambda* l(const Identifier* const head, const Term* const body) { return new(GC) Lambda(head, body); }
const struct evaluated e(const Term* const ast) { return evaluate(ast, std::map<std::string, const Lambda*>()); }

class Type {
  public:
    virtual std::string pretty_print() const = 0;
};
std::ostream& operator<<(std::ostream &stream, const Type &t) {
  return stream << t.pretty_print();
}
std::ostream& operator<<(std::ostream &stream, const Type* const t) {
  return stream << t->pretty_print();
}

class TypeHolder: public Type {
  public:
    const int name;
    TypeHolder(int name_): name(name_) {}
    std::string pretty_print() const { return std::to_string(name); }
};

class LambdaType: public Type {
  public:
    const Type* const head;
    const Type* const body;
    LambdaType(const Type* const head_, const Type* const body_): head(head_), body(body_) {};
    std::string pretty_print() const {
      return "("+head->pretty_print()+" -> "+body->pretty_print()+")";
    }
};

struct typechecked { const Type* const type; const int nextType; std::map<const Type*, const Type*> alias; };
std::ostream& operator<<(std::ostream &stream, const std::map<std::string, const Type*> &map) {
  stream << "{";
  for (auto t : map) { stream << " " << t.first << " := " << t.second->pretty_print() << " "; };
  stream << "}";
  return stream;
}
std::ostream& operator<<(std::ostream &stream, const std::map<const Type*, const Type*> &map) {
  stream << "{";
  for (auto t : map) { stream << " " << t.first->pretty_print() << " =:= " << t.second->pretty_print() << " "; };
  stream << "}";
  return stream;
}
std::ostream& operator<<(std::ostream &stream, const struct typechecked &t) {
  stream << t.type; if (t.alias.size()) { stream << " " << t.alias; }; return stream;
}

const Type* const recunalias(const Type* type, std::map<const Type*, const Type*> alias) {
  try {
    const Type* const unaliased = alias.at(type);
    if (const LambdaType* l = dynamic_cast<const LambdaType*>(unaliased)) {
      return new(GC) LambdaType(recunalias(l->head, alias), recunalias(l->body, alias));
    } return unaliased;
  } catch (std::out_of_range e) { return type; };
}
const Type* operator>>(std::map<const Type*, const Type*> alias, const Type* const type) {
  return recunalias(type, alias);
}

const struct typechecked
typecheck(
  const Term* const ast,
  const int nextType,
  std::map<std::string, const Type*> map,
  std::map<const Type*, const Type*> alias
) {
  if (const Application* a = dynamic_cast<const Application*>(ast)) {
    const Lambda* const left = dynamic_cast<const Lambda*>(a->left);
    const Lambda* const right = dynamic_cast<const Lambda*>(a->right);
    if (left && right) {
      const struct typechecked tc = typecheck(right, nextType, map, alias); alias = tc.alias;
      map[left->head->name] = tc.type;
      return typecheck(left->body, tc.nextType, map, alias);
    } else {
      const struct typechecked tcr = typecheck(a->right, nextType, map, alias); alias = tcr.alias;
      const TypeHolder* const rt = new(GC) TypeHolder(tcr.nextType);
      const struct typechecked tcl = typecheck(a->left, tcr.nextType+1, map, alias); alias = tcl.alias;
      if (const TypeHolder* th = dynamic_cast<const TypeHolder*>(tcl.type)) {
        alias[th] = new(GC) LambdaType(tcr.type, rt);
      }
      return { rt, tcl.nextType, alias };
    }
  }
  else if (const Identifier* i = dynamic_cast<const Identifier*>(ast)) {
    return { map.at(i->name), nextType, alias };
  }
  else if (const Lambda* l = dynamic_cast<const Lambda*>(ast)) {
    const TypeHolder* const newHolder = new(GC) TypeHolder(nextType);
    map[l->head->name] = newHolder;
    const struct typechecked tc = typecheck(l->body, nextType+1, map, alias); alias = tc.alias;
    return { new(GC) LambdaType(alias>>newHolder, tc.type), tc.nextType, alias };
  }
  throw std::invalid_argument( "invalid type ast" + ast->pretty_print() );
}

const struct typechecked t(const Term* const ast) {
  return typecheck(ast, 1, std::map<std::string, const Type*>(), std::map<const Type*, const Type*>());
}
