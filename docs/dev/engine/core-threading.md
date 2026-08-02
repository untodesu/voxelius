# Threading

The egine wraps around `BS::thread_pool` to shove heavy work into background threads. Stuff like chunk meshing and world generation are "threaded out" as separate tasks.

## How to use it

1. Implement `Task`.
2. Put CPU work in `process` method that runs in a worker thread.
3. Put side-effects in `finalize` method that runs in the main thread.
4. Submit a task: `threading::submit<MyTask>(...)`.
5. If in a custom executable, call `threading::update()` once per frame/tick on the main thread.

```c++
class MyTask final : public Task {
public:
  explicit MyTask(int argument);
  void process(void) override;
  void finalize(void) override;
};

// Whenever you need it
threading::submit<MyTask>(42);
```

## Command line

Pass in a `threads` option to pick a thread pool size, use `max` to use `std::hardware_concurrency` (aka max threads):  

```
voxelius-client.exe -threads 4
```

```
voxelius-client.exe -threads max
```
