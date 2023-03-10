#include "context.h"
#include "core/textfile.h"
#include "core/stl.h"
#include "executor.h"
#include "utils.h"
#include "fmt/format.h"
#include "stdlib/common.h"
#include "stdlib/numbers.h"
#include "stdlib/strings.h"

#include <any>
#include <map>
#include <stack>
#include <string>
#include <vector>

using namespace wwiv::stl;

namespace wwivbasic {

void Module::upsert(const std::string& name, const Value& value) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      auto& var = it->local_vars.at(name);
      var.value(value);
      // updated existing.
      return;
    }
  }

  auto& scope = scopes.back();
  scope.local_vars.emplace(name, Var(name, value));
}

std::optional<Var> Module::var(const std::string& name) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      auto& var = it->local_vars.at(name);
      fmt::print("Found Var: {}={} at scope: {}\n", name, var.value(), it->fn_name);
      return var;
    }
  }
  std::cout << "UNKNOWN VARIABLE REFERENCED: " << name << std::endl;
  return std::nullopt;
}

bool Module::has_var(const std::string& name) const {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      return true;
    }
  }
  return false;
}

bool Module::has_fn(const std::string& name) const {
  return contains(functions, name);
}

Value Module::call(const std::string& function_name, const std::vector<Value>& params,
                             ExecutionVisitor* visitor) {
  if (!contains(functions, function_name)) {
    std::cout << "Unknown function: " << function_name << std::endl;
    return Value(false);
  }

  auto& fn = functions.at(function_name);

  // Validate params
  const auto want_count = fn.params.size();
  const auto have_count = params.size();
  if (fn.type == BasicFunction::Type::BASIC && have_count != want_count) {
    // Only bail on wrong args on BASIC functions.
    std::cout << "Wrong number of parameter to function: " << function_name << std::endl;
    std::cout << "have: " << have_count << std::endl;
    std::cout << "want: " << want_count << std::endl;
    std::cout << std::endl;
    return Value(false);
  }

  // Create a new scope for the function body
  Scope fnscope(function_name);

  // Add variables for parameters
  if (fn.type == BasicFunction::Type::BASIC) {
    for (int i = 0; i < want_count; i++) {
      const auto& n = fn.params.at(i);
      const auto& v = params.at(i);
      fnscope.local_vars.insert_or_assign(n, Var(n, v));
    }
  }

  // Put the scope on top fo the stack
  scopes.push_back(fnscope);

  Value result;
  if (fn.type == BasicFunction::Type::BASIC) {
    // Visit the body of the function call
    result = Value(visitor->visit(fn.def_fn->statements()));
  } else if (fn.type == BasicFunction::Type::NATIVE) {
    result = fn.cpp_fn(params);
  }

  fmt::print("{} RETURNED: '{}'\n", fn.name, Value(result));

  // remove latest scope.
  scopes.pop_back();
  return result;
}

Context::Context() {
  // Start off with only global scope
  modules.emplace("", Module("") );
  modules.at("").scopes.emplace_back("<GLOBAL>");
  root = module = &modules.at("");

  // Load default modules.

  // Numeric
  REGISTER_NATIVE(root, stdlib::abs);

  // common
  REGISTER_NATIVEL(root, stdlib::len);
  REGISTER_NATIVEL(root, stdlib::val);


  //string
  REGISTER_NATIVE(root, stdlib::asc);
  REGISTER_NATIVE(root, stdlib::chr);
  REGISTER_NATIVE(root, stdlib::left);
  REGISTER_NATIVE(root, stdlib::right);
  REGISTER_NATIVEL(root, stdlib::mid);
}

Context::Context(const std::filesystem::path& path) : Context() {
  add_source(path);
}

void BasicParserErrorListener::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line,
  size_t charPositionInLine, const std::string& msg,
  std::exception_ptr e) {
  su_->errors.push_back(fmt::format("{}({}:{}) {}", su_->filename_, line, charPositionInLine, msg));
}


void Context::upsert(const std::string& name, const Value& value) {
  
  if (const auto [pkg, id] = split_package_from_id(name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      modules.at(pkg).upsert(id, value);
      return;
    }
    // TODO(rushfan): Error that we don't have a module loaded for this.
    return;
  }

  // Not fully qualified case.
  if (!module->has_var(name) && root->has_var(name)) {
    // No existing variable in current module, but one at the root, update it.
    root->upsert(name, value);
  }
  else {
    // Add it to the current module.
    module->upsert(name, value);
  }
}

std::optional<Var> Context::var(const std::string& name) {
  
  if (const auto [pkg, id] = split_package_from_id(name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      return modules.at(pkg).var(id);
    }
    return std::nullopt;
  }
  // Not fully qualified case.
  if (!module->has_var(name) && root->has_var(name)) {
    // No existing variable in current module, but one at the root, update it.
    return root->var(name);
  }
  return module->var(name);
}

Value Context::call(const std::string& function_name, const std::vector<Value>& params,
                             ExecutionVisitor* visitor) {
  
  if (const auto [pkg, id] = split_package_from_id(function_name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      return modules.at(pkg).call(id, params, visitor);
    }
    // TODO(rushfan): Error that we don't have a module loaded for this.
    return Value(false);
  }
  // Not fully qualified case.
  if (!module->has_fn(function_name) && root->has_fn(function_name)) {
    // No existing variable in current module, but one at the root, update it.
    return root->call(function_name, params, visitor);
  }
  return module->call(function_name, params, visitor);

}

bool Context::add_source(const std::filesystem::path& path) {
  TextFile f(path, "rb");
  if (!f) {
    const auto err = fmt::format("Unable to open file: {}", path.string());
    errors.push_back(err);
    std::cout << err << std::endl;
    return false;
  }

  const auto text = f.ReadFileIntoString();
  return add_source(path.string(), text);
}
} // namespace wwivbasic