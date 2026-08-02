# Buffers

Voxelius uses a common API to serialize and deserialize data structures to/from a network-friendly representation. The `core/buffer.hh` header defines two classes: `ReadBuffer` and `WriteBuffer` that allow doing exactly this

## Base data types

These are read or written byte-by-byte in a specific order (network big-endian) and every other data type is based on these

|Type|Size|Read API|Write API|
|----|----|----|----|
|`byte`|1|`read<std::byte>()`|`write<std::byte>(...)`|
|`uint8`|1|`read<std::uint8_t>()`|`write<std::uint8_t>(...)`|
|`uint16`|2|`read<std::uint16_t>()`|`write<std::uint16_t>(...)`|
|`uint32`|4|`read<std::uint32_t>()`|`write<std::uint32_t>(...)`|
|`uint64`|8|`read<std::uint64_t>()`|`write<std::uint64_t>(...)`|

## Bit-casted data types

These are converted to/from base data types without any custom logic, just bytes in memory treated as a different type

|Type|Casted to|Read API|Write API|
|----|----|----|----|
|`int8`|`uint8`|`read<std::int8_t>()`|`write<std::int8_t>(...)`|
|`int16`|`uint16`|`read<std::int16_t>()`|`write<std::int16_t>(...)`|
|`int32`|`uint32`|`read<std::int32_t>()`|`write<std::int32_t>(...)`|
|`int64`|`uint64`|`read<std::int64_t>()`|`write<std::int64_t>(...)`|
|`float`|`uint32`|`read<float>()`|`write<float>(...)`|

> **NOTE:** for `float` casting to work, the host system must have them as [IEEE-754](https://en.wikipedia.org/wiki/IEEE_754) floating-point values so `float` is a single-precision number that takes 32 bits in memory

## Compound types

These are constructed from multiple base or bit-casted data types

|Type|Structure|Read API|Write API|
|----|----|----|----|
|`string`|`uint16`, `byte[N]`|`read<std::string>()`|`write<std::string_view>(...)`|
|`vector2<T>`|`T[2]`|`read_vector<T, 2>()`|`write_vector<T, 2>(...)`|
|`vector3<T>`|`T[3]`|`read_vector<T, 3>()`|`write_vector<T, 3>(...)`|
|`vector4<T>`|`T[4]`|`read_vector<T, 4>()`|`write_vector<T, 4>(...)`|

## C++ conventions

Usually, if an object can be serialized/deserialized, it defines two static functions in its header file:

```c++
class ReadBuffer;
class WriteBuffer;

struct MyPacket final {
  static void serialize(const MyPacket& packet, WriteBuffer& buffer);
  static void deserialize(MyPacket& packet, ReadBuffer& buffer);
};

class MyClass final {
public:
  static void serialize(const MyClass& object, WriteBuffer& buffer);
  static void deserialize(MyClass& object, ReadBuffer& buffer);

  explicit MyClass(...);
};
```

## Working with `ReadBuffer`

A `ReadBuffer` can be initialized from raw data (`std::span`), an `ENetPacket` pointer or from a `PHYSFS_File` pointer depending on what you need. Both constructor and `reset` overloads are available:

```c++
ReadBuffer buffer(packet);

// Or:
ReadBuffer buffer {};
buffer.reset(packet);
```

The `ReadBuffer` can also be `reset` with an `std::istream` reference so it can be used with standard library's file IO when needed

You can check if the buffer has ended with an `is_ended` method:

```c++
while(!buffer.is_ended()) {
  NetworkPacket packet;
  NetworkPacket::deserialize(packet, buffer);
  ...
}
```

You can rewind the read position with a `void rewind(void)` call

## Working with `WriteBuffer`

A `WriteBuffer` is initialized as empty but can be cleared with a `reset` method:

```c++
WriteBuffer buffer;

NetworkPacket packet = ....
NetworkPacket::serialize(packet, buffer);

send_packet(buffer);

buffer.reset();

// Can do all that again
```

The `WriteBuffer` can submit data to various targets:

```c++
NetworkPacket packet;
NetworkPacket::setup(packet, ...);

WriteBuffer buffer;
buffer.write<std::uint32_t>(magic);
NetworkPacket::serialize(packet, buffer);

// Save it to file
auto file = buffer.to_file("dump.bin");
vx::throw_if_not(file);
PHYSFS_close(file);

// Send it over network
auto packet = buffer.to_packet();
vx::throw_if_not(packet);
enet_peer_send(peer, 0, packet);

// Write it to a standard file
std::ofstream stream("dump.bin", std::ios::out | std::ios::binary);
vx::throw_if_not(stream.good());
buffer.to_stream(stream);

buffer.reset();
```
