#include <iostream>
#include <conio.h>
#include <ctime>
#include <vector>
#include <fstream>

using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;

struct Fort {
    int x;
    int health; // 6 full health max
};

struct Enemy {
    int x, y;
};

void clearScreen() {
    system("cls");
}

void saveHighScore(int score) {
    ofstream file("highscore.dat");
    if (file.is_open()) {
        file << score;
        file.close();
    }
}

int loadHighScore() {
    ifstream file("highscore.dat");
    int hs = 0;
    if (file.is_open()) {
        file >> hs;
        file.close();
    }
    return hs;
}

void draw(const vector<Enemy>& enemies, const vector<Fort>& forts, int playerX, int bulletX, int bulletY, bool bulletActive, int score, int highScore) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            bool drawn = false;

            if (y == HEIGHT - 1 && x == playerX) {
                cout << "A";
                drawn = true;
            }

            if (!drawn && bulletActive && x == bulletX && y == bulletY) {
                cout << "|";
                drawn = true;
            }

            if (!drawn) {
                for (const auto& e : enemies) {
                    if (e.x == x && e.y == y) {
                        cout << "W";
                        drawn = true;
                        break;
                    }
                }
            }

            if (!drawn) {
                for (const auto& f : forts) {
                    if (f.x == x && y == HEIGHT - 3) {
                        if (f.health == 6) cout << "F";
                        else if (f.health >= 2 && f.health <= 5) cout << "f";
                        else if (f.health == 1) cout << ".";
                        else cout << " ";
                        drawn = true;
                        break;
                    }
                }
            }

            if (!drawn) cout << " ";
        }
        cout << "\n";
    }
    cout << "Score: " << score << "    High Score: " << highScore << "\n";
}

int main() {
    int playerX = WIDTH / 2;
    int bulletX = 0, bulletY = 0;
    bool bulletActive = false;

    int score = 0;
    int highScore = loadHighScore();

    double enemySpeed = 0.5;
    int round = 1;

    vector<Enemy> enemies;
    vector<Fort> forts;

    for (int i = 5; i < WIDTH; i += 8) {
        forts.push_back({ i, 6 });
    }

    auto initEnemies = [&]() {
        enemies.clear();
        for (int y = 1; y <= 2; ++y) {
            for (int x = 5; x < WIDTH - 5; x += 4) {
                enemies.push_back({ x, y });
            }
        }
    };

    initEnemies();

    clock_t lastEnemyMove = clock();
    clock_t lastShotTime = 0;
    const double shootCooldown = 0.2; // 0.2 seconds now

    bool enemiesMovingRight = true;

    while (true) {
        clearScreen();

        draw(enemies, forts, playerX, bulletX, bulletY, bulletActive, score, highScore);

        if (kbhit()) {
            char ch = getch();
            if (ch == 'a' || ch == 'A') {
                if (playerX > 0) playerX--;
            } else if (ch == 'd' || ch == 'D') {
                if (playerX < WIDTH - 1) playerX++;
            } else if (ch == 'k' || ch == 'K') {
                clock_t currentTime = clock();
                double elapsedSeconds = double(currentTime - lastShotTime) / CLOCKS_PER_SEC;
                if (elapsedSeconds >= shootCooldown && !bulletActive) {
                    bulletX = playerX;
                    bulletY = HEIGHT - 2;
                    bulletActive = true;
                    lastShotTime = currentTime;
                }
            }
        }

        if (bulletActive) {
            bulletY--;
            if (bulletY < 0) bulletActive = false;
        }

        if (bulletActive) {
            for (size_t i = 0; i < enemies.size(); ++i) {
                if (enemies[i].x == bulletX && enemies[i].y == bulletY) {
                    enemies.erase(enemies.begin() + i);
                    bulletActive = false;
                    score += 100;
                    if (score > highScore) {
                        highScore = score;
                        saveHighScore(highScore);
                    }
                    break;
                }
            }
        }

        if (bulletActive) {
            for (auto& f : forts) {
                if (f.x == bulletX && bulletY == HEIGHT - 3 && f.health > 0) {
                    f.health--;
                    bulletActive = false;
                    break;
                }
            }
        }

        clock_t now = clock();
        double enemyElapsed = double(now - lastEnemyMove) / CLOCKS_PER_SEC;
        if (enemyElapsed >= enemySpeed) {
            bool edgeHit = false;
            if (enemiesMovingRight) {
                for (auto& e : enemies) {
                    if (e.x >= WIDTH - 1) {
                        edgeHit = true;
                        break;
                    }
                }
                if (edgeHit) {
                    enemiesMovingRight = false;
                    for (auto& e : enemies) e.y++;
                } else {
                    for (auto& e : enemies) e.x++;
                }
            } else {
                for (auto& e : enemies) {
                    if (e.x <= 0) {
                        edgeHit = true;
                        break;
                    }
                }
                if (edgeHit) {
                    enemiesMovingRight = true;
                    for (auto& e : enemies) e.y++;
                } else {
                    for (auto& e : enemies) e.x--;
                }
            }
            lastEnemyMove = now;
        }

        for (auto& e : enemies) {
            if (e.y >= HEIGHT - 1) {
                cout << "\nGAME OVER! Final Score: " << score << "\n";
                return 0;
            }
        }

        if (enemies.empty()) {
            round++;
            enemySpeed *= 0.8;
            initEnemies();

            for (auto& f : forts) {
                f.health = 6;
            }
        }

        for (int i = 0; i < 1000000; ++i) {}
    }

    return 0;
}