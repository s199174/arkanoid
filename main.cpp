#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;

//Œcie¿ki do plików
const string FONT_PATH = "C:/Users/adamk/OneDrive/Pulpit/Projekt Informatyka/arkanoid/x64/Debug/arial.ttf";
const string BACKGROUND_PATH = "C:/Users/adamk/OneDrive/Pulpit/Projekt Informatyka/arkanoid/arkanoid/space.png";
const string IRREGULAR_PATH = "C:/Users/adamk/OneDrive/Pulpit/Projekt Informatyka/arkanoid/arkanoid/ufo.png";

// Wymiary i parametry gry
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_HEIGHT = 20.0;
const float BALL_RADIUS = 10.0;
const float BLOCK_WIDTH = 60.0;
const float BLOCK_HEIGHT = 20.0;
const int MENU_NUMBER = 5;
const int NUMBER_OF_LEVELS = 3;
const int WALL_HEIGHT = 20;
const int WALL_WIDTH = 100;

float PADDLE_WIDTH = 100.0;
float ball_speed = -2.0;
float ufo_speed = 1.0;

//---------------------------Struktury---------------------------------------
// Struktura dane gracza
struct Player {
    string name;
    int score;
};

//Struktura stanu gry 
struct GameState {
    sf::Vector2f paddlePosition;
    sf::Vector2f ballPosition;
    vector<bool> blocksDestroyed;
};
//---------------------------------------------------------------------------

//----------------------------------MENU-------------------------------------
// Klasa menu
class Menu {
public:
    Menu(float width, float height);

    void draw(sf::RenderWindow& window);
    void MoveUp();
    void MoveDown();
    int getSelectedItemIndex() const { return selectedItemIndex; }

private:
    int selectedItemIndex;
    sf::Font font;
    sf::Text text[MENU_NUMBER];
    sf::Text arkanoid;
    sf::Text help;
    sf::Text ESC;
};

Menu::Menu(float width, float height) {
    selectedItemIndex = 0;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        exit(EXIT_FAILURE);
    }

    // Pozycje i treœci menu
    string items[MENU_NUMBER] = { "Graj", "Pomoc", "Opcje", "Statystyki", "Wyjscie" };
    for (int i = 0; i < MENU_NUMBER; i++) {
        sf::Color color;
        if (i == selectedItemIndex) {
            color = sf::Color::Red;
        }
        else {
            color = sf::Color::White; 
        }

        text[i].setFont(font);
        text[i].setString(items[i]);
        text[i].setFillColor(color);
        text[i].setPosition(sf::Vector2f(width / 2 - 50, height / (MENU_NUMBER + 1) * (i + 1)));
    }

    arkanoid.setString("Arkanoid");
    arkanoid.setFont(font);
    arkanoid.setFillColor(sf::Color::Green);
    arkanoid.setCharacterSize(40);
    arkanoid.setPosition((WINDOW_WIDTH / 2)-100, 20);

    help.setString("Pomoc - F1");
    help.setFont(font);
    help.setFillColor(sf::Color::Green);
    help.setCharacterSize(20);
    help.setPosition(10, 500);

    ESC.setString("Wyjscie - ESC");
    ESC.setFont(font);
    ESC.setFillColor(sf::Color::Green);
    ESC.setCharacterSize(20);
    ESC.setPosition(10, 550);
    
}

void Menu::draw(sf::RenderWindow& window) {
    window.draw(arkanoid);
    window.draw(help);
    window.draw(ESC);
    for (int i = 0; i < MENU_NUMBER; i++) {
        window.draw(text[i]);
    }
}

void Menu::MoveUp() {
    if (selectedItemIndex - 1 >= 0) { 
        text[selectedItemIndex].setFillColor(sf::Color::White);
        selectedItemIndex--; 
        text[selectedItemIndex].setFillColor(sf::Color::Red);
    }
}

void Menu::MoveDown() {
    if (selectedItemIndex + 1 < MENU_NUMBER) {
        text[selectedItemIndex].setFillColor(sf::Color::White);
        selectedItemIndex++;
        text[selectedItemIndex].setFillColor(sf::Color::Red);
    }
}
//---------------------------------------------------------------------------

//-----------------------------GRA-------------------------------------------
//---------------------------------KLASY-------------------------------------
// Klasa reprezentuj¹ca paletkê
class Paddle {
public:
    sf::RectangleShape shape; 
    float paddle_speed = 5.0;

    Paddle(float x, float y) {
        shape.setSize({ PADDLE_WIDTH, PADDLE_HEIGHT });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Blue);
    }

    // Ruch w lewo
    void moveLeft() {
        if (shape.getPosition().x > 0) 
            shape.move(-paddle_speed, 0); 
    }

    // Ruch w prawo
    void moveRight() {
        if (shape.getPosition().x + PADDLE_WIDTH < WINDOW_WIDTH)
            shape.move(paddle_speed, 0);
    }
};

// Klasa pi³ki
class Ball {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity{ ball_speed, ball_speed };

    Ball(float x, float y) {
        shape.setRadius(BALL_RADIUS);
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Red);
    }

    // Aktualizacja pozycji i kolizja
    void update() {
        shape.move(velocity); 

        // Odbicie od œcian bocznych
        if (shape.getPosition().x <= 0 || shape.getPosition().x + BALL_RADIUS * 2 >= WINDOW_WIDTH)
            velocity.x = -velocity.x;

        // Odbicie od górnej œciany
        if (shape.getPosition().y <= 0)
            velocity.y = -velocity.y;
    }
};

// Klasa bloczka
class Block {
public:
    sf::RectangleShape shape; 
    bool destroyed = false;

    Block(float x, float y) {
        shape.setSize({ BLOCK_WIDTH, BLOCK_HEIGHT });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Green);
    }
};

// Klasa przeszkody (rózne scenerie gry)
class Obstacle {
public:
    sf::RectangleShape shape;

    Obstacle(float x, float y) {
        shape.setSize({ WALL_WIDTH, WALL_HEIGHT });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Magenta);
    }

    void draw(sf::RenderWindow& window) const { 
        window.draw(shape);
    }
};

vector<Obstacle> obstacles; // Globalna lista przeszkód

// Klasa nieregularnego kszta³tu
class Irregular {
private:
    sf::Sprite sprite;       
    sf::Texture texture;     
    sf::Vector2f velocity{ufo_speed, ufo_speed};

public:
    Irregular(float x, float y) {
        if (!texture.loadFromFile(IRREGULAR_PATH)) {
            cerr << "Error loading texture from " << IRREGULAR_PATH << endl;
            return;
        }
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, 144, 94));
        sprite.setPosition(x,y);              
    }

    void update(const sf::RenderWindow& window) {
        sprite.move(velocity);

        sf::FloatRect bounds = sprite.getGlobalBounds();

        if (bounds.left <= 0 || bounds.left + bounds.width >= window.getSize().x) {
            velocity.x = -velocity.x;
        }
        if (bounds.top <= 0 || bounds.top + bounds.height >= window.getSize().y) {
            velocity.y = -velocity.y; 
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

//Klasa wyboru opcji Tak/Nie
class GameDialog {
protected:
    sf::Font font;
    sf::Text title;
    sf::Text optionYes;
    sf::Text optionNo;
    bool isYesSelected = true;

public:
    GameDialog(const string& dialogTitle, const sf::Vector2f& titlePos, const sf::Vector2f& yesPos, const sf::Vector2f& noPos) {
        font.loadFromFile(FONT_PATH);
        if (!font.loadFromFile(FONT_PATH)) {
            cerr << "Error loading font" << endl;
            exit(EXIT_FAILURE);
        }

        title.setFont(font);
        title.setString(dialogTitle);
        title.setCharacterSize(24);
        title.setFillColor(sf::Color::White);
        title.setPosition(titlePos);

        optionYes.setFont(font);
        optionYes.setString("TAK");
        optionYes.setCharacterSize(20);
        optionYes.setFillColor(sf::Color::Red);
        optionYes.setPosition(yesPos);

        optionNo.setFont(font);
        optionNo.setString("NIE");
        optionNo.setCharacterSize(20);
        optionNo.setFillColor(sf::Color::White);
        optionNo.setPosition(noPos);
    }

    void selection() {
        if (isYesSelected) {
            isYesSelected = false; // Prze³¹cz na NIE
            optionYes.setFillColor(sf::Color::White);
            optionNo.setFillColor(sf::Color::Red);
        }
        else {
            isYesSelected = true; // Prze³¹cz na TAK
            optionYes.setFillColor(sf::Color::Red);
            optionNo.setFillColor(sf::Color::White);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(title);
        window.draw(optionYes);
        window.draw(optionNo);
    }

    bool getIsYesSelected() const {
        return isYesSelected;
    }
};

// Dziedziczenie
class EndGame : public GameDialog {
public:
    EndGame() : GameDialog("Czy chcesz zakonczyc gre?", { 250, 200 }, { 300, 300 }, { 400, 300 }) {}
};

class PlayAgain : public GameDialog {
public:
    PlayAgain() : GameDialog("Czy chcesz zagrac ponownie?", { 250, 400 }, { 300, 500 }, { 400, 500 }) {}
};
//---------------------------------------------------------------------------


//---------------------------FUNKCJE-----------------------------------------
//Kolizje
bool checkCollision(const sf::RectangleShape& rect, sf::CircleShape& ball, sf::Vector2f& ballVelocity) {
    sf::FloatRect ballBounds = ball.getGlobalBounds();
    sf::FloatRect rectBounds = rect.getGlobalBounds();

    if (ballBounds.intersects(rectBounds)) {
        sf::Vector2f ballCenter(ballBounds.left + ballBounds.width / 2, ballBounds.top + ballBounds.height / 2);
        sf::Vector2f rectCenter(rectBounds.left + rectBounds.width / 2, rectBounds.top + rectBounds.height / 2);

        //roznica srodkow mas
        float deltaX = ballCenter.x - rectCenter.x; 
        float deltaY = ballCenter.y - rectCenter.y; 

        //obliczenie glebokosci 
        float intersectX = (rectBounds.width / 2 + ballBounds.width / 2) - abs(deltaX);
        float intersectY = (rectBounds.height / 2 + ballBounds.height / 2) - abs(deltaY);

        //kierunek kolizji i odbicie
        if (intersectX < intersectY) { 
            ballVelocity.x = -ballVelocity.x;
            float newX = ball.getPosition().x;
            if (deltaX > 0) {
                newX += intersectX;
            }
            else {
                newX -= intersectX;
            }
            ball.setPosition(newX, ball.getPosition().y);

        }
        else { 
            ballVelocity.y = -ballVelocity.y;
            float newY = ball.getPosition().y; 

            if (deltaY > 0) {
                newY += intersectY;
            }
            else {
                newY -= intersectY;
            }
            ball.setPosition(ball.getPosition().x, newY);
        }
        return true;
    }
    return false;
}

// Funkcja kolizji pi³ki z paletk¹ i bloczkami
void handleCollisions(Ball& ball, Paddle& paddle, vector<Block>& blocks, int& score) {
    checkCollision(paddle.shape, ball.shape, ball.velocity);

    for (int i = 0; i < blocks.size(); ++i) {
        if (!blocks[i].destroyed && checkCollision(blocks[i].shape, ball.shape, ball.velocity)) {
            blocks[i].destroyed = true; 
            score++; 
        }
    }

    for (int i = 0; i < obstacles.size(); ++i) {
        checkCollision(obstacles[i].shape, ball.shape, ball.velocity);
    }

}

// Funkcja ekranu koñca gry
void renderEndScreen(sf::RenderWindow* window, const string* message, const string* timeMessage) { //Przekazywanie danych do funkcji przez wskaŸnik
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    sf::Text mainMessage(*message, font, 50);
    mainMessage.setFillColor(sf::Color::White);
    mainMessage.setPosition(200, 150);

    sf::Text timeText(*timeMessage, font, 30);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(200, 250);

    PlayAgain again;

    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
                return;
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Left:
                case sf::Keyboard::Right:
                    again.selection();
                    break;
                case sf::Keyboard::Enter:
                    if (again.getIsYesSelected()) return;
                    else window->close();
                    break;
                }
            }
        }

        window->clear(sf::Color::Black);
        window->draw(mainMessage);
        window->draw(timeText);
        again.draw(*window);
        window->display();
    }
}

// Funkcja ekranu pomocy
void showHelpScreen(sf::RenderWindow& window) {
    sf::Font font;
    font.loadFromFile(FONT_PATH);

    sf::Text helpText("Instrukcja:\n-Uzyj strzalek, aby poruszac paletka.\n-Zbij wszystkie bloczki!\n-Naciskajac 'S' mozna zapisac gre\n-Naciskajac 'L' mozna zaladowac zapisana gre\n-W opcji 'Options' mozna wybrac poziom trudnosci\n-W opcji 'Stats' znajduja sie ostatnie wyniki graczy\n\nNacisnij ENTER, aby powrocic.", font, 20);
    helpText.setFillColor(sf::Color::White);
    helpText.setPosition(50, 50);

    window.clear(sf::Color::Black);
    window.draw(helpText);
    window.display();

    while (true) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                return;
        }
    }
}

// Funkcje zapisu i odczytu gry
void saveGameState(const Paddle& paddle, const Ball& ball, const vector<Block>& blocks) {
    ofstream file("gamestate.txt", ios::out);
    if (!file) {
        cerr << "Error: Could not open file for saving." << endl;
        return;
    }
    if (file.is_open()) {
        file << paddle.shape.getPosition().x << " " << paddle.shape.getPosition().y << "\n"; 
        file << ball.shape.getPosition().x << " " << ball.shape.getPosition().y << "\n"; 
        for (int i = 0; i < blocks.size(); ++i) {
            file << blocks[i].destroyed << " ";
        }
        file << "\n";
        file.close();
        cout << "Game state saved." << endl;
    }
}

void loadGameState(Paddle& paddle, Ball& ball, vector<Block>& blocks) {
    ifstream file("gamestate.txt", ios::in); 
    if (!file) { 
        cerr << "Error: Could not open file for loading." << endl;
        return;
    }
    if (file.is_open()) {
        sf::Vector2f paddlePosition;
        sf::Vector2f ballPosition;

        file >> paddlePosition.x >> paddlePosition.y; 
        file >> ballPosition.x >> ballPosition.y;

        paddle.shape.setPosition(paddlePosition);
        ball.shape.setPosition(ballPosition);

        for (int i = 0; i < blocks.size(); ++i) {
            bool destroyed;
            file >> destroyed;
            blocks[i].destroyed = destroyed;
        }

        file.close();
        cout << "Game state loaded." << endl;
    }
}

// Tablica struktur do wyników graczy
vector<Player> players;

// Funkcja do wczytywania wyników graczy z pliku
void loadHighScores() {
    players.clear();
    ifstream file("highscores.txt");
    if (file.is_open()) {
        Player player;
        while (file >> player.name >> player.score) {
            players.push_back(player);
        }
        file.close();
    }
}

// Funkcja do zapisywania wyników graczy do pliku
void saveHighScores() {
    ofstream file("highscores.txt", ios::trunc); 
    if (file.is_open()) { 
        for (int i = 0; i < players.size(); ++i) {
            file << players[i].name << " " << players[i].score << endl;
        }
        file.close();
    }
}

// Funkcja dodaj¹ca nowy wynik do rankingu
void addHighScore(const string& playerName, int score, double elapsedTime) {
    players.push_back({ playerName, score }); 

    sort(players.begin(), players.end(), [](const Player& a, const Player& b) 
        {return a.score > b.score;});

    if (players.size() > 10) {
        players.pop_back();
    }

    ofstream file("highscores.txt", ios::app); 
    if (file.is_open()) {
        file << playerName << ", score: " << score << ", time " << elapsedTime << "s" << endl;
        file.close();
    }
}

// Funkcja wyœwietlaj¹ca statystyki graczy
void showStatsScreen(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    sf::Text statsTitle("High Scores", font, 30);
    statsTitle.setFillColor(sf::Color::White);
    statsTitle.setPosition(WINDOW_WIDTH / 2 - 100, 50);

    sf::Text backTitle("Nacisnij 'Enter' aby wrocic do menu glownego", font, 15);
    backTitle.setFillColor(sf::Color::White);
    backTitle.setPosition(100, WINDOW_HEIGHT-50);

    vector<string> statsLines;
    ifstream file("highscores.txt");
    string line;
    while (getline(file, line)) {
        statsLines.push_back(line);
    } 
    file.close();

    vector<sf::Text> statsText;
    for (int i = 0; i < statsLines.size(); ++i) {
        sf::Text text(statsLines[i], font, 20);
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 100 + i * 30);
        statsText.push_back(text);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                return; 
            }
        }

        window.clear(sf::Color::Black);
        window.draw(statsTitle);
        window.draw(backTitle);
        for (int i = 0; i < statsText.size(); ++i) {
            window.draw(statsText[i]);
        }
        window.display();
    }
}

// Funckja t³a
void renderBackground(sf::RenderWindow& window) {
    static sf::Texture texture;
    static sf::Sprite background;

    if (!texture.getSize().x) {
        if (!texture.loadFromFile(BACKGROUND_PATH)) {
            cerr << "Error loading background texture" << endl;
            return;
        }
        background.setTexture(texture);
    }

    window.draw(background);
}


//---------------------------------------------------
// Funkcja runGame - G³ówna pêtla gry
void runGame(sf::RenderWindow& window) {
    Paddle paddle(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 50);
    Ball ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 100);

    Irregular irregular( 100, 100 );

    time_t pauseStartTime = 0; // Czas rozpoczêcia pauzy
    double totalPauseTime = 0; // Czas spêdzony na pauzach

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    int score = 0; 

    sf::Text scoreText, timeText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    timeText.setFont(font);
    timeText.setCharacterSize(20);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(100, 10);

    EndGame endgame;

    vector<Block> blocks;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            blocks.emplace_back(i * (BLOCK_WIDTH + 10) + 50, j * (BLOCK_HEIGHT + 10) + 50);
        }
    }

    time_t startTime = time(NULL); // Czas rozpoczêcia gry
    bool gameRunning = true; 
    bool isPaused = false;

    while (window.isOpen() && gameRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1) {
                pauseStartTime = time(NULL); // Rozpoczêcie pauzy- ekran pomocy
                showHelpScreen(window);
                totalPauseTime += difftime(time(NULL), pauseStartTime); // £¹czny czas pauzy
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                pauseStartTime = time(NULL); // Rozpoczêcie pauzy- ekran koñca gry
                isPaused = true;
                while (isPaused && window.isOpen()) {
                    sf::Event pauseEvent;
                    while (window.pollEvent(pauseEvent)) {
                        if (pauseEvent.type == sf::Event::Closed) {
                            window.close();
                        }
                        if (pauseEvent.type == sf::Event::KeyPressed) {
                            switch (pauseEvent.key.code) {
                            case sf::Keyboard::Left:
                            case sf::Keyboard::Right:
                                endgame.selection();
                                break;
                            case sf::Keyboard::Enter:
                                if (endgame.getIsYesSelected()) {
                                    window.close();
                                }
                                else {
                                    isPaused = false;
                                    totalPauseTime += difftime(time(NULL), pauseStartTime); // Dodanie czasu pauzy
                                }
                                break;
                            }
                        }
                    }

                    window.clear();
                    endgame.draw(window);
                    window.display();
                }
            }
        }

        if (!isPaused) {
            // Sterowanie paletk¹
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                paddle.moveLeft();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                paddle.moveRight();

            // Aktualizacja pozycji pi³ki
            ball.update();
            irregular.update(window);
        }

        // Obs³uga kolizji i aktualizacja wyniku
        handleCollisions(ball, paddle, blocks, score);

        // Sprawdzenie, czy pi³ka dotknê³a dolnej granicy (koniec gry)
        if (ball.shape.getPosition().y > WINDOW_HEIGHT) {
            time_t endTime = time(NULL);
            int elapsedSeconds = static_cast<int>(difftime(endTime, startTime) - totalPauseTime);
            string timeMessage = "Czas gry: " + to_string(elapsedSeconds) + " sekund";

            // Dodanie wyniku do rankingu
            addHighScore("Player", score, elapsedSeconds);
            string gameOverMessage = "GAME OVER";
            renderEndScreen(&window, &gameOverMessage, &timeMessage); //Przekazywanie danych do funkcji przez wskaŸnik
            gameRunning = false;
            break;
        }

        // Sprawdzenie, czy wszystkie bloki zosta³y zniszczone
        bool allDestroyed = true;

        for (int i = 0; i < blocks.size(); ++i) {
            if (!blocks[i].destroyed) {
                allDestroyed = false;
                break;
            }
        }

        if (allDestroyed) {
            time_t endTime = time(NULL);
            int elapsedSeconds = static_cast<int>(difftime(endTime, startTime) - totalPauseTime);
            string timeMessage = "Czas gry: " + to_string(elapsedSeconds) + " sekund";

            addHighScore("Player", score, elapsedSeconds);
            string winMessage = "WIN";
            renderEndScreen(&window, &winMessage, &timeMessage); //Przekazywanie danych do funkcji przez wskaŸnik
            gameRunning = false;
            break;
        }


        // Renderowanie

        window.clear();

        renderBackground(window);

        irregular.draw(window);

        window.draw(paddle.shape);
        window.draw(ball.shape);

        //renderowanie bloczków
        for (int i = 0; i < blocks.size(); ++i) {
            if (!blocks[i].destroyed)
                window.draw(blocks[i].shape);
        }

        //renderowanie przeszkód
        for (int i = 0; i < obstacles.size(); ++i) {
            obstacles[i].draw(window);
        }

        // Wyœwietlanie wyniku i czasu na ekranie
        scoreText.setString("Score: " + to_string(score));
        int elapsedTime = static_cast<int>(difftime(time(NULL), startTime) - totalPauseTime);
        timeText.setString("Time: " + to_string(elapsedTime) + "s");
        window.draw(scoreText);
        window.draw(timeText);

        window.display();

        // Zapis/odczyt stanu gry
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            saveGameState(paddle, ball, blocks);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
            loadGameState(paddle, ball, blocks);
        }
    }
}

// Funkcja wyboru poziomu trudnoœci
void chooseLevel(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    sf::Text levelTitle("Choose Level", font, 50);
    levelTitle.setFillColor(sf::Color::White);
    levelTitle.setPosition(WINDOW_WIDTH / 2 - 150, 50);

    const vector<string> items = { "EASY", "MEDIUM", "HARD" };
    const vector<float> speeds = { -2.5f, -3.4f, -4.0f };
    const vector<float> paddlewidth = { 100.0f, 85.0f, 70.0f };
    vector<sf::Text> levels(items.size());
    int selectedLevel = 0;

    for (int i = 0; i < items.size(); i++) {
        sf::Color color;
        if (i == selectedLevel) {color = sf::Color::Red;}
        else {color = sf::Color::White;}
        levels[i].setFont(font);
        levels[i].setString(items[i]);
        levels[i].setFillColor(color);
        levels[i].setPosition(WINDOW_WIDTH / 2 - 100, 150 + i * 50);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    selectedLevel = (selectedLevel - 1 + items.size()) % items.size();
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    selectedLevel = (selectedLevel + 1) % items.size();
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                   
                    ball_speed = speeds[selectedLevel];
                    PADDLE_WIDTH = paddlewidth[selectedLevel];

                    obstacles.clear();
                    if (items[selectedLevel] == "MEDIUM") {
                        obstacles.emplace_back(WINDOW_WIDTH / 2 - WALL_WIDTH / 2, WINDOW_HEIGHT / 2 - WALL_HEIGHT / 2);
                    }
                    else if (items[selectedLevel] == "HARD") {
                        obstacles.emplace_back(WINDOW_WIDTH / 3 - WALL_WIDTH / 2, WINDOW_HEIGHT / 2 - WALL_HEIGHT / 2);
                        obstacles.emplace_back(2 * WINDOW_WIDTH / 3 - WALL_WIDTH / 2, WINDOW_HEIGHT / 2 - WALL_HEIGHT / 2);
                    }

                    return;
                }
            }
        }

        for (int i = 0; i < items.size(); i++) {
            sf::Color color;
            if (i == selectedLevel) {color = sf::Color::Red;}
            else {color = sf::Color::White;}
            levels[i].setFillColor(color);
        }

        window.clear(sf::Color::Black);
        window.draw(levelTitle);
        for (int i = 0; i < levels.size(); ++i) {
            window.draw(levels[i]);
        }
        window.display();
    }
}
//---------------------------------------------------------------------------


//---------------------------------G£ÓWNA PÊTLA-----------------------------
int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arkanoid");

    window.setFramerateLimit(60);

    loadHighScores();

    Menu menu(WINDOW_WIDTH, WINDOW_HEIGHT);

    bool isPaused = false;
    EndGame endgame;

    Paddle paddle(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 50);
    Ball ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    vector<Block> blocks;
    int score = 0;

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            blocks.emplace_back(i * (BLOCK_WIDTH + 10) + 50, j * (BLOCK_HEIGHT + 10) + 50);
        }
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1) {
                showHelpScreen(window);
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                isPaused = true;
                while (isPaused && window.isOpen()) {
                    sf::Event pauseEvent;
                    while (window.pollEvent(pauseEvent)) {
                        if (pauseEvent.type == sf::Event::Closed) {
                            window.close();
                        }
                        if (pauseEvent.type == sf::Event::KeyPressed) {
                            if (pauseEvent.key.code == sf::Keyboard::Left || pauseEvent.key.code == sf::Keyboard::Right) {
                                endgame.selection();
                            }
                            if (pauseEvent.key.code == sf::Keyboard::Enter) {
                                if (endgame.getIsYesSelected()) {
                                    window.close();
                                }
                                else {
                                    isPaused = false;
                                }
                            }

                        }
                    }

                    // Renderowanie menu pauzy
                    window.clear();
                    endgame.draw(window);
                    window.display();
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Up:
                    menu.MoveUp();
                    break;
                case sf::Keyboard::Down:
                    menu.MoveDown();
                    break;
                case sf::Keyboard::Enter:
                    switch (menu.getSelectedItemIndex()) {
                    case 0: runGame(window); break;
                    case 1: showHelpScreen(window); break;
                    case 2: chooseLevel(window); break;
                    case 3: showStatsScreen(window); break;
                    case 4: window.close(); break;
                    }
                    break;
                }

                
            }
        }

        window.clear();
        menu.draw(window);
        window.display();
    }

    return 0;
}