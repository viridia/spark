import spark.core.memory.Address;
import spark.core.enumeration.FlagsEnum;
import spark.collections.Array;

/** Base class for all attributes. */
__intrinsic__ class Attribute {
  /** Defines what types of elements the attribute can be attached to. */
  enum Target : FlagsEnum {
    CLASS, 				///< Attribute can annotate class types.
    STRUCT,				///< Attribute can annotate struct types.
    INTERFACE, 		///< Attribute can annotate interface types.
    ENUM,			 		///< Attribute can annotate enum types.
    NAMESPACE, 		///< Attribute can annotate namespaces.
    FUNCTION, 		///< Attribute can annotate functions and methods.
    CONSTRUCTOR, 	///< Attribute can annotate constructors.
    PARAMETER, 		///< Attribute can annotate parameters.
    VARIABLE, 		///< Attribute can annotate variables and constants.
    PROPERTY, 		///< Attribute can annotate properties.
    MACRO, 				///< Attribute can annotate macros.

		/** Attribute can annotate any type definition. */
    TYPE, // = CLASS | STRUCT | INTERFACE | ENUM,

		/** Attribute can annotate any callable. */
    CALLABLE, // = CONSTRUCTOR | FUNCTION | MACRO,

		/** Attribute can annotate any declaration. */
    ANY, // = TYPE | NAMESPACE | CALLABLE | PARAMETER | VARIABLE | PROPERTY,
  }

  /** Indicates whether the attribute is retained in the compiled binary. */
  enum Retention {
    NONE,							///< Attribute is only present during compilation.
    RUNTIME,					///< Attribute is retained at runtime.
    RUNTIME_ITERABLE, ///< Attribute is retained at runtime, and all instances are iterable.
  }

  /** Indicates whether derived elements (callers, subclasses) get the attribute as well. */
  enum Propagation : FlagsEnum {
    NONE /*= 0*/,
    SUBTYPES,   ///< Attribute propagates to subtypes
    MEMBERS,    ///< Attribute propagates to members of attached type
//    CALLERS,    ///< Attribute propagates to callers
  }

  let target:Target;
  let retention:Retention;
  let propagation:Propagation;

  def new() {
    .target = Target.ANY;
    .retention = Retention.NONE;
    .propagation = Propagation.NONE;
  }

  def new(target:Target, retention:Retention = NONE, propagation:Propagation = NONE) {
    .target = target;
    .retention = retention;
    .propagation = propagation;
  }
}

/** Attribute class that is used to designate the 'main' method of a program. */
@Attribute(Attribute.Target.FUNCTION)
__intrinsic__ class EntryPoint {
  private static def programStart(
      argc:int,
      argv:Address[Address[u8]],
      main:fn (Array[String]) -> int) -> int {
    return main(Array(0));
  }
}

/** Attribute class used to mark a method having a native implementation. */
@Attribute(Attribute.Target.FUNCTION)
__intrinsic__ class Native {
  let linkageName:String;

  def new() {
    .linkageName = "";
  }

  def new(linkageName:String) {
    .linkageName = linkageName;
  }
}
