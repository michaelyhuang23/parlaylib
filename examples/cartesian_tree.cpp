#include <iostream>
#include <string>
#include <utility>
#include <random>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/random.h>
#include <parlay/internal/get_time.h>

// **************************************************************
// Cartesian Tree
// Given a sequence of numbers, builds the cartesian tree on them.  In
// the cartesian tree the smallest number is placed at the root, and
// recursively the left subtree is the cartesian tree of the numbers
// to the left of it in the input sequence, and the right subtree is
// the cartesian trees of the numbers to the right of it.
// Returns a binary tree where every position points to its parent
// position.  The root points to itself.
// Ties on equal elements are broken arbitrarily.
// Uses the parallel algorithm from:
//   A Simple Parallel Cartesian Tree Algorithm and its Application
//   to Parallel Suffix Tree Construction.
//   Julian Shun and Guy Blelloch
//   ACM Transactions on Parallel Computing, 2014.
// It is a divide-and-conquer algorithm that builds the
// cartesian trees for the two halves, and then merges their spines.
// Does linear total work.
// **************************************************************

// here we use longs as our numbers
using seq = parlay::sequence<long>;

// V are the values, and P the parents
void spine_merge(const seq& V, seq& P, long left, long right) {
  long head;
  if (V[left] > V[right]) {
    head = left; left = P[left];}
  else {head = right; right= P[right];}

  while(1) {
    if (V[left] > V[right]) {
      P[head] = left;
      if (P[left] == left) {P[left] = right; break;}
      left = P[left];}
    else {
      P[head] = right;
      if (P[right] == right) {P[right] = left; break;}
      right = P[right];}
    head = P[head];
  }
}

void cartesian_tree(const seq& V, seq& P, long s, long e) {
  if (e-s < 2) {
  } else if (e-s == 2) {
    if (V[s] > V[s+1]) P[s]= s+1;
    else P[s+1] = s;
  } else {
    long mid = (s+e)/2;
    parlay::par_do_if((e-s) > 100,
	  [&] {cartesian_tree(V, P, s, mid);},
          [&] {cartesian_tree(V, P, mid, e);});
    spine_merge(V, P, mid-1, mid);
  }
}

seq cartesian_tree(const seq& V) {
  seq parents = parlay::tabulate(V.size(), [] (long i) {return i;});
  cartesian_tree(V, parents, 0, V.size());
  return parents;
}

// **************************************************************
// Driver
// **************************************************************

parlay::sequence<long> generate_values(long n) {
  parlay::random_generator gen;
  std::uniform_int_distribution<long> dis(0, n-1);
  
  return parlay::tabulate(n, [&] (long i) {
      auto r = gen[i];
      return dis(r);
    });
}

int main(int argc, char* argv[]) {
  auto usage = "Usage: cartesian_tree <n>";
  if (argc != 2) std::cout << usage << std::endl;
  else {
    long n;
    try { n = std::stol(argv[1]); }
    catch (...) { std::cout << usage << std::endl; return 1; }

    parlay::sequence<long> values = generate_values(n);
    parlay::sequence<long> parents;
    parlay::internal::timer t;
    for (int i=0; i < 3; i++) {
      parents = cartesian_tree(values);
      t.next("Cartesian Tree");
    }

    auto h = parlay::tabulate(n, [&] (long i) {
	long depth = 1;
	while (i != parents[i]) {
	  i = parents[i];
	  depth++;
	}
	return depth;});

    std::cout << "depth of tree: " 
	      << parlay::reduce(h, parlay::maximum<long>())
	      << std::endl;
  }
}
