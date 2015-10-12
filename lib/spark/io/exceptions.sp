import spark.collections.Array;

/** Exception that signals an i/o error. */
class IOError : Exception {}

/** Exception thrown when attempting to open a non-existent file for reading. */
class FileNotFoundError : IOError {}

/** Exception thrown when attempting to read a non-existent directory. */
class DirectoryNotFoundError : IOError {}

/** Exception thrown when attempting to create a file that already exists. */
class FileAlreadyExistsError : IOError {}

/** Indicates that the user did not have authorization to perform the operation. */
class UnauthorizedAccessError : IOError {}
