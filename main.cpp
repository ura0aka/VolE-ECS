#include "Game.hpp"


#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <random>

std::default_random_engine gen;
std::uniform_real_distribution<float> randPosx(0.0f, 500.0f);
std::uniform_real_distribution<float> randPosy(0.0f, 500.0f);
std::uniform_int_distribution<int> randColorRed(0,255);
std::uniform_int_distribution<int> randColorGreen(0,255);
std::uniform_int_distribution<int> randColorBlue(0,255);

class Entity;


// == BASE COMPONENT CLASS ==
class Component
{
public:
Entity* mEntity{nullptr};

Component() {}
virtual ~Component() {}

virtual void updateComponent(const float& dt) {}
virtual void renderComponent(sf::RenderWindow& targetWin) {}

};


// == ENTITY CLASS ==
class Entity
{
private:
bool mAlive{true};
std::vector<std::unique_ptr<Component>> mComponentsContainer {};

public:
// takes in T(specified component type) <T>
// takes in any amount of specified arguments that will be forwarded to the Component constructor <TArgs>
template<typename T, typename... TArgs>
T& addComponent(TArgs&&... mArgs)
{
    // 1. allocate new component of type <T>, 
    T* component(new T(std::forward<TArgs>(mArgs)...));
    // 2. components entity owner is set like so
    component->mEntity = this;
    // 3. wrap the regular pointer into a smart pointer
    std::unique_ptr<Component> uC_Ptr{component};
    // 4. store the component ptr in our container
    mComponentsContainer.emplace_back(std::move(uC_Ptr));

    // return reference(so it's not lost to the container's ownership) to the component
    return *component;
}

// == accessor functions ==
bool isAlive() const { return mAlive; }
void destroyObj() { mAlive = false; } 

// == main loop functions ==
void updateObj(const float& dt)
{
    for (auto& component : mComponentsContainer)
    {
        component->updateComponent(dt);
    }
}

void renderObj(sf::RenderWindow& targetWin)
{
    for (auto& component : mComponentsContainer)
    {
        component->renderComponent(targetWin);
    }
}

};

// == ENTITY MANAGER CLASS ==
class EntityManager
{
private:
std::vector<std::unique_ptr<Entity>> mEntityContainer {};

public:
EntityManager() {}
~EntityManager() {}

Entity& addEntity()
{
    Entity* entity{new Entity{}};
    std::unique_ptr<Entity> uPtr{entity};
    mEntityContainer.emplace_back(std::move(uPtr));
    return *entity;
}

void updateManager(const float& dt)
{
    // remove all dead entities from mEntityContainer
    // 1. we have an iterator return a value in removedEntity
    // 2. using the lambda, it returns a dead entity into removedEntity
    // 3. erase dead entity from container
    mEntityContainer.erase
    (std::remove_if(mEntityContainer.begin(), mEntityContainer.end(),
    [](const std::unique_ptr<Entity>& entity)
        {
            return !entity->isAlive();
        }
    ),
    mEntityContainer.end());

    // update all entities in container
    for(auto& entity : mEntityContainer)
    {
        entity->updateObj(dt);
    } 
}

void renderManager(sf::RenderWindow& targetWin)
{
    for(auto& entity : mEntityContainer)
    {
        entity->renderObj(targetWin);
    }
}

};

struct CounterComponent : Component
{
    float counter;
    void updateComponent(const float& dt) override
    {
        counter += dt;
        std::cout << counter << std::endl;
    }
};

struct KillComponent : Component
{
    CounterComponent& cCounter;

    KillComponent(CounterComponent& mCounterComponent)
        : cCounter(mCounterComponent)
    {
    }

    void updateComponent(const float& mFT) override
    {
        if(cCounter.counter >= 10000) mEntity->destroyObj();
    }
};

struct ShapeComponent : Component
{
    sf::RectangleShape mShape;
    ShapeComponent()
    {
        mShape.setFillColor(sf::Color(randColorRed(gen),randColorGreen(gen),randColorBlue(gen),255));
        mShape.setSize(sf::Vector2f(10.0f,10.0f));
        mShape.setPosition(randPosx(gen),randPosy(gen));
    }

    void renderComponent(sf::RenderWindow& targetWin) override
    {
        targetWin.draw(this->mShape);
    }
    
};

int main()
{
    sf::RenderWindow mainWindow(sf::VideoMode(920,920),"ECS Test",sf::Style::Titlebar | sf::Style::Close);
    EntityManager manager;


    // We create an entity and get a reference to it:
    auto& entity(manager.addEntity());

    // We create components:
    auto& cCounter(entity.addComponent<CounterComponent>());
    auto& cKill(entity.addComponent<KillComponent>(cCounter));
    auto& cShape(entity.addComponent<ShapeComponent>());


    while(mainWindow.isOpen())
    {
        mainWindow.clear();
        manager.updateManager(1.f);
        manager.renderManager(mainWindow);
        mainWindow.display();
    }
    
}
