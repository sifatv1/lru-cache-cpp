# LRU Cache (C++14)

An in-memory least recently used cache with string keys and integer values. The
`Cache` type accepts string keys; `BasicCache<int>` supports integer keys.
Capacity must be positive, otherwise construction throws `std::invalid_argument`.
`get(key)` returns the value on a hit and `-1` on a miss. A stored value of `-1`
is allowed, so that return value alone cannot distinguish a hit from a miss.

## How it works

- A `std::unordered_map` maps each key to its entry in a `std::list`. Hash lookup
  gives average constant-time access to any key.
- The list stores entries in recency order: its front is the most recently used
  entry and its back is the least recently used entry.
- A successful `get` or an update in `put` moves the entry to the front with
  `std::list::splice`, which relinks its node without copying it.
- A new `put` inserts at the front. If the cache exceeds capacity, it removes
  the back entry from both the map and the list. A miss leaves the order alone.
- The map reserves space for `capacity + 1` keys. This also covers the brief
  insert-before-evict step without a rehash.

`get` and `put` take **O(1) average time**. Hash table operations have a
pathological **O(n) worst case** when many keys collide. The cache uses
**O(capacity) space** for the map and list. Construction reserves the map and
therefore uses O(capacity) time and space.

## Run the example

Use a C++14 compiler. From the repository root:

```sh
mkdir -p build
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic -Iinclude examples/main.cpp -o build/example
./build/example
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic -Iinclude examples/main.cpp -o build/example.exe
.\build\example.exe
```

Expected output:

```text
get(A): 10
get(B): -1
get(C): 30
get(A): 10
```

## Run the tests

The test program covers the given example, updates, misses, eviction,
nonpositive capacities, copies, integer keys, and 80,000 deterministic random
operations checked against an independent linear-time reference model.

```sh
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic -Iinclude tests/test_cache.cpp -o build/test_cache
./build/test_cache
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic -Iinclude tests/test_cache.cpp -o build/test_cache.exe
.\build\test_cache.exe
```

Expected output: `All LRU cache tests passed.`
