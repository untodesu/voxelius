# Configuration

The engine provides a simple API to parse/store configuration. C++ API provides a way to read these as specific data types. Use it for user preferences, server configuration, stuff like [`modinfo.conf`](../modding/init-modinfo.md) and so on.

## Pieces

- `config::Map` is a string-keyed configuration parser. Allows directly reading values, or binding a typed reference to a key

- `config::Ref<T>` is a typed reference to a key inside `config::Map`. It uses generation counters for more-or-less quick reading if values do not change for a while  

> **NOTE:** one doesn't need a global map. For convenience, client and server define a `globals::client_config` and `globals::server_config` respectively to use as a global configuration but stuff like [modinfo.conf](../modding/init-modinfo.md) is parsed with a separately created and destroyed `config::Map`

## Example: reading/writing directly

```c++
config::Map config;
config.load("myconfig.conf");

auto integer = config.value<int>("value.integer");
assert(integer.has_value());

config.set_value<Eigen::Vector3f>("value.vector", { 1.0f, 2.0f, 3.0f });

config.save("myconfig.conf");
```

## Example: binding a reference

```c++
static config::Ref<int> s_integer { 42 };

void mysystem::init(void)
{
  s_integer.bind(globals::client_config, "value.integer");

  // If we're on client, we can register
  // a GUI setting that references the exact
  // same key and internally keeps a config::Ref<T> as well
  settings::slider(2, settings_location::MOUSE, "value.integer", 40, 43, false);
}

void mysystem::update(void) // called per-frame
{
  auto value = s_integer.value();
  LOG_INFO("{}", value);
}
```
