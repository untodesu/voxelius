# Resources

The engine provides a resource management API. It allows a somewhat simple way to load resources from within mods and other places.

## Loading resources

- Use `res::load<T>(id, flags)` to load or find an existing resource

- Use `res::find<T>(id)` to find a loaded resource

> **NOTE:** `RESFLAG_CACHE` keeps a strong reference so soft purge does not drop the asset while nobody else holds it. Custom flag bits live in `RESFLAG_CUSTOM`. Soft purge frees unreferenced entries. Hard purge frees everything and warns about zombies still in use.

## Custom loader

> **NOTE:** Loaders only see a plain PhysFS path and flags. Namespaced ids are turned into paths before the loader runs. The loader must not care about mods or namespaces.

```c++
static const void* my_resource_load(const char* name, std::uint32_t flags)
{
  // stuff, return nullptr on error
}

static void my_resource_free(const void* resource)
{
  // usually just delete reinterpret_cast<const MyType*>(resource);
}

void MyType::register_resource(void)
{
  res::register_loader<MyType>(&my_resource_load, &my_resource_free);
}
```
