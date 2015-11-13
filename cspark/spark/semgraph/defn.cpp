#include "spark/semgraph/defn.h"
#include "spark/semgraph/type.h"

namespace spark {
namespace semgraph {

Member::~Member() {
}

void Defn::formatModifiers(std::ostream& out) const {
  if (_visibility == PRIVATE) {
    out << "private ";
  } else if (_visibility == spark::semgraph::PROTECTED) {
    out << "protected ";
  }

  if (_static) {
    out << "static ";
  }
  if (_final) {
    out << "final ";
  }
  if (_override) {
    out << "override ";
  }
  if (_abstract) {
    out << "abstract ";
  }
  if (_undef) {
    out << "undef ";
  }
}

PossiblyGenericDefn::~PossiblyGenericDefn() {
  for (auto tparam : _typeParams) {
    delete tparam;
  }
  for (auto iscope : _interceptScopes) {
    delete iscope.second;
  }
}

TypeDefn::~TypeDefn() {
  for (Member* member : _members) {
    delete member;
  }
}

void TypeDefn::format(std::ostream& out) const {
  formatModifiers(out);
  switch (type()->kind()) {
    case Type::Kind::CLASS: out << "class "; break;
    case Type::Kind::STRUCT: out << "struct "; break;
    case Type::Kind::INTERFACE: out << "interface "; break;
    case Type::Kind::ENUM: out << "enum "; break;
    default:
      assert(false);
  }
  out << name();
}

void TypeParameter::format(std::ostream& out) const {
  formatModifiers(out);
  out << "tparam " << name();
}

void ValueDefn::format(std::ostream& out) const {
  formatModifiers(out);
  switch (kind()) {
    case Kind::LET: out << "let "; break;
    case Kind::VAR: out << "var "; break;
    default:
      assert(false);
  }
  out << name();
}

void Parameter::format(std::ostream& out) const {
  formatModifiers(out);
  out << "param " << name();
}

Function::~Function() {
  for (Parameter* param : _params) {
    delete param;
  }
}

void Function::format(std::ostream& out) const {
  formatModifiers(out);
  out << "fn " << name();
}

Property::~Property() {
  for (Parameter* param : _params) {
    delete param;
  }
  if (_getter != nullptr) {
    delete _getter;
  }
  if (_setter != nullptr) {
    delete _setter;
  }
}

void Property::format(std::ostream& out) const {
  formatModifiers(out);
  out << "fn " << name();
  out << name();
}

}}

