#pragma once

#include "BasicLexer.h"
#include "BasicParser.h"
#include "value.h"
#include "core/stl.h"
#include "fmt/format.h"

#include <any>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace wwivbasic {

class Var {
public:
  Var(const std::string& n, const Value& v) : name(n), value(v) {}

  std::string name;
  Value value;
};

class Scope {
public:
  Scope(const std::string& name) : fn_name(name) {}
  std::string fn_name;
  std::map<std::string, Var, wwiv::stl::ci_less> local_vars;
};

typedef std::function<Value(std::vector<Value>)> basic_function_fn;

class BasicFunction {
public:
  enum class Type { NATIVE, BASIC };

  BasicFunction(const std::string& n, BasicParser::ProcedureDefinitionContext* fn,
                const std::vector<std::string>& p)
      : name(n), type(Type::BASIC), def_fn(fn), params(p) {}

  BasicFunction(const std::string& n, const basic_function_fn& fn,
                const std::vector<std::string>& p)
      : name(n), type(Type::NATIVE), cpp_fn(fn), params(p) {}

  std::string name;
  Type type{Type::BASIC};
  BasicParser::ProcedureDefinitionContext* def_fn{nullptr};
  basic_function_fn cpp_fn;
  std::vector<std::string> params;
};

#define REGISTER_NATIVE(module, func)                                                              \
  do {                                                                                             \
    module->native_functionl(#func, make_basic_fn(func));                                           \
  } while (0)

template<class T>
struct AsFunction
  : public AsFunction<decltype(&T::operator())>
{};

template<class ReturnType, class... Args>
struct AsFunction<ReturnType(Args...)> {
  using type = std::function<ReturnType(Args...)>;
};

template<class ReturnType, class... Args>
struct AsFunction<ReturnType(*)(Args...)> {
  using type = std::function<ReturnType(Args...)>;
};

template<class Class, class ReturnType, class... Args>
struct AsFunction<ReturnType(Class::*)(Args...) const> {
  using type = std::function<ReturnType(Args...)>;
};

// makes various types of functions into a std::function
template<class F>
auto as_fn(F f) -> typename AsFunction<F>::type {
  return { f };
}

template<typename F, typename R, typename P1>
basic_function_fn make_basic_fn_(std::function<R(P1)> f) {
  basic_function_fn f1 = [=](std::vector<Value> params) -> Value {
    if (params.size() < 1) {
      return Value(false); // ERROR
    }
    return Value(f(params.at(0).get<P1>()));
  };
  return f1;
}

template<typename F, typename R, typename P1, typename P2>
basic_function_fn make_basic_fn_(std::function<R(P1, P2)> f) {
  basic_function_fn f1 = [=](std::vector<Value> params) -> Value {
    if (params.size() < 2) {
      return Value(false); // ERROR
    }
    return Value(f(params.at(0).get<P1>(), params.at(1).get<P2>()));
  };
  return f1;
}

template<class F>
basic_function_fn make_basic_fn(F f) {
  return make_basic_fn_<F>(as_fn(std::forward<F>(f)));
}


class ExecutionVisitor;

class Module {
public:
  Module(const std::string& name) : name_(name) {}

  // Creates a variable at the top scope or updates existing variable.
  void upsert(const std::string& name, const Value& value);
  // Gets the value of a variable.
  Value var(const std::string& name);
  // Calls a function
  Value call(const std::string& function_name, const std::vector<Value>& params,
    ExecutionVisitor* visitor);

  bool has_var(const std::string& name) const;
  bool has_fn(const std::string& name) const;

  void native_functionl(const std::string& name, const basic_function_fn& fn,
    const std::vector<std::string>& params) {
    functions.insert_or_assign(name, BasicFunction(name, fn, params));
  }

  void native_functionl(const std::string& name, const basic_function_fn& fn) {
    std::vector<std::string> v;
    native_functionl(name, fn, v);
  }

  template<class F>
  void native_function(const std::string& name, F f, const std::vector<std::string>& params) {
    functions.insert_or_assign(name, BasicFunction(name, make_basic_fn_<F>(as_fn(std::forward<F>(f))), params));
  }

  template<class F>
  void native_function(const std::string& name, F f) {
    std::vector<std::string> params;
    //native_functionl(name, make_basic_fn_<F>(as_fn(std::forward<F>(f))), params);
    functions.insert_or_assign(name, BasicFunction(name, make_basic_fn_<F>(as_fn(std::forward<F>(f))), params));
  }


  std::deque<Scope> scopes;
  std::map<std::string, BasicFunction, wwiv::stl::ci_less> functions;

private:
  std::string name_;
};

class SourceUnit;

class BasicParserErrorListener : public antlr4::BaseErrorListener {
public:
  BasicParserErrorListener(SourceUnit* su) : su_(su) {}
  void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line,
    size_t charPositionInLine, const std::string& msg,
    std::exception_ptr e) override;
  SourceUnit* su_{ nullptr };
};

class SourceUnit {
public:
  SourceUnit(const std::string& filename, const std::string& text)
      : filename_(filename), text_(text), input_(text), lexer_(&input_), tokens_(&lexer_),
        parser_(&tokens_), parserError_(this) {
    parser_.removeErrorListeners();
    parser_.addErrorListener(&parserError_);
    tree_ = parser_.main();
  }

  // Gets the parse tree for this source unit.
  antlr4::tree::ParseTree* tree() { return tree_; }
  BasicParser* parser() { return &parser_; }

  std::string filename_;
  std::string text_;
  antlr4::ANTLRInputStream input_;
  BasicLexer lexer_;
  antlr4::CommonTokenStream tokens_;
  BasicParser parser_;
  antlr4::tree::ParseTree* tree_{nullptr};
  BasicParserErrorListener parserError_;
  std::vector<std::string> errors;
};

class Context {
public:
  Context(const std::filesystem::path& path);
  Context();

  // Creates a variable at the top scope or updates existing variable.
  void upsert(const std::string& name, const Value& value);
  // Gets the value of a variable.
  Value var(const std::string& name);
  // Calls a function
  Value call(const std::string& function_name, const std::vector<Value>& params,
    ExecutionVisitor* visitor);

  bool add_source(const std::filesystem::path& path, const std::string& text) {
    auto su = std::make_unique<SourceUnit>(path.string(), text);
    if (!su->errors.empty()) {
      errors.insert(std::end(errors), std::begin(su->errors), std::end(su->errors));
    }
    sources.insert_or_assign(path, std::move(su));
    return true;
  }

  bool add_source(const std::filesystem::path& path);

  // Gets the parse tree for some unit
  std::optional<antlr4::tree::ParseTree*> parseTree(const std::filesystem::path& filename) {
    if (!wwiv::stl::contains(sources, filename)) {
      return std::nullopt;
    }
    auto& su = sources.at(filename);
    if (!su->errors.empty()) {
      for (const auto& err : su->errors) {
        fmt::print("ERROR: {}\r\n", err);
      }
      return std::nullopt;
    }
    return su->tree();
  }

  // list of modules currently imported using "IMPORT @module"
  std::set<std::string, wwiv::stl::ci_less> imported_modules;
  // All registered modules
  std::map<std::string, Module, wwiv::stl::ci_less> modules;
  std::map<std::filesystem::path, std::unique_ptr<SourceUnit>> sources;
  Module* root{ nullptr };
  Module* module{ nullptr };
  std::vector<std::string> errors;
};

} // namespace wwivbasic