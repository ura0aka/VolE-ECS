#include "Game.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <cstdint>
#include <bitset>
#include <cassert>

// == For testing ==
std::default_random_engine gen;
std::uniform_real_distribution<float> randPosx(0.0f, 900.0f);
std::uniform_real_distribution<float> randPosy(0.0f, 900.0f);
std::uniform_int_distribution<int> randColorRed(0,255);
std::uniform_int_distribution<int> randColorGreen(0,255);
std::uniform_int_distribution<int> randColorBlue(0,255);

class Component;
class EntityManager;
class Entity;


// == COMPONENT ID SYSTEM ==
using ComponentID = std::uint32_t;
constexpr std::size_t maxComponents{32};

// == group variables ==
using GroupID = std::uint32_t;
constexpr std::uint32_t maxGroups{32};
using GroupBitset = std::bitset<maxGroups>;

using ComponentBitset = std::bitset<maxComponents>;
using ComponentArray = std::array<Component*, maxComponents>;

inline ComponentID genUComponentID() noexcept
{
    // generate a unique id for a component 
    // (this gets called in the getComponentTypeID function -> when Entity::addComponent() is called)
    static ComponentID lastID{0u};
    return lastID++;
}

template<typename T> inline ComponentID getComponentTypeID() noexcept
{
    // for each unique component type, the template will be instanciated
    // only once for each type of component thus, creating a unique ID

    // make sure getComponentTypeID only gets called with 'T' that inherits from the Component class
    // (std::is_base_of provides ::value = TRUE if the derived is part of the given base class)
    static_assert(std::is_base_of<Component, T>::value && "ERROR: T must inherit from base Component class.");
    static ComponentID typeID{genUComponentID()};

    // subsequent calls with the same component type will return the same ID
    return typeID;
}


// == BASE COMPONENT CLASS ==
class Component
{
public:
Entity* mEntity{nullptr};

// for any component that is dependant on other component types
// (makes composition easiers)
virtual void initComponent() {}

Component() {}
virtual ~Component() {}

void setOwnership(Entity* eOwner)
{
    this->mEntity = eOwner;
}

virtual void updateComponent(const float& dt) {}
virtual void renderComponent(sf::RenderWindow& targetWin) {}

};


// == ENTITY CLASS ==
class Entity
{
private:

EntityManager& mManager;

bool mAlive{true};
std::vector<std::unique_ptr<Component>> mComponentsContainer {};

ComponentArray mComponentArray {}; // stores the component pointer
ComponentBitset mComponentBitset {}; // stores the ID of a particular component

GroupBitset mGroupBitset {};

public:
// == CONSTRUCTOR/DESTRUCTOR ==
Entity(EntityManager& manager) : mManager{manager} {}
~Entity() {}

template<typename T> bool hasComponent() const
{
    // check if entity possesses a component of type 'T'
    // (bitset returns the value (true/false) of given index, which happens to be the unique ID)
    return mComponentBitset[getComponentTypeID<T>()];
}

// takes in T(specified component type) <T>
// takes in any amount of specified arguments that will be forwarded to the Component constructor <TArgs>
template<typename T, typename... TArgs>
T& addComponent(TArgs&&... mArgs)
{
    assert(!hasComponent<T>() && "ERROR: entity already owns this component.");

    // 1. allocate new component of type <T>, 
    T* component(new T(std::forward<TArgs>(mArgs)...));
    // 2. components entity owner is set like so
    component->setOwnership(this);
    // 3. wrap the regular pointer into a smart pointer
    std::unique_ptr<Component> uC_Ptr{component};
    // 4. store the component smart_ptr in our container
    mComponentsContainer.emplace_back(std::move(uC_Ptr));

    // add a component of type 'T' at mComponentArray's index -> (unique ID) &
    // set the component's bitset (depending on its unique ID)
    mComponentArray[getComponentTypeID<T>()] = component;
    mComponentBitset[getComponentTypeID<T>()] = true;

    component->initComponent();
    // return reference (so it's not lost to the container's ownership) to the component
    return *component;
}

// == GROUP MANAGEMENT ==
bool hasGroup(GroupID group) const noexcept
{
    return mGroupBitset[group];
}

void addGroup(GroupID group) noexcept;

void deleteGroup (GroupID group) noexcept
{
    mGroupBitset[group] = false;
}


// == accessor functions ==
bool isAlive() const { return mAlive; }
void destroyObj() { mAlive = false; }

template<typename T> T& getComponent() const
{
    // retrieve pointer to given component of type 'T' from mComponentArray
    assert(hasComponent<T>() && "ERROR: Component does not exist.");
    auto ptr{mComponentArray[getComponentTypeID<T>()]};
    return *static_cast<T*>(ptr);
} 

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
std::array<std::vector<Entity*>, maxGroups> mGroupedEntities {};

public:
EntityManager() {}
~EntityManager() {}

Entity& addEntity()
{
    // 1. create new entity (add on the heap and assign pointer to it)
    Entity* entity{new Entity{*this}};
    // 2. wrap pure pointer into smart pointer
    std::unique_ptr<Entity> uPtr{entity};
    // 3. place smart pointer -> entity obj in container
    mEntityContainer.emplace_back(std::move(uPtr));


    return *entity;
}

void addToGroup(Entity* entity, GroupID group)
{
    mGroupedEntities[group].emplace_back(entity);
}

std::vector<Entity*>& getEntitiesByGroup(GroupID group)
{
    return mGroupedEntities[group];
}

// main loop functions
void updateManager(const float& dt)
{
    for(auto i (0u); i < maxGroups; ++i)
    {
        auto& eV{mGroupedEntities[i]};

        eV.erase
        (std::remove_if(eV.begin(), eV.end(),
        [i](Entity* entity)
        {
            return !entity->isAlive() || !entity->hasGroup(i);
        }),
        eV.end()); 
    }

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

    //std::cout << "no. of entities: " << mEntityContainer.size() <<  std::endl;

}

void renderManager(sf::RenderWindow& targetWin)
{
    for(auto& entity : mEntityContainer)
    {
        entity->renderObj(targetWin);
    }
}

};

void Entity::addGroup(GroupID group) noexcept
{
    mGroupBitset[group] = true;
    mManager.addToGroup(this,group);
}

// == COMPONENTS ==
struct CounterComponent : Component
{
    float counter;
    void updateComponent(const float& dt) override
    {
        counter += dt;
        //std::cout << counter << std::endl;
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

    const float getPos()
    {
        return mShape.getPosition().y;
    }

    void updateComponent(const float& dt)
    {
        mShape.move(0.0f, 200.0f * dt);
    }

    void renderComponent(sf::RenderWindow& targetWin) override
    {
        targetWin.draw(this->mShape);
    }
    
};

struct KillComponent : Component
{
    CounterComponent* cCounter;
    ShapeComponent* cShape;

    void initComponent() override
    {
        cCounter = &mEntity->getComponent<CounterComponent>();
        cShape = &mEntity->getComponent<ShapeComponent>();
    }

    void updateComponent(const float& dt) override
    {
        if(cCounter->counter >= 2) mEntity->destroyObj();
    }
};



int main()
{
    enum VOLEGroup : std::size_t
    {
        Player,
        NPC
    };

    sf::RenderWindow mainWindow(sf::VideoMode(920,920),"ECS Test",sf::Style::Titlebar | sf::Style::Close);
    mainWindow.setFramerateLimit(120);

    sf::Clock clock;
    
    float spawnTimerMax = 5.0f;
    float spawnTimer = spawnTimerMax;
    
    float UPS = 1.0f / 120.0f;
    float lastFrameTime = 0.0f;
    float dt = 0.0f;

    EntityManager manager;

    while(mainWindow.isOpen())
    {
        float currentFrameTime = clock.getElapsedTime().asSeconds();
        dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        auto& entity (manager.addEntity());
        auto& cCounter(entity.addComponent<CounterComponent>());
        auto& cShape(entity.addComponent<ShapeComponent>());
        auto& cKill(entity.addComponent<KillComponent>());

        entity.addGroup(VOLEGroup::Player);
        

        if(spawnTimer >= spawnTimerMax)
        {
            for(int i {0}; i < 1; ++i)
            {
                auto& entity(manager.addEntity());
                auto& cCounter(entity.addComponent<CounterComponent>());
                auto& cShape(entity.addComponent<ShapeComponent>());
                auto& cKill(entity.addComponent<KillComponent>());

                entity.addGroup(VOLEGroup::NPC);
                

                spawnTimer = 0.0f;
            }
        }
        else
        {
            spawnTimer += 1.0f;
        }

        mainWindow.clear();
        if(dt >= UPS)
        {
            manager.updateManager(dt);
            dt -= UPS;
        }
        
        auto& player(manager.getEntitiesByGroup(Player));
        auto& npcs(manager.getEntitiesByGroup(NPC));

        
        manager.renderManager(mainWindow);
        mainWindow.display();
    }
    
}
