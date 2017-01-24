#include <stdio.h>
#include <entityx/entityx.h>
#include <chrono>
#include <random>

using namespace std::chrono;
using namespace entityx;

struct Position {
  Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

  float x, y;
};

struct Direction {
  Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

  float x, y;
};

struct Health {
    Health(float currentHealth = 2500.0f) : currentHealth(currentHealth) {}

    float currentHealth;
};

bool is_close(Position &pos1, Position &pos2, float dist) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return (dx*dx + dy*dy) < (dist*dist);
}

struct MovementSystem : public System<MovementSystem> {
  void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {

    es.each<Position, Direction>([dt](Entity entity, Position &position, Direction &direction) {
      position.x += direction.x * dt;
      position.y += direction.y * dt;
    });
  };
  ~MovementSystem() {}
};

struct CollisionSystem : public System<CollisionSystem> {
    void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
        es.each<Position, Health>([&es, dt](Entity entity, Position &position, Health &health) {
            es.each<Position, Health>([dt, &entity, &position](Entity entity2, Position &position2, Health &health2) {
                if(entity != entity2 && is_close(position, position2, 10.f)) {
                    health2.currentHealth -= 1.0f;
                    //printf("Collision!\n");
                }
            });
        });
    }
};
int removed;
struct LifeSystem : public System<LifeSystem> {
    void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
        es.each<Health>([&es, dt](Entity entity, Health &health) {
            if(health.currentHealth <= 0) {
                printf("Destroyed entity %lu\n", entity.id().id());
                entity.destroy();
                removed++;
            }
        });
    }
};

entityx::Entity createEntity(EntityX &ex, std::uniform_real_distribution<float>& distribution, std::mt19937& generator) {
    entityx::Entity entity = ex.entities.create();
    entity.assign<Position>(distribution(generator) * 250.0f, distribution(generator) * 250.0f);
    entity.assign<Direction>(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f);
    entity.assign<Health>(2500.0f);
    printf("Created entity %lu at (%f, %f), dir (%f, %f)\n", entity.id().id(), entity.component<Position>()->x,
        entity.component<Position>()->y, entity.component<Direction>()->x, entity.component<Direction>()->y);
    return entity;
}

int main(int, char**) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_real_distribution<float> distribution (0.0f,1.0f);
    EntityX ex;

    ex.systems.add<MovementSystem>();
    ex.systems.add<CollisionSystem>();
    ex.systems.add<LifeSystem>();
    ex.systems.configure();

    for(int i = 0; i < 1000; ++i) {
        createEntity(ex, distribution, generator);
    }

    TimeDelta dt = 0.16666;
    printf("Running Entity-x test\n");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int created = 1000;
    removed = 0;
    for (long i = 0; i < 1000; ++i)
    {
        if((i % 10) == 1) {
            createEntity(ex, distribution, generator);
            created++;
        }
        ex.systems.update<MovementSystem>(dt);
        ex.systems.update<CollisionSystem>(dt);
        ex.systems.update<LifeSystem>(dt);
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    printf("End of Entity-x test. Time taken: %ld us\nEntities created: %d\nEntities removed: %d\n", duration, created, removed);
    return 0;
}
