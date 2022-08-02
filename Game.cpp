#include "Game.hpp"

// == PRIVATE ==

// == INITIALIZER FUNCTIONS
void Game::initVars()
{
    this->mainWindow = nullptr;
    this->endGame = false;
}

void Game::initFonts()
{
    this->font.loadFromFile("fonts/Perfect DOS VGA 437 Win.ttf");
}

void Game::initWindow()
{
    this->videoMode.height = 640;
    this->videoMode.width= 920;
    this->mainWindow = new sf::RenderWindow(this->videoMode, "Test", sf::Style::Titlebar | sf::Style::Close);
    this->mainWindow->setFramerateLimit(120);
}

void Game::initUIText()
{
    this->uiText.setFont(this->font);
    this->uiText.setCharacterSize(20);
    this->uiText.setFillColor(sf::Color::White);
    this->uiText.setString("Test.");
}

// == PUBLIC ==
Game::Game()
{
    // default constructor
    this->initVars();
    this->initWindow();
    this->initFonts();
    this->initUIText(); 
}

Game::~Game()
{
    // destructor
    delete this->mainWindow;
}

// == ACCESSOR FUNCTIONS ==
const bool Game::isRunning() const
{
    return this->mainWindow->isOpen();
}

// check game state
const bool Game::getGameState() const
{
    return this->endGame;
}

// for delta time (seconds)
float Game::getTimeElapsedSeconds()
{
    return this->clock.getElapsedTime().asSeconds();
}

// for delta time (milliseconds)
float Game::getTimeElapsedMilliseconds()
{
    return this->clock.getElapsedTime().asMilliseconds();
}

// == GAME LOOP FUNCTIONS ==
// get system events 
void Game::pollEvents()
{
    // while there is a flow of pending events
    // (we pass in an sf::Event variable)
    while(this->mainWindow->pollEvent(ev))
    {
        // check for event type
        switch(ev.type)
        {
            // window is closed
            case sf::Event::Closed:
            {
                this->mainWindow->close();
                break;
            }

            // if esc key is pressed, close window
            case sf::Event::KeyPressed:
            {
                if(ev.key.code == sf::Keyboard::Escape)
                {
                    this->mainWindow->close();
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

// updates FPS-count for now
void Game::updateUIText(float dt)
{
    float currentTime = clock.restart().asSeconds();
    float FPS = 1.0f / dt;
    std::stringstream ss;
    ss << "FPS: " << FPS << '\n';

    this->uiText.setString(ss.str());
}

// update frame
void Game::updateAll(float dt)
{
    this->pollEvents();
    this->updateUIText(dt);
}

void Game::renderUIText(sf::RenderTarget& targetWin)
{
    targetWin.draw(this->uiText);
}

// render new frame
void Game::renderAll()
{
    // 1- clear old frame
    this->mainWindow->clear();

    // 2- draw objects on window
    this->renderUIText(*this->mainWindow);

    // 3- display
    this->mainWindow->display();  
}
