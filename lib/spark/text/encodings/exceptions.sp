/** Exception thrown when an attempt is made to encode a character that is not supported
    by the encoding. */
class InvalidCharacterError : Exception {}

/** Exception thrown when attempting to decode a sequence of input bytes which is not
    valid for the encoding. */
class MalformedInputError : Exception {}
