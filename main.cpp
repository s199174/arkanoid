
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
const std::string FONT_PATH = "C:/Users/adamk/OneDrive/Pulpit/Projekt Informatyka/arkanoid/x64/Debug/arial.ttf";

using namespace std;

// Sta�e definiuj�ce wymiary i parametry gry
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_HEIGHT = 20.0f;
const float BALL_RADIUS = 10.0f;
const float BLOCK_WIDTH = 60.0f;
const float BLOCK_HEIGHT = 20.0f;
const int MAX_NUMBER_OF_ITEMS = 5;
const int NUMBER_OF_LEVELS = 3;

float PADDLE_WIDTH = 100.0f;
float ball_speed = -1.5f;

//---------------------------Struktury---------------------------------------
// Struktura przechowuj�ca dane gracza
struct Player {
    string name;
    int score;
};

//Struktura przechowuj�ca stan gry 
struct GameState {
    sf::Vector2f paddlePosition;
    sf::Vector2f ballPosition;
    sf::Vector2f ballVelocity;
    std::vector<bool> blocksDestroyed;
};
//---------------------------------------------------------------------------

//----------------------------------MENU-------------------------------------
//->->->->->->->->->

// Klasa reprezentuj�ca menu
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
    sf::Text text[MAX_NUMBER_OF_ITEMS];
    sf::Text arkanoid;
    sf::Text help;
    sf::Text ESC;
};

Menu::Menu(float width, float height) : selectedItemIndex(0) {
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja pozycji i tre�ci element�w menu
    string items[MAX_NUMBER_OF_ITEMS] = { "Play", "Help", "Options", "Stats", "Exit" };
    for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
        sf::Color color;
        if (i == selectedItemIndex) {
            color = sf::Color::Red; // Wybrany element menu na czerwono
        }
        else {
            color = sf::Color::White; // Pozosta�e elementy na bia�o
        }

        text[i].setFont(font);
        text[i].setString(items[i]);
        text[i].setFillColor(color); // Ustawienie koloru tekstu
        text[i].setPosition(sf::Vector2f(width / 2 - 50, height / (MAX_NUMBER_OF_ITEMS + 1) * (i + 1)));
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
    for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
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
    if (selectedItemIndex + 1 < MAX_NUMBER_OF_ITEMS) {
        text[selectedItemIndex].setFillColor(sf::Color::White);
        selectedItemIndex++;
        text[selectedItemIndex].setFillColor(sf::Color::Red);
    }
}
//---------------------------------------------------------------------------

//-----------------------------GRA-------------------------------------------
//---------------------------------KLASY-------------------------------------
// Klasa reprezentuj�ca paletk�
class Paddle {
public:
    sf::RectangleShape shape; // Graficzny kszta�t paletki
    float paddle_speed = 4.0f;       // Pr�dko�� poruszania si� paletki (float na ko�cu)

    // Konstruktor inicjalizuj�cy pozycj� i wygl�d paletki
    Paddle(float x, float y) {
        shape.setSize({ PADDLE_WIDTH, PADDLE_HEIGHT }); //nadanie wymiar�w z const
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Blue);
    }

    // Ruch w lewo
    void moveLeft() {
        if (shape.getPosition().x > 0) // wymiary okna zaczynaj sie od lewej strony //.getPosition- Pobiera aktualn� pozycj� obiektu graficznego w oknie gry
            shape.move(-paddle_speed, 0); 
    }

    // Ruch w prawo
    void moveRight() {
        if (shape.getPosition().x + PADDLE_WIDTH < WINDOW_WIDTH)
            shape.move(paddle_speed, 0);
    }
};

// Klasa reprezentuj�ca pi�k�
class Ball {
public:
    sf::CircleShape shape;       // Graficzny kszta�t pi�ki
    sf::Vector2f velocity{ ball_speed, ball_speed }; // Pr�dko�� pi�ki

    // Konstruktor inicjalizuj�cy pozycj� i wygl�d pi�ki
    Ball(float x, float y) {
        shape.setRadius(BALL_RADIUS);
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Red);
    }

    // Aktualizacja pozycji pi�ki i obs�uga kolizji ze �cianami
    void update() {
        shape.move(velocity); //Przesuwa obiekt graficzny o wektor velocity

        // Odbicie od �cian bocznych
        if (shape.getPosition().x <= 0 || shape.getPosition().x + BALL_RADIUS * 2 >= WINDOW_WIDTH)
            velocity.x = -velocity.x;

        // Odbicie od g�rnej �ciany
        if (shape.getPosition().y <= 0)
            velocity.y = -velocity.y;
    }
};

// Klasa reprezentuj�ca bloczek
class Block {
public:
    sf::RectangleShape shape; // Graficzny kszta�t bloczka
    bool destroyed = false;   // Czy bloczek zosta� zniszczony?

    // Konstruktor inicjalizuj�cy pozycj� i wygl�d bloczka
    Block(float x, float y) {
        shape.setSize({ BLOCK_WIDTH, BLOCK_HEIGHT });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Green);
    }
};

class GameDialog {
protected:
    sf::Font font;
    sf::Text title;
    sf::Text optionYes;
    sf::Text optionNo;
    bool isYesSelected = true;

public:
    GameDialog(const std::string& dialogTitle, const sf::Vector2f& titlePos, const sf::Vector2f& yesPos, const sf::Vector2f& noPos) {
        font.loadFromFile(FONT_PATH);

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
        isYesSelected = !isYesSelected;
        if (isYesSelected) {
            optionYes.setFillColor(sf::Color::Red);
            optionNo.setFillColor(sf::Color::White);
        }
        else {
            optionYes.setFillColor(sf::Color::White);
            optionNo.setFillColor(sf::Color::Red);
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
// ----------------------------------
// Funkcja checkCollision - Kolizje
// ----------------------------------
// Sprawdza, czy pi�ka koliduje z prostok�tnymi obiektami (paletk�, bloczkami).
// Oblicza kierunek i g��boko�� kolizji, aby odpowiednio zmieni� kierunek ruchu pi�ki.
bool checkCollision(const sf::RectangleShape& rect, sf::CircleShape& ball, sf::Vector2f& ballVelocity) {
    sf::FloatRect ballBounds = ball.getGlobalBounds(); // Granice pi�ki
    sf::FloatRect rectBounds = rect.getGlobalBounds(); // Granice prostok�ta

    //gdy pilka intersects(przetnie) pole prostokata
    if (ballBounds.intersects(rectBounds)) {
        // Obliczenie �rodka pi�ki i prostok�ta
        sf::Vector2f ballCenter(ballBounds.left + ballBounds.width / 2, ballBounds.top + ballBounds.height / 2);
        sf::Vector2f rectCenter(rectBounds.left + rectBounds.width / 2, rectBounds.top + rectBounds.height / 2);

        //roznica srodkow mas
        //okre�laj�, jak bardzo �rodek pi�ki jest przesuni�ty wzgl�dem �rodka prostok�ta w osi X i Y. Pomaga to ustali� kierunek kolizji.
        float deltaX = ballCenter.x - rectCenter.x; // R�nica X 
        float deltaY = ballCenter.y - rectCenter.y; // R�nica Y

        //obliczenie glebokosci przeciecia
        //abs- wartosc bezwzgkedna
        //jest r�nic� mi�dzy sum� po��wek szeroko�ci / wysoko�ci obu obiekt�w a odleg�o�ci� ich �rodk�w.U�ywa si� tutaj warto�ci bezwzgl�dnych(std::abs), poniewa� r�nica ta musi by� dodatnia.
        float intersectX = (rectBounds.width / 2 + ballBounds.width / 2) - std::abs(deltaX); //szerokosci pilki i bloczka od srodka - wart bezwgl roznicy srodkow mas w x
        float intersectY = (rectBounds.height / 2 + ballBounds.height / 2) - std::abs(deltaY); //wyskosci pilki i bloczka od srodka - wart bezwgl roznicy srodkow mas w y

        // Odbicie pi�ki w zale�no�ci od kierunku kolizji
        //Okre�lenie kierunku kolizji i odbicie pi�ki
        //porownanie intersects X i Y okre�la, czy kolizja jest bardziej "pozioma" (na bokach prostok�ta) czy "pionowa" (na g�rze/dole prostok�ta)
        if (intersectX < intersectY) { //na boki prostokata -> zmieniamy kierunek predk x
            ballVelocity.x = -ballVelocity.x;
            float newX = ball.getPosition().x;//zmiana ball.setPosition(ball.getPosition().x + (deltaX > 0 ? intersectX : -intersectX), ball.getPosition().y);
            if (deltaX > 0) {
                newX += intersectX;
            }
            else {
                newX -= intersectX;
            }
            ball.setPosition(newX, ball.getPosition().y);
            //(deltaX > 0 ? intersectX : -intersectX) -> sprawdza czy deltaX>0 (Oznacza to, �e �rodek pi�ki znajduje si� po prawej stronie �rodka prostok�ta.)
            //jesli war prawdziwy to wynik to intersectX, jesli falszywy -intersectX
        }
        else { //inaczej zmieniamy kier predk y
            ballVelocity.y = -ballVelocity.y;
            float newY = ball.getPosition().y; //zmiana ball.setPosition(ball.getPosition().x, ball.getPosition().y + (deltaY > 0 ? intersectY : -intersectY));

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

// Funkcja obs�uguj�ca kolizje pi�ki z paletk� i bloczkami //Przekazywanie danych przez referencj� w funkcji handleCollisions
void handleCollisions(Ball& ball, Paddle& paddle, std::vector<Block>& blocks, int& score) {
    checkCollision(paddle.shape, ball.shape, ball.velocity); // Kolizja z paletk�

    // Kolizja z blokami
    for (auto& block : blocks) {
        if (!block.destroyed && checkCollision(block.shape, ball.shape, ball.velocity)) {
            block.destroyed = true; // Oznacz bloczek jako zniszczony
            score++; // Dodaj punkt
        }
    }
}


// ----------------------------------
// Funkcja renderEndScreen - Ekran ko�ca gry
// ----------------------------------
// Wy�wietla ekran z komunikatem ko�cowym ("GAME OVER" lub "WIN")
// oraz mechanizm wyboru TAK/NIE za pomoc� klasy GameDialog.
void renderEndScreen(sf::RenderWindow& window, const std::string& message, const std::string& timeMessage) {
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    sf::Text mainMessage(message, font, 50);
    mainMessage.setFillColor(sf::Color::White);
    mainMessage.setPosition(200, 150);

    sf::Text timeText(timeMessage, font, 30);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(200, 250);

    PlayAgain again; // Mechanizm wyboru TAK/NIE

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Left:
                case sf::Keyboard::Right:
                    again.selection(); // Prze��czanie mi�dzy TAK/NIE
                    break;
                case sf::Keyboard::Enter:
                    if (again.getIsYesSelected()) return; // Powr�t do menu g��wnego
                    else window.close(); // Zamykanie gry
                    break;
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(mainMessage);
        window.draw(timeText);
        again.draw(window);
        window.display();
    }
}

// Funkcja wy�wietlaj�ca ekran pomocy
void showHelpScreen(sf::RenderWindow& window) {
    sf::Font font;
    font.loadFromFile(FONT_PATH);

    sf::Text helpText("Instrukcja:\n-Uzyj strzalek, aby poruszac paletka.\n-Zbij wszystkie bloczki!\n-Naciskajac 'S' mozna zapisac gre\n-Naciskajac 'L' mozna zaladowac zapisana gre\n-W opcji 'Options' mozna wybrac poziom trudnosci\n-W opcji 'Stats' znajduja sie ostatnie wyniki graczy\n\nNacisnij ENTER, aby powrocic.", font, 20);
    helpText.setFillColor(sf::Color::White);
    helpText.setPosition(50, 50);

    window.clear(sf::Color::Black);
    window.draw(helpText);
    window.display();

    // Czekaj na ENTER
    while (true) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                return;
        }
    }
}


//---------------------------------------------------------------------------
//-------------------Zapis i odczyt gry--------------------------------------

// Save game state to file
void saveGameState(const Paddle& paddle, const Ball& ball, const std::vector<Block>& blocks) {
    ofstream file("gamestate.txt", ios::out); //Tworzy obiekt pliku do zapisu. Otwiera (lub tworzy) plik gamestate.txt w trybie zapisu. Je�li plik istnieje, zostanie nadpisany.
    if (!file) {
        cerr << "Error: Could not open file for saving." << endl;
        return;
    }
    if (file.is_open()) {
        file << paddle.shape.getPosition().x << " " << paddle.shape.getPosition().y << "\n"; //zapisuje pozycje paletki (x,y)
        file << ball.shape.getPosition().x << " " << ball.shape.getPosition().y << "\n"; //zapisuje pozycje pilki (x,y)
        file << ball.velocity.x << " " << ball.velocity.y << "\n"; //zapisuje predkosc pilki (x,y)
        for (const auto& block : blocks) { //Iteruje po wektorze blocks i zapisuje do pliku warto�� zmiennej destroyed ka�dego bloczka.
            file << block.destroyed << " "; //zapisuje zniszczone bloczki (true 1-zniszczony, false 0-niezniszczony)
        }
        file << "\n";
        file.close();
        cout << "Game state saved." << endl;
    }
}

void loadGameState(Paddle& paddle, Ball& ball, std::vector<Block>& blocks) {
    ifstream file("gamestate.txt", ios::in); //Tworzy obiekt pliku do odczytu. Otwiera plik gamestate.txt w trybie odczytu.
    if (!file) {
        cerr << "Error: Could not open file for loading." << endl;
        return;
    }
    if (file.is_open()) {
        sf::Vector2f paddlePosition;
        sf::Vector2f ballPosition;
        sf::Vector2f ballVelocity;

        //pobranie pozycji z pliku
        file >> paddlePosition.x >> paddlePosition.y; //Odczytuje dwie liczby (pozycj� x i y) z pliku i przypisuje je do zmiennych paddlePosition.x i paddlePosition.y
        file >> ballPosition.x >> ballPosition.y;
        file >> ballVelocity.x >> ballVelocity.y;

        paddle.shape.setPosition(paddlePosition);
        ball.shape.setPosition(ballPosition);
        ball.velocity = ballVelocity;

        for (auto& block : blocks) {
            bool destroyed;
            file >> destroyed;
            block.destroyed = destroyed;
        }

        file.close();
        cout << "Game state loaded." << endl;
    }
}


// Tablica struktur do przechowywania wynik�w graczy
vector<Player> players;
// Funkcja do wczytywania wynik�w graczy z pliku
void loadHighScores() {
    players.clear(); // Wyczy�� istniej�c� list�
    ifstream file("highscores.txt");
    if (file.is_open()) {
        Player p;
        while (file >> p.name >> p.score) {
            players.push_back(p);
        }
        file.close();
    }
}

// Funkcja do zapisywania wynik�w graczy do pliku
void saveHighScores() {
    ofstream file("highscores.txt", ios::trunc); // Nadpisz plik //Tworzy plik highscores.txt w trybie zapisu, usuwaj�c jego poprzedni� zawarto��.
    if (file.is_open()) {
        for (const auto& p : players) {
            file << p.name << " " << p.score << std::endl;
        }
        file.close();
    }
}

// Funkcja dodaj�ca nowy wynik do rankingu
void addHighScore(const std::string& playerName, int score, double elapsedTime) {
    players.push_back({ playerName, score });
    sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
        });
    if (players.size() > 10) { // Zachowaj tylko top 10 wynik�w
        players.pop_back();
    }

    // Zapisz do pliku z czasem
    ofstream file("highscores.txt", ios::app); // Dopisuj do pliku //Tworzy plik highscores.txt w trybie dopisywania. Nowe dane b�d� dodawane na ko�cu pliku, bez usuwania istniej�cej zawarto�ci.
    if (file.is_open()) {
        file << playerName << ", score: " << score << ", time " << elapsedTime << "s" << std::endl;
        file.close();
    }
}

// Funkcja wy�wietlaj�ca statystyki graczy
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

    vector<std::string> statsLines;
    ifstream file("highscores.txt");
    string line;
    while (getline(file, line)) {
        statsLines.push_back(line);
    } //Iteruje po liniach pliku i dodaje ka�d� lini� do wektora statsLines
    file.close();

    std::vector<sf::Text> statsText;
    for (size_t i = 0; i < statsLines.size(); ++i) {
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
                return; // Powr�t do menu g��wnego
            }
        }

        window.clear(sf::Color::Black);
        window.draw(statsTitle);
        window.draw(backTitle);
        for (const auto& text : statsText) {
            window.draw(text);
        }
        window.display();
    }
}

//---------------------------------------------------------------------------


// ----------------------------------
// Funkcja runGame - G��wna p�tla gry
// ----------------------------------
// Zarz�dza ca�� logik� gry: sterowaniem, aktualizacj� stanu gry, renderowaniem obiekt�w
void runGame(sf::RenderWindow& window) {
    Paddle paddle(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 50);
    Ball ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 100);

    time_t pauseStartTime = 0; // Czas rozpocz�cia pauzy (do oblicze� ca�kowitego czasu gry)
    double totalPauseTime = 0; // ��czny czas sp�dzony na pauzach

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    int score = 0; // Licznik punkt�w

    // Inicjalizacja tekst�w dla wyniku i czasu
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

    time_t startTime = time(nullptr); // Czas rozpocz�cia gry
    bool gameRunning = true; // Flaga kontroluj�ca stan gry
    bool isPaused = false;

    while (window.isOpen() && gameRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1) {
                pauseStartTime = time(nullptr); // Rozpocz�cie pauzy przy ekranie pomocy //Pobiera aktualny czas (w sekundach od epoki UNIX-a) i przypisuje go do pauseStartTime
                showHelpScreen(window);
                totalPauseTime += difftime(time(nullptr), pauseStartTime); // Aktualizacja ��cznego czasu pauzy
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                pauseStartTime = time(nullptr); // Rozpocz�cie pauzy przy ekranie ko�ca gry
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
                                    totalPauseTime += difftime(time(nullptr), pauseStartTime); // Dodanie czasu pauzy
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
            // Sterowanie paletk�
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                paddle.moveLeft();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                paddle.moveRight();

            // Aktualizacja pozycji pi�ki
            ball.update();
        }

        // Obs�uga kolizji i aktualizacja wyniku
        handleCollisions(ball, paddle, blocks, score);

        // Sprawdzenie, czy pi�ka dotkn�a dolnej granicy (koniec gry)
        if (ball.shape.getPosition().y > WINDOW_HEIGHT) {
            time_t endTime = time(nullptr);
            int elapsedSeconds = static_cast<int>(difftime(endTime, startTime) - totalPauseTime);
            std::string timeMessage = "Czas gry: " + to_string(elapsedSeconds) + " sekund";

            // Dodanie wyniku do rankingu
            addHighScore("Player", score, elapsedSeconds);
            renderEndScreen(window, "GAME OVER", timeMessage);
            gameRunning = false;
            break;
        }

        // Sprawdzenie, czy wszystkie bloki zosta�y zniszczone
        bool allDestroyed = true;
        for (const auto& block : blocks) {
            if (!block.destroyed) {
                allDestroyed = false;
                break;
            }
        }
        if (allDestroyed) {
            time_t endTime = time(nullptr);
            int elapsedSeconds = static_cast<int>(difftime(endTime, startTime) - totalPauseTime);
            std::string timeMessage = "Czas gry: " + to_string(elapsedSeconds) + " sekund";

            addHighScore("Player", score, elapsedSeconds);
            renderEndScreen(window, "WIN", timeMessage);
            gameRunning = false;
            break;
        }

        // Renderowanie obiekt�w gry
        window.clear();
        window.draw(paddle.shape);
        window.draw(ball.shape);
        for (const auto& block : blocks) {
            if (!block.destroyed)
                window.draw(block.shape);
        }

        // Wy�wietlanie wyniku i czasu na ekranie
        scoreText.setString("Score: " + to_string(score));
        int elapsedTime = static_cast<int>(difftime(time(nullptr), startTime) - totalPauseTime);
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

//---------------------------------------------------------------------------

// ----------------------------------
// Funkcja chooseLevel - Wyb�r poziomu trudno�ci
// ----------------------------------
// Pozwala graczowi wybra� poziom trudno�ci (EASY, MEDIUM, HARD)
// i ustawia odpowiedni� pr�dko�� pi�ki.
void chooseLevel(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        cerr << "Error loading font" << endl;
        return;
    }

    sf::Text levelTitle("Choose Level", font, 50);
    levelTitle.setFillColor(sf::Color::White);
    levelTitle.setPosition(WINDOW_WIDTH / 2 - 150, 50);

    const std::vector<std::string> items = { "EASY", "MEDIUM", "HARD" };
    const std::vector<float> speeds = { -2.0f, -3.0f, -4.0f };
    const std::vector<float> paddlewidth = { 100.0f, 80.0f, 60.0f };
    std::vector<sf::Text> levels(items.size());
    int selectedLevel = 0;

    for (size_t i = 0; i < items.size(); i++) {
        if (i == selectedLevel) {
            levels[i].setFillColor(sf::Color::Red);
        }
        else {
            levels[i].setFillColor(sf::Color::White);
        }
        levels[i].setFont(font);
        levels[i].setString(items[i]);
        //levels[i].setFillColor(i == selectedLevel ? sf::Color::Red : sf::Color::White);
        levels[i].setPosition(WINDOW_WIDTH / 2 - 100, 150 + i * 50);
    }


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up)
                    selectedLevel = (selectedLevel - 1 + items.size()) % items.size(); //przesuwa wyb�r poziomu w menu w g�r�, z uwzgl�dnieniem cykliczno�ci.
                    //Wyja�nienie-> (selectedLevel - 1) przesuwa wyb�r o jeden w g�r�. Dodanie items.size() i u�ycie % zapobiega przekroczeniu indeks�w (cykliczno��).
                else if (event.key.code == sf::Keyboard::Down)
                    selectedLevel = (selectedLevel + 1) % items.size();
                else if (event.key.code == sf::Keyboard::Enter) {
                    ball_speed = speeds[selectedLevel];
                    PADDLE_WIDTH = paddlewidth[selectedLevel];
                    return;
                }
            }
        }

        for (size_t i = 0; i < items.size(); i++) {
            if (i == selectedLevel) {
                levels[i].setFillColor(sf::Color::Red);
            }
            else {
                levels[i].setFillColor(sf::Color::White);
            }
            //levels[i].setFillColor(i == selectedLevel ? sf::Color::Red : sf::Color::White);
        }

        window.clear(sf::Color::Black);
        window.draw(levelTitle);
        for (const auto& level : levels) {
            window.draw(level);
        }
        window.display();
    }
}
//---------------------------------------------------------------------------


//---------------------------------G��WNA P�TLA-----------------------------
//->->->->->->->->->
int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arkanoid");

    window.setFramerateLimit(60); // Ograniczenie liczby klatek na sekund�

    loadHighScores();

    Menu menu(WINDOW_WIDTH, WINDOW_HEIGHT);

    bool isPaused = false;
    EndGame endgame;

    Paddle paddle(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 50);
    Ball ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    std::vector<Block> blocks;
    int score = 0;

    // Wype�nij wektor blok�w
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