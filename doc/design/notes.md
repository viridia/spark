# Design notes

## Memory management and object lifetimes.

* Runtime objects should be in perm space.
* Runtime objects should not refer to the semantic graph.
* Runtime objects should be built only from the serialized semantic graph.
* How do we handle the mixing of STL collections with arena-based objects?
  * Answer: we don't, allocators and shared pointers are simply too complex to deal with.
* New strategy:
  * Defns and scopes are allocated on the regular stack.
  * Defns own scopes via auto_ptr.
  * Defn destructors are responsible for cleaning up members.
  * Special-cased for local defns.
  * Expressions allocated on the module's arena.

Do scopes need to be built incrementally? Examples:

* Local block scopes: symbols only get defined as they are encountered in the block (although...it's
  actually an error to refer to a symbol before it's defined in the block, so in a sense we can
  say that the scope needs to be created all at once.)
* OK here's an example: Adding a constructor to the member scope if no constructor has been defined.
  * But that's one of the *VERY FEW* cases.
