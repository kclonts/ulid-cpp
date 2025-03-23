# ULID (Universally Unique Lexicographically Sortable Identifier)

A header-only C++20 implementation of [ULID](https://github.com/ulid/spec).

ULIDs are 128-bit identifiers that combine a timestamp with random data to create identifiers that are:
- Lexicographically sortable
- URL-safe
- Globally unique
- Human-readable

## Features

- Header-only implementation
- Simple API
- Thread-safe
- Time-based ordering
- Optional OpenSSL support for better entropy
- Boost compatibility (via `boost::uuids`)

## Requirements

- C++20 compatible compiler
- CMake 3.21+ (for building)
- OpenSSL (optional)
- Boost (optional)

## Installation

### Using CMake

```bash
git clone https://github.com/yourusername/ulid.git
cd ulid
cmake --preset release
cmake --build ./build
cmake --install ./build
```

### Using in Your Code

```cpp
#include "ulid.h"

int main() {
    // Create a ULID with current timestamp and random entropy
    ulid::ULID id = ulid::CreateNowRand();

    // Convert to string representation
    std::string str = ulid::Marshal(id);
    std::cout << "ULID: " << str << std::endl;

    // Get timestamp from ULID
    auto time_point = ulid::Time(id);
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::cout << "Timestamp: " << std::ctime(&time_t);

    // Convert to UUID (for database storage)
    boost::uuids::uuid uuid = ulid::MarshalUuid(id);

    // Convert back from UUID to ULID
    ulid::ULID id_from_uuid = ulid::UnmarshalBinary(uuid);

    return 0;
}
```
## Credits

Initial Library by Suyash https://github.com/suyash/ulid
Fix made to use milliseconds correctly by ChrisBove https://github.com/ChrisBove/ulid
