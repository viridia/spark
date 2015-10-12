/** Exception thrown when a sequence subscript is out of range. */
class IndexError : Exception {}

/** Exception thrown when a map key lookup fails. */
class KeyError : Exception {}

/** Exception that is thrown when a method detects a concurrent mutation of an object
    when such mutation is not permitted.

    For example, it is generally not permitted to modify a Collection while iterating
    over its contents.
 */
class ConcurrentMutationError : Exception {}
