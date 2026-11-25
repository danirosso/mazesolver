#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800
#define MENU_WIDTH 200
#define BORDER_HEIGHT 20
#define BORDER_WIDTH 15

#define WALL -1
#define DEFAULT_FLOOR 0
#define WALKED_FLOOR 1
#define SOLVED_FLOOR -2

typedef struct{
    int x;
    int y;
} Position;

typedef struct{
    int columns;
    int lines;
    int tiles;
    int **maze;
} Maze;

typedef struct{
    Position *walked;
    int index;
    int capacity;
} WalkedPath;
void pushPosition(Position start, WalkedPath *path);
//pushes positions dinamically into the path
void popPosition(WalkedPath *path);
//not an actual pop, just sets the index back
//https://youtu.be/95M6V3mZgrI

Maze convertFileToMaze(FILE *f);
//returns a Maze struct from the given file
void freeMaze(Maze *m);
//frees the allocated maze
void calcSizes(Maze *m, Position *startPos, Position *sqSize);
//calculates the proportional sizes to draw the maze
void drawMaze(Maze *m, Position *startPos, Position *sqSize);
//draws a maze in the window
bool insideMaze(Vector2 *mousePos, Position *startPos, Position *sqSize, Maze *maze);
//returns true if the cursor is inside the maze area
bool convertFromCursor(Position *pos, Vector2 *mousePos, Position *startPos, Position *sqSize, Maze *maze);
//returns a X/Y position relative to the maze
void printPosCursor(Vector2 *mousePos, Position *pos);
//prints a position around the mouse
bool solveMaze(Maze *m, WalkedPath *path, Position start, Position end, int floorPass);
//it solves the maze <shrug>

int main(int argc, char **argv)
{
    if (argc != 2){
        fprintf(stderr, "To run this program you need to pass the"
                " location of a maze file in the first argument\n");
        return 1;
    }

    FILE *mazeIn = fopen(*(argv + 1), "r");
    if (mazeIn == NULL){
        fprintf(stderr,"Error opening the file\n");
        return 1;
    }

    Maze maze = convertFileToMaze(mazeIn);
    Maze solvedMaze = maze;
    Position sqSize, startPos;
    calcSizes(&maze, &startPos, &sqSize);

    WalkedPath solvedPath = {0};
    bool solved = false;

    Position start;
    bool startButtonSelected = false;
    bool startSetd = false;

    Position end;
    bool endButtonSelected = false;
    bool endSetd = false;

    bool goSetd = false;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "maze");
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    while (!WindowShouldClose()){

        if (!solved && startSetd && endSetd && goSetd){
            solved = solveMaze(&maze, &solvedPath, start, end, WALKED_FLOOR);
            for(int i = 0; i < solvedPath.index; i++){
                maze.maze[solvedPath.walked[i].y][solvedPath.walked[i].x]= SOLVED_FLOOR;
            }
        }

        BeginDrawing();
        Vector2 mousePos = GetMousePosition();
        ClearBackground(BLACK);

        drawMaze(&maze, &startPos, &sqSize);

        if(GuiButton((Rectangle){BORDER_WIDTH, BORDER_HEIGHT*2, MENU_WIDTH - BORDER_WIDTH, BORDER_WIDTH*2}, "Start position") && !startButtonSelected){
            endButtonSelected = false;
            startButtonSelected = !startButtonSelected;
        }

        if(GuiButton((Rectangle){BORDER_WIDTH, BORDER_HEIGHT*6, MENU_WIDTH - BORDER_WIDTH, BORDER_WIDTH*2}, "End Position") && !endButtonSelected){
            startButtonSelected = false;
            endButtonSelected = !endButtonSelected;
        }


        if (insideMaze(&mousePos, &startPos, &sqSize, &maze) && (startButtonSelected || endButtonSelected)){
            SetMouseCursor(3);
            if (startButtonSelected){
                if (convertFromCursor(&start, &mousePos, &startPos, &sqSize, &maze)){;
                    printPosCursor(&mousePos, &start);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                        startButtonSelected = false;
                        startSetd = true;
                    }
                }
            }

            if (endButtonSelected){
                if (convertFromCursor(&end, &mousePos, &startPos, &sqSize, &maze)){;
                    printPosCursor(&mousePos, &end);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                        endButtonSelected = false;
                        endSetd = true;
                    }
                }
            }
        } else SetMouseCursor(1);

        if(startSetd){
            char startext[10];
            snprintf(startext, 10, "X:%2d Y:%2d", start.x, start.y);
            DrawText(startext, BORDER_WIDTH, BORDER_HEIGHT*4, BORDER_HEIGHT, RED);
        }

        if(endSetd){
            char entxt[10];
            snprintf(entxt, 10, "X:%2d Y:%2d", end.x, end.y);
            DrawText(entxt, BORDER_WIDTH, BORDER_HEIGHT*8, BORDER_HEIGHT, RED);
        }

        if(startSetd && endSetd && !solved){
            if (GuiButton((Rectangle){BORDER_WIDTH, BORDER_HEIGHT*10, MENU_WIDTH - BORDER_WIDTH, BORDER_WIDTH*2}, "Solve Maze")){
                goSetd = true;
            }
        }

            for(int i = 0; i < solvedPath.index; i++){
                char txt[10];
                snprintf(txt, 10, "X:%2d Y:%2d", solvedPath.walked[i].x, solvedPath.walked[i].y);
                DrawText(txt, BORDER_WIDTH  + 100 ,BORDER_HEIGHT +  10*i, 12, RED);
            }





        EndDrawing();
    }

    freeMaze(&maze);
    CloseWindow();
    return 0;
}


void printPosCursor (Vector2 *mousePos, Position *pos)
{
    char position[12];
    snprintf(position, 12, "X: %d\n"
            "Y: %d\n", pos->x, pos->y);
    DrawText(position, mousePos->x + 5, mousePos->y + 5, BORDER_HEIGHT, RED);
}

bool insideMaze (Vector2 *mousePos, Position *startPos, Position *sqSize, Maze *maze)
{
    if(mousePos->x >= startPos->x && mousePos->y >= startPos->y &&
            mousePos->y <= (sqSize->y * maze->lines) + BORDER_HEIGHT &&
            mousePos->x <= (sqSize->x * maze->columns) + MENU_WIDTH + BORDER_WIDTH){
        return true;
    }

    return false;
}

bool convertFromCursor(Position *pos, Vector2 *mousePos, Position *startPos, Position *sqSize, Maze *m)
{

    pos->x = (mousePos->x - MENU_WIDTH - BORDER_WIDTH) / sqSize->x;
    pos->y = (mousePos->y - BORDER_HEIGHT) / sqSize->y;

    if(m->maze[pos->y][pos->x] != WALL) return true;

    return false;
}

bool solveMaze (Maze *m, WalkedPath *path, Position start, Position end, int floorPass)
{
    if(start.x < 0 || start.x >= m->columns || start.y < 0 || start.y >= m->lines){
        return false;
    }

    if(m->maze[start.y][start.x] != DEFAULT_FLOOR){
        return false;
    }

    m->maze[start.y][start.x] = floorPass;
    pushPosition(start, path);

    if(start.y == end.y && start.x == end.x) return true;

    if (solveMaze(m, path, (Position){start.x + 1, start.y}, end, floorPass + 1)) return true;
    if (solveMaze(m, path, (Position){start.x - 1, start.y}, end, floorPass + 1)) return true;
    if (solveMaze(m, path, (Position){start.x, start.y + 1}, end, floorPass + 1)) return true;
    if (solveMaze(m, path, (Position){start.x, start.y - 1}, end, floorPass + 1)) return true;

    popPosition(path);
    return false;
}

void popPosition(WalkedPath *path)
{
    if(path->index == 0) return;

    path->index--;
}

void pushPosition(Position start, WalkedPath *path)
{
    if(path->capacity == 0){
        path->walked = (Position *)malloc(sizeof(Position *));
        path->capacity = 1;
    }
    if(path->capacity <= path->index){
        path->capacity *= 2;
        path->walked = (Position *)realloc(path->walked, sizeof(Position *)*path->capacity);
    }

    path->walked[path->index++]= start;
}

void calcSizes (Maze *m, Position *startPos, Position *sqSize)
{
    startPos->x = MENU_WIDTH + BORDER_WIDTH;
    startPos->y = BORDER_HEIGHT;

    sqSize->x = (float)(WINDOW_WIDTH - BORDER_WIDTH - startPos->x) / m->columns;
    sqSize->y = (float)(WINDOW_HEIGHT - BORDER_HEIGHT - startPos->y) / m->lines;
}

void drawMaze (Maze *m, Position *startPos, Position *sqSize)
{
    for (int i = 0; i < (m->lines); i++){
        for (int j = 0; j < (m->columns); j++){
            if (m->maze[i][j] == WALL)
                DrawRectangle(startPos->x + sqSize->x*j, startPos->y + sqSize->y*i,
                        sqSize->x - 1, sqSize->y - 1, DARKBROWN);

            else if (m->maze[i][j] == DEFAULT_FLOOR)
                DrawRectangle(startPos->x + sqSize->x*j, startPos->y + sqSize->y*i,
                        sqSize->x - 1, sqSize->y - 1, GRAY);

            else if (m->maze[i][j] == SOLVED_FLOOR)
                DrawRectangle(startPos->x + sqSize->x*j, startPos->y + sqSize->y*i,
                        sqSize->x - 1, sqSize->y - 1, VIOLET);

            else if(m->maze[i][j] > DEFAULT_FLOOR)
                DrawRectangle(startPos->x + sqSize->x*j, startPos->y + sqSize->y*i,
                        sqSize->x - 1, sqSize->y - 1, (Color){0, 255, 0, MIN(10 + m->maze[i][j] * 2, 255)});
        }
    }
}

Maze convertFileToMaze(FILE *f)
{
    Maze out = {0};
    fscanf(f, "%d\n%d\n", &out.lines, &out.columns);

    /*This is wasteful but it's easier to understand*/
    out.maze = (int **)malloc(sizeof(int *)*out.lines);
    for (int i = 0; i < out.lines; i++){
        *(out.maze + i) = (int *)malloc(sizeof(int)*out.columns);
    }

    for (int i = 0; i < out.lines; i++){
        for(int j = 0; j < out.columns; j++){
            char ch = getc(f);
            if (ch == '\n') ch = getc(f);
            if (ch == 'X') out.maze[i][j] = WALL;
            if (ch == ' '){
                out.maze[i][j] = DEFAULT_FLOOR;
                out.tiles++;
            }
        }
    }

    return out;
}

void freeMaze(Maze *m)
{
    for(int i = 0; i < m->lines; i++){
        free(m->maze[i]);
    }

    free(m->maze);
}
