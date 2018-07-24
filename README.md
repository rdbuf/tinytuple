Aimed at a partial [implementation](http://eel.is/c++draft/tuple.syn) of `std::tuple`.

Relies on an [unspecified](http://eel.is/c++draft/class.derived#5) property of the base class subobjects layout: it is assumed they get laid out trivially, in the order of derivation.

For such a case this implementation optimizes the layout, taking the alignment requirements of each type into account and minimizing padding:

```c++
static_assert(sizeof(tiny::tuple<bool, long, char, bool, int, bool>) == 16);
static_assert(sizeof(std::tuple<bool, long, char, bool, int, bool>) == 32);
```

Inspired by the two brilliant talks:
- [Odin Holmes](https://twitter.com/odinthenerd), "[The fastest template metaprogramming in the West](https://www.youtube.com/watch?v=ZpVPexZHYrQ)", code::dive '17
- [Bartosz Milewski](https://twitter.com/BartoszMilewski), "[Haskell -- The Pseudocode Language for C++ Template Metaprogramming (Part 1)](https://www.youtube.com/watch?v=GjhsSzRtTGY)", BoostCon '13
