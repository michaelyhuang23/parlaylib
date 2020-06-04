// A slice is a non-owning view of a range defined by a pair
// of iterators. Slices can be created from any range object,
// or directly from an iterator pair. They can contain const
// iterators, in which case the slice represents a read-only
// view, or they can contain mutable iterators, so the slice
// can be used to modify the underlying sequence.
//
// Usage:
//
//   // create from a range
//   auto s = parlay::make_slice(r);  
//
//   // create from an iterator pair
//   auto s = parlay::make_slice(r.begin(), r.end())
//
//   // supports subscripting
//   std::cout << s[i] << std::endl;
//
//   // cutting out a substring
//   auto s_mid = s.cut(from,to);
//
// slice satisfies the parlay::Range concept so they can be
// used with all of Parlay's sequence primitives
//

#ifndef PARLAY_SLICE_H_
#define PARLAY_SLICE_H_

#include <cstddef>

#include <iterator>
#include <type_traits>

#include "range.h"

namespace parlay {

template <typename It, typename S>  
struct slice;

// Create a slice from an explicit iterator range
template<typename It, typename S>
auto make_slice(It it, S s) {
  return slice<It, S>(it, s);
}

// Create a slice from a range.
//
// The range must be passed by lvalue reference, since making
// a slice out of a temporary object would lead to the iterators
// dangling, leading to undefined behaviour.
template<PARLAY_RANGE_TYPE R>
auto make_slice(R&& r) {
  static_assert(std::is_lvalue_reference<R>::value,
    "Don't use make_slice on temporary objects. The iterators will dangle.");
  return make_slice(std::begin(r), std::end(r));
}

// A slice is a non-owning view of a range defined by an iterator pair.
template <typename It, typename S>  
struct slice {
 public:
 
  // Note the distinction -- value_type is the underlying type pointed to
  // by the iterator ignoring const-ness or reference-ness. reference is
  // the actual type returned by dereferencing the iterator, which may
  // not actually be a reference (for delayed sequence, it is a value)
  using value_type = typename std::iterator_traits<It>::value_type;
  using reference = typename std::iterator_traits<It>::reference;
  
  using iterator = It;
  using sentinel = S;
  
  slice(iterator s, sentinel e) : s(s), e(e){};
  
  // Copy construction and assignment
  slice(const slice<It,S>&) = default;
  slice<It,S>& operator=(const slice<It,S>&) = default;
  
  // Return the i'th element of the sequence
  reference operator[](size_t i) const { return s[i]; }
  
  // Return the size of the sequence
  auto size() const { return e - s; }
  
  // Return a slice corresponding to the subrange from
  // positions ss to ee.
  slice<It, It> cut(size_t ss, size_t ee) const {
    return make_slice(s + ss, s + ee);
  }
  
  // Iterator access
  iterator begin() const { return s; }
  sentinel end() const { return e; }

 private:
  iterator s;
  sentinel e;
};

}

#endif  // PARLAY_SLICE_H_
