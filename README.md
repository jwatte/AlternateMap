# AlternateMap
A small implementation subset of std::map (really, std::unordered_map) that I 
know doesn't use any static members, and thus is guaranteed threading oblivious.

# Usage
The basic operations (insert, find, erase, and iterators) are implemented.
The map uses hashing, so ordering is not determinate, and changes on re-hashing 
when the map grows.

```
#include "imap.h"
  
imap<std::string, std::string> amap;
amap["1234"] = "5678";
for (auto &p : amap) {
    std::cout << p.first << "," << p.second << std::endl;
}
amap.erase(amap.find("1234"));
```

This is not intended to be the best implementation of a map in the world; 
instead it is intended to be easy to step through, debug, and verify.
For objects with non-trivial copy constructors, it may be less efficient than 
the standard library, and it is does not make use of move constructors.
