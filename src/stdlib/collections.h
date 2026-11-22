#pragma once

namespace wwivbasic {
class Module;
}

namespace wwivbasic::stdlib {

// Registers the my_basic-compatible collection functions (LIST, PUSH, POP,
// BACK, INSERT, REMOVE, CLEAR, EXISTS, INDEX_OF, GET, SET, CLONE, SORT,
// ITERATOR, MOVE_NEXT) in the given module. Indices are 0-based.
void register_collections(Module& m);

} // namespace wwivbasic::stdlib
