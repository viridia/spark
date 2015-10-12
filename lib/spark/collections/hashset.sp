/** A quadratically-probed hash set. */
final class HashSet[
    Element,
    Comparator <: hashing.HashComparator[Element] = hashing.DefaultHashComparator[Element]]
  : Set[Element]
    where Comparator.new()
{
  private {
    static let comparator = Comparator();

    // Special sentinel hash values for empty and deleted slots. */
    static let EMPTY:u64 = 0;
    static let DELETED:u64 = 1;

    struct Entry {
      let value:Element;
      let hash:u64;

      def new(value:Element, hash:u64) {
        .value = value;
        .hash = hash;
      }
    }

    var data:Array[Entry];
    var sz:int;
    var modified:bool = false;
  }

  override size:int { get => sz; }

  override empty:bool { get => sz == 0; }

  //override contains(value:Element) -> bool => findEntry(value) >= 0;
  undef contains(value:Element) -> bool;

  override clear() {
    data = Array(0);
    sz = 0;
  }

  undef add(element:Element);
  undef addAll(elements:Element...);
  undef addAll(elements:Iterable[Element]);
  undef remove(element:Element) -> bool;
  undef removeAll(elements:Element...);
  undef removeAll(elements:Iterable[Element]);
  undef iterate(self:const) -> Iterator[Element];
}
