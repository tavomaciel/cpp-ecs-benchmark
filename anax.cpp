#include <stdio.h>
#include <anax/anax.hpp>
#include <chrono>
#include <random>

using namespace std::chrono;
using namespace anax;

struct Position : public Component {
  Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

  float x, y;
};

struct Direction : public Component {
  Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

  float x, y;
};

struct Health : public Component {
    Health(float currentHealth = 2500.0f) : currentHealth(currentHealth) {}

    float currentHealth;
};

bool is_close(Position &pos1, Position &pos2, float dist) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return (dx*dx + dy*dy) < (dist*dist);
}

struct MovementSystem : public System<Requires<Position, Direction>> {
  void update(double dt) {
    auto entities = getEntities();
    for(auto entity : entities) {
        auto &position = entity.getComponent<Position>();
        auto &direction = entity.getComponent<Direction>();
        position.x += direction.x * dt;
        position.y += direction.y * dt;
    }
  };
  ~MovementSystem() {}
};

struct CollisionSystem : public System<Requires<Position, Health>> {
    void update(double dt) {
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
                    //printf("Collision!\n");
                }
            }
        }
    }
};

int removed;
struct LifeSystem : public System<Requires<Health>> {
    void update(double dt) {
        auto entities = getEntities();
        for(auto entity : entities) {
            auto &health = entity.getComponent<Health>();
            if(health.currentHealth <= 0) {
                printf("Destroyed entity %lu\n", entity.getId().index);
                entity.kill();
                removed++;
            }
        }
    }
};

Entity createEntity(World &world, std::uniform_real_distribution<float>& distribution, std::mt19937& generator) {
    Entity entity = world.createEntity();
    entity.addComponent<Position>(distribution(generator) * 250.0f, distribution(generator) * 250.0f);
    entity.addComponent<Direction>(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f);
    entity.addComponent<Health>(2500.0f);
    printf("Created entity %lu at (%f, %f), dir (%f, %f)\n", entity.getId().index, entity.getComponent<Position>().x,
        entity.getComponent<Position>().y, entity.getComponent<Direction>().x, entity.getComponent<Direction>().y);
    entity.activate();
    return entity;
}


int main(int, char**) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_real_distribution<float> distribution (0.0f,1.0f);
    World world;

    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    LifeSystem lifeSystem;
    world.addSystem(movementSystem);
    world.addSystem(collisionSystem);
    world.addSystem(lifeSystem);

    for(int i = 0; i < 1000; ++i) {
        createEntity(world, distribution, generator);
    }

    double dt = 0.16666;
    printf("Running anax test\n");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int created = 1000;
    removed = 0;
    world.refresh();
    for (long i = 0; i < 1000; ++i)
    {
        if((i % 10) == 1) {
            createEntity(world, distribution, generator);
            created++;
        }
        movementSystem.update(dt);
        collisionSystem.update(dt);
        lifeSystem.update(dt);
        world.refresh();
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    printf("End of anax test. Time taken: %ld us\nEntities created: %d\nEntities removed: %d\n", duration, created, removed);
}
