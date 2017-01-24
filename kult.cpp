#include <stdio.h>
#include <iostream>
#include <chrono>
#include <random>
#include <kult.hpp>

using namespace std::chrono;
using namespace entityx;
using namespace kult;

struct Position {
  Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

  float x, y;
  template<class T>
  friend inline T& operator <<( T &ostream, const vec2f &self ) {
      return ostream << "(x=" << self.x << ",y=" << self.y << ")", ostream;
  }
};

struct Direction {
  Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    float x, y;
    template<class T>
    friend inline T& operator <<( T &ostream, const vec2f &self ) {
        return ostream << "(x=" << self.x << ",y=" << self.y << ")", ostream;
    }
};

struct Health {
    Health(float currentHealth = 2500.0f) : currentHealth(currentHealth) {}

    float currentHealth;
    friend inline T& operator <<( T &ostream, const vec2f &self ) {
        return ostream << "(x=" << self.x << ",y=" << self.y << ")", ostream;
    }
};

using position = component<'position', Position>;
using direction = component<'direction', Direction>;
using health = component<'health', Health>;

bool is_close(Position &pos1, Position &pos2, float dist) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return (dx*dx + dy*dy) < (dist*dist);
}

kult::system<double> movement = [&](double dt) {
        for(auto &id : join(position, direction)) {
            get<position>(id).x += get<velocity>(id).x * dt;
            get<position>(id).y += get<velocity>(id).y * dt;
        }
};

kult::system<double> collision = [&](double dt) {
    auto entities = join(position, health);
    for(auto it = entities.begin(), end = entities.end(); it != end; ++it) {
        auto &entity1 = *it;
        auto jt = it;
        jt++;
        for(; jt != end; ++jt) {
            auto &entity2 = *it;
            if(is_close(get<position>(entity1), get<position>(entity2), 10.f)) {
                get<health>(entity1).currentHealth -= 1.0f;
                get<health>(entity2).currentHealth -= 1.0f;
            }
        }
    }
};
int removed;
kult::system<double> life = [&](double dt) {
    for(auto &entity : join(health)) {
        if(get<health>(entity).currentHealth <= 0) {
            printf("Destroyed entity %lu\n", entity);
            purge(entity);
            removed++;
        }
    }
};
int lastId = 0;
kult::entity createEntity(std::uniform_real_distribution<float>& distribution, std::mt19937& generator) {
    int e = lastId++;
    float x = distribution(generator) * 250.0f;
    float y = distribution(generator) * 250.0f;
    add<position>(e) = {x, y};
    x = distribution(generator) * 2.0f - 1.0f;
    y = distribution(generator) * 2.0f - 1.0f;
    add<direction>(e) = {x, y}
    add<health>(e) = {2500.0f};
    printf("Created entity %lu at (%f, %f), dir (%f, %f)\n", e, get<position>(e).x,
        get<position>(e).y, get<direction>(e).x, get<direction>(e).y);
    return e;
}

int main(int, char**) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_real_distribution<float> distribution (0.0f,1.0f);

    for(int i = 0; i < 1000; ++i) {
        createEntity(distribution, generator);
    }

    double dt = 0.16666;
    printf("Running kult test\n");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int created = 1000;
    removed = 0;
    for (long i = 0; i < 1000; ++i)
    {
        if((i % 10) == 1) {
            createEntity(distribution, generator);
            created++;
        }
        movement(dt);
        collision(dt);
        life(dt);
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    printf("End of Entity-x test. Time taken: %ld us\nEntities created: %d\nEntities removed: %d\n", duration, created, removed);
    return 0;
}
