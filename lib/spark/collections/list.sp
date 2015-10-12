/** List interface */
interface List[Element] : Sequence[Element] {

  /** Append an item to the end of the list.
      @param e the element to append to the list.
   */
  def add(e:Element);

  /** Append all of the items in a collection to the end of the list.
      @param src The collection of elements to append.
   */
  def addAll(src:const Slice[Element]);
  def addAll(src:const Collection[Element]);

  /** Insert the element 'e' at the specific position 'position'.
      @param position The insertion point.
      @param e The element to insert.
   */
  def insert(position:int, e:Element);

  /** Insert all of the elements of the collection 'collection' at the specific position 'position'.
      @param position The insertion point.
      @param src The collection of elements to insert.
   */
  def insertAll(position:int, src:const Slice[Element]);
  def insertAll(position:int, src:const Collection[Element]);

  /** Replace 'count' elements, starting from 'index', with the contents of 'src'.
      @param index The starting index of the elements to replace.
      @param count The number of elements to remove.
      @param src The set of replacement elements.
   */
  def replace(index:int, count:int, src:const Slice[Element]);
  def replace(index:int, count:int, src:const Collection[Element]);

  /** Remove the item at the specified index. */
  def remove(index:int);

  /** Remove all items from the list. */
  def clear();

  /** Return true if the specified item is in the collection. */
  def contains(self:const, item:Element) -> bool;
}
