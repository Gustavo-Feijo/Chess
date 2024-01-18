#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define bitmap_size 100
#define BLACK 1
#define WHITE -1

typedef bool (*ThreatFunction)(int, int);

void drawPieces(ALLEGRO_BITMAP *bitmap);
void drawValidMove(int y, int x);

int *pieceBitmapCoordinates(int chessPieces);
int *getClickPosition(int y, int x);

bool isInCheck();
bool isHorseThreat(int y, int x);
bool isDiagonalThreat(int y, int x);
bool isLineThreat(int y, int x);
bool isPawnThreat(int y, int x);
bool isKingThreat(int y, int x);
bool isThreatened(int y, int x);

bool isValidMove(int curY, int curX, int nextY, int nextX);
void horsePattern(int y, int x);
void horseMove(int curY, int curX, int nextY, int nextX);

// ENUM with chess notation.
enum pieces
{
    P = 1,
    K = 2,
    Q = 3,
    B = 4,
    N = 5,
    R = 6
};

// Creates a 2D array for the board, where:
//  0 - Empty || 1 - Pawn ||  2 - King ||  3 - Queen ||  4 - Bishop ||  5 - Knight || 6 - Rook
// Negative values are the White pieces.
int chessBoard[8][8] = {
    {R, N, B, Q, K, B, N, R},
    {P, P, P, P, P, P, P, P},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {-P, -P, -P, -P, -P, -P, -P, -P},
    {-R, -N, -B, -Q, -K, -B, -N, -R}};

// Variables to game state control.
int currentPlayer = WHITE;
int currentPiece[3] = {0, 0, 0};
bool isPieceSelected = false;

int main()
{

    // Allegro 5 initialization functions.
    if (!al_init())
    {
        printf("Failed to initialize Allegro!\n");
        return -1;
    }
    if (!al_install_mouse())
    {
        printf("Failed to initialize mouse!\n");
        return -1;
    }
    if (!al_init_primitives_addon())
    {
        printf("Failed to initialize Allegro primitives addon!\n");
        return -1;
    }
    if (!al_init_image_addon())
    {
        printf("Failed to initialize Allegro image addon!\n");
        return -1;
    }
    fflush(stdout);

    // Create a 900x900 display.
    int windowWidth = 900;
    int windowHeight = 900;
    ALLEGRO_DISPLAY *display = al_create_display(windowWidth, windowHeight);

    // Confirms the display was created and set the title.
    if (!display)
    {
        printf("Failed to create display!\n");
        return -1;
    }
    al_set_window_title(display, "Chess");

    // Set up event queue to handle events.
    ALLEGRO_EVENT_QUEUE *eventQueue = al_create_event_queue();
    al_register_event_source(eventQueue, al_get_display_event_source(display));
    al_register_event_source(eventQueue, al_get_mouse_event_source());

    // Innitializes the bitmap for the chess pieces.
    ALLEGRO_BITMAP *piecesBitmap = al_load_bitmap("chessPieces.png");

    drawPieces(piecesBitmap);

    ALLEGRO_EVENT event;
    ALLEGRO_MOUSE_STATE mouse;

    // White pieces to move first.

    while (1)
    {
        al_wait_for_event(eventQueue, &event);
        if ((event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) && !isPieceSelected)
        {
            al_get_mouse_state(&mouse);

            int *click = getClickPosition(mouse.y, mouse.x);
            if (chessBoard[click[0]][click[1]] * currentPlayer > 0)
            {
                currentPiece[0] = chessBoard[click[0]][click[1]];
                currentPiece[1] = click[0];
                currentPiece[2] = click[1];
            }

            if (currentPiece[0] == N * currentPlayer)
            {
                isPieceSelected = true;
                horsePattern(click[0], click[1]);
            }
        }
        else if ((event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) && isPieceSelected)
        {
            al_get_mouse_state(&mouse);

            int *nextClick = getClickPosition(mouse.y, mouse.x);

            horseMove(currentPiece[1], currentPiece[2], nextClick[0], nextClick[1]);
            drawPieces(piecesBitmap);
            isPieceSelected = false;
        }

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break; // Exit the loop if the window is closed
        }
    }

    // Clean up resources
    al_destroy_display(display);
    al_destroy_event_queue(eventQueue);

    return 0;
}

// Loops throught the array and draws the pieces.
void drawPieces(ALLEGRO_BITMAP *bitmap)
{
    // Set the background color to white
    al_clear_to_color(al_map_rgb(17, 17, 0));

    // Draw a filled brown rectangle rectangle
    al_draw_filled_rectangle(50, 50, 850, 850, al_map_rgb(255, 255, 255));

    // Loop throught the chessBoard array and draw each piece.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Calculates the color of each rectangle in order to draw the border.
            ALLEGRO_COLOR color = (i + j) % 2 == 0 ? al_map_rgb(255, 255, 255) : al_map_rgb(51, 20, 0);
            al_draw_filled_rectangle(50 + (j * bitmap_size), 50 + (i * bitmap_size), 50 + (j * bitmap_size) + bitmap_size, 50 + (i * bitmap_size) + bitmap_size, color);

            int *bitmapPositions;

            // Get the piece to draw with the bitmaps position.
            bitmapPositions = pieceBitmapCoordinates(chessBoard[i][j]);
            al_draw_bitmap_region(bitmap, bitmapPositions[0], bitmapPositions[1], bitmap_size, bitmap_size, 50 + (j * bitmap_size), 50 + (i * bitmap_size), 0);

            // Frees the allocated memory.
            free(bitmapPositions);
        }
    }
    al_flip_display();
}

// Receives the value on the chess piece and returns it's position on the bitmap.
int *pieceBitmapCoordinates(int chessPieces)
{
    // Check if it's a white or black piece.
    int color = 0;
    if (chessPieces > 0)
    {
        color = 1;
    }

    // Allocate memory for 2 integers that stands for the top-left corner coordinates.
    int *position = malloc(2 * sizeof(int));
    if (position == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Switch case with the absolute value of the piece, getting it's X coordinate.
    switch (abs(chessPieces))
    {
    case 1:
        position[0] = bitmap_size * 5;
        break;
    case 2:
        position[0] = 0;
        break;
    case 3:
        position[0] = bitmap_size;
        break;
    case 4:
        position[0] = bitmap_size * 2;
        break;
    case 5:
        position[0] = bitmap_size * 3;
        break;
    case 6:
        position[0] = bitmap_size * 4;
        break;
    }

    // Gets the Y coordinate based on the color, if it's black, gets the second row of the bitmap.
    position[1] = 0 + (color * bitmap_size);
    return position;
}

// Returns which coordinate (X,Y) was clicked on the board.
int *getClickPosition(int y, int x)
{
    static int position[2] = {-1, -1};

    // Checks if the click is in the board or in the boarder, if it's in the boarder, returns -1 to both positions.
    if ((x > 50 && x < 850) && (y > 50 && y < 850))
    {
        position[0] = (y - 50) / bitmap_size;
        position[1] = (x - 50) / bitmap_size;
    }
    else
    {
        position[0] = -1;
        position[1] = -1;
    }
    return position;
}

// Loops through the board until it finds the king and check if it's checked.
bool isInCheck()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (chessBoard[i][j] == K * currentPlayer)
            {
                return isThreatened(j, i);
            }
        }
    }
    return false;
}

// Call all functions to check if a given position is threatened or not.
bool isThreatened(int y, int x)
{
    ThreatFunction threatFunctions[] = {
        isHorseThreat,
        isDiagonalThreat,
        isLineThreat,
        isPawnThreat,
        isKingThreat};
    for (int i = 0; i < sizeof(threatFunctions) / sizeof(threatFunctions[0]); i++)
    {
        if (threatFunctions[i](y, x))
        {
            return true;
        }
    }
    return false;
}

// Checks if there is any knight attacking the (X,Y) coordinate, pass the current player to ensure it's a opponent knight.
bool isHorseThreat(int y, int x)
{
    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = y - 2; i <= y + 2; i += 4)
    {
        // Checks if it's not offboard, if it is, then continue to the next loop interaction.
        if (i > 8 || i < 0)
            continue;

        // Procedes to check the two possible moves.
        for (int j = x - 1; j <= x + 1; j += 2)
        {

            if (j > 8 || j < 0)
                continue;
            // Checks if the position is equal to a knight of the opposite color. Player * -1 indicates that is from the opponent.
            if (chessBoard[i][j] == currentPlayer * N * -1)
            {
                return true;
            }
        }
    }
    // Does the same as above, but for the horizontal options.
    for (int j = x - 2; j <= x + 2; j += 4)
    {
        if (j > 8 || j < 0)
            continue;
        for (int i = y - 1; i <= y + 1; i += 2)
        {
            if (i > 8 || i < 0)
                continue;
            if (chessBoard[i][j] == currentPlayer * N * -1)
            {
                return true;
            }
        }
    }

    // If there is no threat, returns false.
    return false;
}

// Check for any attack on a diagonal.
bool isDiagonalThreat(int y, int x)
{
    int bishop = B * currentPlayer * -1;
    int queen = Q * currentPlayer * -1;
    int directions[4][2] =
        {
            {-1, -1},
            {-1, 1},
            {1, -1},
            {1, 1}};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; (i < 8 && i >= 0) && (j < 8 && j >= 0); i += directions[dir][0], j += directions[dir][1])
        {
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            if (((chessBoard[i][j] != bishop) && (chessBoard[i][j] != queen)) && (chessBoard[i][j] != 0))
            {
                break;
            }

            if ((chessBoard[i][j] == bishop) || (chessBoard[i][j] == queen))
            {
                return true;
            }
        }
    }
    return false;
}

// Check for any attack on a column/row.
bool isLineThreat(int y, int x)
{
    int rook = R * currentPlayer * -1;
    int queen = Q * currentPlayer * -1;
    int directions[4][2] =
        {
            {0, -1},
            {0, 1},
            {1, 0},
            {-1, 0}};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; (i < 8 && i >= 0) && (j < 8 && j >= 0); i += directions[dir][0], j += directions[dir][1])
        {
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            if (((chessBoard[i][j] != rook) && (chessBoard[i][j] != queen)) && (chessBoard[i][j] != 0))
            {
                break;
            }

            if ((chessBoard[i][j] == rook) || (chessBoard[i][j] == queen))
            {
                return true;
            }
        }
    }
    return false;
}

// Checks for pawn threats.
bool isPawnThreat(int y, int x)
{
    for (int i = -1; i <= 1; i++)
    {
        if (((x + i) > 8 || (x + i) < 0) || ((y + currentPlayer) > 8 || (y + currentPlayer) < 0))
        {
            continue;
        }
        if (chessBoard[x + i][y + currentPlayer] == currentPlayer * -1 * P)
        {
            return true;
        }
    }
    return false;
}

// Checks for threats of the king on a given position. Essential for endgames.
bool isKingThreat(int y, int x)
{
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if ((i > 8 && i < 0) || (j > 8 && j < 0))
            {
                continue;
            }
            if (chessBoard[i][j] == K * currentPlayer * -1)
            {
                return true;
            }
        }
    }
    return false;
}

// Check all possible moves for a given knight and draw the avaible moves.
void horsePattern(int y, int x)
{
    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = y - 2; i <= y + 2; i += 4)
    {
        // Checks if it's not offboard, if it is, then continue to the next loop interaction.
        if (i >= 8 || i < 0)
            continue;

        // Procedes to check the two possible moves.
        for (int j = x - 1; j <= x + 1; j += 2)
        {

            if (j >= 8 || j < 0)
                continue;

            if (!isValidMove(y, x, i, j))
                continue;
            drawValidMove(i, j);
        }
    }
    // Does the same as above, but for the horizontal options.
    for (int j = x - 2; j <= x + 2; j += 4)
    {
        if (j >= 8 || j < 0)
            continue;
        for (int i = y - 1; i <= y + 1; i += 2)
        {
            if (i >= 8 || i < 0)
                continue;

            if (!isValidMove(y, x, i, j))
                continue;
            drawValidMove(i, j);
        }
    }
    al_flip_display();
}

void horseMove(int curY, int curX, int nextY, int nextX)
{
    printf("CurX: %d,CurY: %d, NextX: %d,NextY: %d\n",curX,curY,nextX,nextY);
    fflush(stdout);
    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = curY - 2; i <= curY + 2; i += 4)
    {
        // Checks if it's not offboard, if it is, then continue to the next loop interaction.
        if (i >= 8 || i < 0)
            continue;

        // Procedes to check the two possible moves.
        for (int j = curX - 1; j <= curX + 1; j += 2)
        {

            if (j >= 8 || j < 0)
                continue;

            if (!isValidMove(curY, curX, i, j))
                continue;
            printf("NextY: %d,nextX: %d, i: %d, j: %d\n\n", nextY, nextX, i, j);
            fflush(stdout);
            if (nextY == i && nextX == j)
            {
                chessBoard[curY][curX] = 0;
                chessBoard[nextY][nextX] = N * currentPlayer;
            }
        }
    }
    // Does the same as above, but for the horizontal options.
    for (int j = curX - 2; j <= curX + 2; j += 4)
    {
        if (j >= 8 || j < 0)
            continue;
        for (int i = curY - 1; i <= curY + 1; i += 2)
        {
            if (i >= 8 || i < 0)
                continue;

            if (!isValidMove(curY, curX, i, j))
                continue;
            printf("NextY: %d,nextX: %d, i: %d, j: %d\n\n", nextY, nextX, i, j);
            fflush(stdout);
            if (nextY == i && nextX == j)
            {
                chessBoard[curY][curX] = 0;
                chessBoard[nextY][nextX] = N * currentPlayer;
            }
        }
    }
}

// Draws the avaible movements.
void drawValidMove(int y, int x)
{
    al_draw_filled_circle(100 + (bitmap_size * x), 100 + (bitmap_size * y), 25, al_map_rgb(255, 0, 0));
}

// Checks if a move creates a validateMove. Returns true if it does.
bool isValidMove(int curY, int curX, int nextY, int nextX)
{
    // Checks if the destination is occupied by a enemy piece.
    if ((chessBoard[curY][curX] * chessBoard[nextY][nextX] < 0) || (chessBoard[nextY][nextX] == 0))
    {
        // Copy the current piece position.
        int temp[3] = {chessBoard[curY][curX], curY, curX};

        chessBoard[curY][curX] = 0;
        if (isInCheck(currentPlayer))
        {
            chessBoard[temp[1]][temp[2]] = temp[0];
            return false;
        }
        else
        {
            chessBoard[temp[1]][temp[2]] = temp[0];
            return true;
        }
    }
    return false;
}