#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <vector>
#include "list.h"

using namespace std;

static bool allowMove = false;
Color green = {91, 135, 49, 255};
Color darkGreen = {43, 51, 24, 255};
Color start = {112, 179, 81, 255};

int cellSize = 64;
int cellCount = 13;
int offset = 64;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    std::vector<Texture2D> headTextures;
    std::vector<Texture2D> bodyTextures;
    std::vector<Texture2D> tailTextures;
    Rectangle headSourceRect = {0, 0, 64, 64};

    Snake()
    {
        headTextures.push_back(LoadTexture("images\\cap_dreapta.png"));
        headTextures.push_back(LoadTexture("images\\cap_jos.png"));
        headTextures.push_back(LoadTexture("images\\cap_stanga.png"));
        headTextures.push_back(LoadTexture("images\\cap_sus.png"));

        bodyTextures.push_back(LoadTexture("images\\corp_dreapta.png"));
        bodyTextures.push_back(LoadTexture("images\\corp_jos.png"));
        bodyTextures.push_back(LoadTexture("images\\corp_stanga.png"));
        bodyTextures.push_back(LoadTexture("images\\corp_sus.png"));

        tailTextures.push_back(LoadTexture("images\\coada_dreapta.png"));
        tailTextures.push_back(LoadTexture("images\\coada_sus.png"));
        tailTextures.push_back(LoadTexture("images\\coada_stanga.png"));
        tailTextures.push_back(LoadTexture("images\\coada_jos.png"));
    }

    ~Snake()
    {
        for (auto &texture : headTextures) {
            UnloadTexture(texture);
        }

        for (auto &texture : bodyTextures) {
            UnloadTexture(texture);
        }

        for (auto &texture : tailTextures) {
            UnloadTexture(texture);
        }
    }

    void Draw()
    {
        int directionIndex = 0; // Default to right-facing texture
        if (direction.x == -1) directionIndex = 2; // Left
        else if (direction.y == 1) directionIndex = 1; // Down
        else if (direction.y == -1) directionIndex = 3; // Up

        // Draw head
        DrawTexturePro(headTextures[directionIndex], headSourceRect, Rectangle{offset + body[0].x * cellSize, offset + body[0].y * cellSize, 64, 64}, Vector2{0}, 0.0f, WHITE);

        // Draw body + tail
        int bodyDirectionIndex = 0;  // Initialize to default value
        for (size_t i = 1; i < body.size(); i++)
        {
            Vector2 currentSegment = body[i];
            Vector2 previousSegment = (i == 1) ? body[0] : body[i - 1];

            // Determine the direction of the current segment (based on the previous segment)
            if (currentSegment.x > previousSegment.x) bodyDirectionIndex = 2; // Left
            else if (currentSegment.x < previousSegment.x) bodyDirectionIndex = 0; // Right
            else if (currentSegment.y > previousSegment.y) bodyDirectionIndex = 1; // Down
            else if (currentSegment.y < previousSegment.y) bodyDirectionIndex = 3; // Up

            Texture2D textureToUse = (i < body.size() - 1) ? bodyTextures[bodyDirectionIndex] : tailTextures[bodyDirectionIndex];

            DrawTextureEx(textureToUse, Vector2{offset + currentSegment.x * cellSize, offset + currentSegment.y * cellSize}, 0, 1.0, WHITE);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture, texture2;
    bool isGolden = false;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("images\\apple.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);

        Image image2 = LoadImage("images\\bolat_cola.png");
        texture2 = LoadTextureFromImage(image2);
        UnloadImage(image2);

        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
        UnloadTexture(texture2);
    }

    void Draw()
    {
        if (isGolden == true)
        {
            DrawTexture(texture2, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }
        else
        {
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    enum GameState { MAIN_MENU, RUNNING, GAME_OVER, PAUSE, QUESTION };
    GameState state = MAIN_MENU;

    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    int goldenapple = 0;
    Texture2D background;
    Sound eatSound;
    Sound wallSound;
    std::string question;
    std::vector<std::string> answers;
    int correctAnswerIndex;
    struct node* currentQuestion;

    Game()
    {
        InitAudioDevice();
        Image bgImage = LoadImage("images\\bg.png");
        background = LoadTextureFromImage(bgImage);
        UnloadImage(bgImage);

        eatSound = LoadSound("Sounds\\eat.mp3");
        wallSound = LoadSound("Sounds\\bonk.mp3");

        // Load initial question
        LoadRandomQuestion();
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void LoadRandomQuestion()
{
    if (head == NULL) {
        // Handle the case where the linked list is empty
        // For example, print an error message or return
        return;
    }

    int totalQuestions = 0;
    struct node* temp = head;
    while (temp != NULL) {
        totalQuestions++;
        temp = temp->next;
    }

    if (totalQuestions == 0) {
        // Handle the case where the linked list is empty
        // For example, print an error message or return
        return;
    }

    int randomIndex = GetRandomValue(0, totalQuestions - 1);
    temp = head;
    for (int i = 0; i < randomIndex; i++) {
        if (temp == NULL) {
            // Handle the case where the index is out of range
            // For example, print an error message or return
            return;
        }
        temp = temp->next;
    }

    if (temp == NULL) {
        // Handle the case where the index is out of range
        // For example, print an error message or return
        return;
    }

    currentQuestion = temp;
    question = temp->grila[0];
    answers.clear();
    for (int i = 1; i <= 4; i++) {
        answers.push_back(temp->grila[i]);
    }
    correctAnswerIndex = -1;
    for (int i = 0; i < 4; i++) {
        if (temp->rasp[i] == 1) {
            correctAnswerIndex = i;
            break;
        }
    }
}


    void Draw()
    {
        switch (state)
        {
            case MAIN_MENU:
                DrawMainMenu();
                break;
            case RUNNING:
                food.Draw();
                snake.Draw();
                break;
            case GAME_OVER:
                ClearBackground(RED);
                DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 60) / 2, GetScreenHeight() / 2 - 40, 60, BLACK);
                DrawText("Press SPACE to restart", GetScreenWidth() / 2 - MeasureText("Press SPACE to restart", 20) / 2, GetScreenHeight() / 2 + 10, 20, BLACK);
                break;
            case PAUSE:
                DrawPauseScreen();
                break;
            case QUESTION:
                DrawQuestionScreen();
                break;
        }
    }

    void DrawMainMenu()
    {
        ClearBackground(start);

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        int buttonWidth = 200;
        int buttonHeight = 50;
        int buttonX = screenWidth / 2 - buttonWidth / 2;

        // Start Button
        int startButtonY = screenHeight / 2 - buttonHeight / 2 - 50;
        DrawRectangle(buttonX, startButtonY, buttonWidth, buttonHeight, GREEN);
        DrawText("START", buttonX + 60, startButtonY + 15, 25, BLACK);

        // Quit Button
        int quitButtonY = screenHeight / 2 + buttonHeight - 50;
        DrawRectangle(buttonX, quitButtonY, buttonWidth, buttonHeight, RED);
        DrawText("QUIT", buttonX + 73, quitButtonY + 15, 25, BLACK);

        // Check if buttons are pressed
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mousePoint = GetMousePosition();

            if (CheckCollisionPointRec(mousePoint, Rectangle{ (float)buttonX, (float)startButtonY, (float)buttonWidth, (float)buttonHeight }))
            {
                state = RUNNING;
            }

            if (CheckCollisionPointRec(mousePoint, Rectangle{ (float)buttonX, (float)quitButtonY, (float)buttonWidth, (float)buttonHeight }))
            {
                CloseWindow(); // Closes the window
            }
        }
    }

    void DrawPauseScreen()
    {
        ClearBackground(DARKGRAY);
        if (IsKeyPressed(KEY_ENTER))
        {
            state = RUNNING;
        }
    }

    void DrawQuestionScreen()
    {
        ClearBackground(DARKGRAY);
        DrawText("BOLAT COLA appeared!", GetScreenWidth() / 2 - MeasureText("BOLAT COLA appeared!", 20) / 2, GetScreenHeight() / 2 - 120, 20, BLACK);
        DrawText("Answer this question for a power-up, but if you answer wrong...", GetScreenWidth() / 2 - MeasureText("Answer this question for a power-up, but if you answer wrong...", 20) / 2, GetScreenHeight() / 2 - 80, 20, BLACK);
        DrawText(question.c_str(), GetScreenWidth() / 2 - MeasureText(question.c_str(), 20) / 2, GetScreenHeight() / 2 - 40, 20, BLACK);

        for (int i = 0; i < answers.size(); i++)
        {
            DrawText(answers[i].c_str(), GetScreenWidth() / 2 - MeasureText(answers[i].c_str(), 19) / 2, GetScreenHeight() / 2 + i * 30, 19, BLACK);
        }
    }

    void Update()
    {
        if (state == RUNNING)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
        else if (state == GAME_OVER && IsKeyPressed(KEY_SPACE))
        {
            RestartGame();
        }
        else if (state == QUESTION)
        {
            UpdateQuestionScreen();
        }
    }

    void UpdateQuestionScreen()
    {
        if (IsKeyPressed(KEY_ONE))
        {
            CheckAnswer(0);
        }
        else if (IsKeyPressed(KEY_TWO))
        {
            CheckAnswer(1);
        }
        else if (IsKeyPressed(KEY_THREE))
        {
            CheckAnswer(2);
        }
        else if (IsKeyPressed(KEY_FOUR))
        {
            CheckAnswer(3);
        }
    }

    void CheckAnswer(int answerIndex)
    {
        if (answerIndex == correctAnswerIndex)
        {
            state = RUNNING; // Correct answer
            score++;
        }
        else
        {
            state = GAME_OVER; // Incorrect answer
        }
    }

    void RestartGame()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        score = 0;
        goldenapple = 0;
        state = RUNNING;
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            snake.addSegment = true;
            score++;
            goldenapple++;

            if (food.isGolden)
            {
                LoadRandomQuestion();
                state = QUESTION; // Pause the game if it's a golden apple
            }

            if (goldenapple % 3 == 0 && goldenapple != 0)
            {
                food.isGolden = true; // Every third apple is golden
            }
            else
            {
                food.isGolden = false;
            }

            food.position = food.GenerateRandomPos(snake.body);
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
        state = GAME_OVER;
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }
};

int main()
{
    readFile();
    printList();
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake ECO");
    SetTargetFPS(144);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (EventTriggered(0.2) && game.state != Game::GameState::QUESTION)
        {
            allowMove = true;
            game.Update();
        }
        else if (game.state == Game::GameState::QUESTION)
        {
            game.UpdateQuestionScreen();
        }

        if (game.state == Game::GameState::RUNNING && (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && game.snake.direction.y != 1 && allowMove)
        {
            game.snake.direction = {0, -1};
            game.running = true;
            allowMove = false;
        }
        if (game.state == Game::GameState::RUNNING && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && game.snake.direction.y != -1 && allowMove)
        {
            game.snake.direction = {0, 1};
            game.running = true;
            allowMove = false;
        }
        if (game.state == Game::GameState::RUNNING && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && game.snake.direction.x != 1 && allowMove)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
            allowMove = false;
        }
        if (game.state == Game::GameState::RUNNING && (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && game.snake.direction.x != -1 && allowMove)
        {
            game.snake.direction = {1, 0};
            game.running = true;
            allowMove = false;
        }

        if (IsKeyPressed(KEY_SPACE) && game.state == Game::GameState::GAME_OVER)
        {
            game.RestartGame();
        }

        int textWidth = MeasureText("Snake ECO", 40);
        int screenWidth = GetScreenWidth();
        int textX = (screenWidth - textWidth) / 2;

        // Drawing
        DrawTexture(game.background, offset, offset, WHITE);
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Snake ECO", textX, 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
