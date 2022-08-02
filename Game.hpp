#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <sstream>


#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

class Game
{

private:
    // == WINDOW VARIABLES ==
    sf::RenderWindow* mainWindow;
    sf::VideoMode videoMode;
    // == EVENT VARIABLES ==
    sf::Event ev;
    // == TIME VARIABLES ==
    sf::Clock clock;
    // == GAME OBJECTS ==

    // == GAME LOGIC ==
    bool endGame;
    // == RESOURCES ==
    sf::Font font;
    // == TEXT ==
    sf::Text uiText;

    // == INITIALIZER FUNCTIONS
    void initVars();
    void initWindow();
    void initFonts();
    void initUIText();


    public:
    // default constrtuctor
    Game();

    // destuctor
    ~Game();

    // == ACCESSOR FUNCTIONS ==
    const bool isRunning() const;
    const bool getGameState() const;
    float getTimeElapsedSeconds();
    float getTimeElapsedMilliseconds();

    // == GAME LOOP FUNCTIONS ==
    void pollEvents();
    void updateUIText(float dt);
    void updateAll(float dt);

    void renderUIText(sf::RenderTarget& targetWin);
    void renderAll();
};

#endif // GAME_H