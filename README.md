# cpp-ecs-benchmark
Benchmarks of the available open-source Entity-Component-System libraries available for C++

The lshw specs of the computer used for the tests can be found at `test_computer_specs.html`

## Installing / building

- Entity-x:
    Must be compiled separately and linked. Used to be dependant on Boost, but it seems that they changed everything to C++11.
    The .so itself is around 466kb compiled without any extra flags than the ones provided in the CMakeLists.

- Anax:
    Should also be linked. Has no further dependencies but C++11.
    The .so itself is around 376kb compiled without any extra flags than the ones provided in the CMakeLists

- Kult:
    Header only, while this is nice for ease of use and install, it may add some overhead on the compile times.

# Out of order traversal

One common use with entities is to try to match entities that are colliding.
An example pseudo loop would look like this:
```c++
    for(int i = 0; i < entities.size; ++i) {
        auto entity1 = entities[i];
        for(int j = i+1; j < entities.size; ++i) {
            auto entity2 = entities[j];
            doLogic(entity1, entity2);
        }
    }
```
There's no reason to process every entity behind i again, because we know that those entities have already been passed
on the test.

- Entity-x:
    I swear I looked everywhere, but every interface open to me, are iterators. I think I can't control neither the order
    for the traversal, or the starting point.

    O(n²) traversal example:
    ```c++
    es.each<Position, Health>([&es, dt](Entity entity, Position &position, Health &health) {
        es.each<Position, Health>([dt, &position](Entity entity2, Position &position2, Health &health2){
            if(is_close(position, position2, 10.0f)) {
                health2.currentHealth -= 0.1f;
            }
        });
    });
    ```
- Kult:
    After a join you get a set, you can get the begin and end iterators yourself and do something like this:

    ```c++
    auto entities = join(position, health);
    for(auto it = entities.begin(), end = entities.end(); it != end; ++it) {
        auto &entity1 = (*it);
        auto jt = it;
        jt++;
        for(; jt != end; ++jt) {
            auto &entity2 = (*it);
            if(is_close(get<position>(entity1), get<position>(entity2), 10.f)) {
                get<health>(entity1).currentHealth -= 1.0f;
                get<health>(entity2).currentHealth -= 1.0f;
            }
        }
    }
    ```**
- Anax:
    While a little more verbose, it doesn't hide its implementation under any iterator. You know exactly what your system will process, and it's available under a vector.
    O(n²) traversal example:
    ```c++
    auto entities = getEntities();
    for(int i = 0, s = entities.size(); i < s; ++i) {
        for(int j = i+1; j < s; ++j) {
            auto entity1 = entities[i];
            auto entity2 = entities[j];
            auto &pos1 = entity1.getComponent<Position>();
            auto &pos2 = entity2.getComponent<Position>();
            if(is_close(pos1, pos2, 10.f)) {
                auto &health1 = entity1.getComponent<Health>();
                auto &health2 = entity2.getComponent<Health>();
                health1.currentHealth -= 1.0f;
                health2.currentHealth -= 1.0f;
            }
        }
    }
    ```

# Finding entities with given components outside of systems.
If you ever have to find which entities have Components A and B, then:

- Entity-X has got you covered:
```cpp
ex.entities.each<Health>([&removed](Entity e, Health &health) {
    doLogic(e, health);
});
```
I've found this to neither be faster or slower than using systems in entityX

- Anax on the other hand do not. But this is not that bad.
It's interface is made in a way that a system is actually not something that processes entities, it actually is a filter of entities. This means the only thing a system has by default is a "getEntities" method, and that can be used as you wish.

- Kult was probably made with this in mind. You just need a `kult::join(component1, component2);` and you get a set with these components.

# Processing time.

All tests creates 3 components (Position, Direction, Health) and 2 systems (MovementSystem, CollisionSystem).
1000 entities are added at start and one more entity will enter every 10 frames.
The test then will run 1 000 frames. Each frame both systems are updated, and outside of systems (to check flexibility)
we check all entities that have health below 0 and destroy these.
Every test is run 6 times.

- Entity-x: Avg 25.304sec, Min 24.848sec, Max 25.632sec
- Anax: I didn't expect it to be this slow. Ran it only two times: Avg 75.502762sec, Min 72.75281sec, Max 78.252714sec
- Kult: I couldn't get it to run. There's no documentation, the examples have differing syntaxes. All of them that I tried, all give tons of template errors.
