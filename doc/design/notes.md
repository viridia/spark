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

## Incomplete lookups.

Deferred symbol lookups occur when the lookup of a name depends on type information which, and
the type has not yet been inferred.

The question is whether type name lookups can be deferred. Generally they shouldn't be.

Examples:
  var foo:Graph[Node].Iterator;
  var bar:

One kind of incomplete lookup is symbols defined in a base class which are looked up from inside
the context of a subclass. In order for this to happen, the base class type has to be resolved, at
least to the extend that the base class's members are accessible. However, this can lead to a
cyclic dependency: a base class can contain a member whose type is a subclass of that base class.

First, let's talk about restrictions on base classes: the scope for resolving base classes is the
one containing the subclass, so unqualified names can't refer to names inside the subclass. However,
this doesn't restrict qualified names from doing so.

In a previous version of spark we had a fairly complicated system for resolving cycling type
dependencies. It allowed type resolution steps to fail (if, for example, the lookup scope was
not completely built yet) and be retried at a later stage. However, I'd really like to avoid that
level of complexity if I can possibly avoid doing so.

The simplest scheme would be one in which name lookups proceeded in some defined order that was
guaranteed to work. So in some hypothetical language, imports would always be looked up first,
then base classes (and subsequently we could populate inherited scopes for classes), then class
members, and finally local declarations.

Unfortunately, a single ordering won't work: member lookups can be affected by base classes, and
base class name lookups can be affected by members.

I think we'll use the same algorithm that we did for python spark:

* Base classes are resolved on-demand.
* Inherited member scopes populate themselves on demand.
* Check for cycles and raise an error.

## Can we simplify the lookup of members?

In python spark, we had throw-away scope wrappers for inherited scopes and instantiated scopes.

What would be nice if we could cut out the middleman and simply transform a list of stem members
into a list of leaf members, where some of those leaves would be specialized as needed.

There are several variations:

-- members -> members
-- types -> members
-- static vs. instance

The issue with all this is that these wrapper scopes do have one important use, which is that they
make name lookups faster, particularly in the case of templated base types.
