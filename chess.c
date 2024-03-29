#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define bitmap_size 100 // Size of each piece bitmap.
#define BLACK 1
#define WHITE -1
#define window_size 900

typedef bool (*ThreatFunction)(int, int);
bool isThreatened(int y, int x);
bool isInCheck();
bool isHorseThreat(int y, int x);
bool isDiagonalThreat(int y, int x);
bool isLineThreat(int y, int x);
bool isPawnThreat(int y, int x);
bool isKingThreat(int y, int x);

void drawPieces(ALLEGRO_BITMAP *bitmap);
void drawValidMove(int y, int x);
int *pieceBitmapCoordinates(int chessPieces);
int *getClickPosition(int y, int x);
bool isOffBoard(int position);

void kingMove(int y, int x);
void pawnMove(int y, int x);
void horseMove(int y, int x);
void bishopMove(int y, int x);
void rookMove(int y, int x);

bool isValidMove(int curY, int curX, int nextY, int nextX);
void validateMove(int y, int x);

void clearMovements();

bool isNoMoveAvaible();

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

// Creates a 2D array with the available movements.
// It's not the most efficient way to list the avaible movements, however,
// it's simple and pretty much easy to catch on.
bool movements[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}};

// Variables to game state control.
int currentPlayer = WHITE;

// Variable that holds the current piece (Y,X);
int currentPiece[2] = {0, 0};

// Variable that holds the current piece value.
int selectedPiece = 0;

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
    if (!al_init_font_addon() || !al_init_ttf_addon())
    {
        printf("Failed to initialize font and ttf!\n");
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

    // Creates the display, event queue, timer, event and mouse states.
    ALLEGRO_DISPLAY *display = al_create_display(window_size, window_size);
    ALLEGRO_EVENT_QUEUE *eventQueue = al_create_event_queue();
    ALLEGRO_BITMAP *piecesBitmap = al_load_bitmap("chessPieces.png");
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_FONT *font = al_load_ttf_font("arial_bold.ttf", 60, 0);
    ALLEGRO_EVENT event;
    ALLEGRO_MOUSE_STATE mouse;

    // Confirms the display was created and set the title.
    if (!display)
    {
        printf("Failed to create display!\n");
        return -1;
    }
    // Confirms the display was created and set the title.
    if (!timer)
    {
        printf("Failed to create timer!\n");
        return -1;
    }
    // Confirms the display was created and set the title.
    if (!eventQueue)
    {
        printf("Failed to create event queue!\n");
        return -1;
    }
    // Confirms the display was created and set the title.
    if (!piecesBitmap)
    {
        printf("Failed to create bitmap!\n");
        return -1;
    }

    al_set_window_title(display, "Chess");

    // Set up event queue to handle events.
    al_register_event_source(eventQueue, al_get_display_event_source(display));
    al_register_event_source(eventQueue, al_get_mouse_event_source());

    // Draw the initial state of the board.
    drawPieces(piecesBitmap);

    // White pieces to move first.

    while (timer)
    {
        al_wait_for_event(eventQueue, &event);

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            al_get_mouse_state(&mouse);

            // Check if the mouse click was not offboard.
            if ((mouse.x > 50 && mouse.x < 850) && (mouse.y > 50 && mouse.y < 850))
            {
                int *click = getClickPosition(mouse.y, mouse.x);

                // Verify if a piece is selected, if it's not, check for a click on a ally piece and select it.
                if (movements[click[0]][click[1]] == 0 && ((chessBoard[click[0]][click[1]] * currentPlayer) > 0))
                {
                    // Clear any previous movement not done.
                    clearMovements();
                    drawPieces(piecesBitmap);

                    selectedPiece = chessBoard[click[0]][click[1]];
                    currentPiece[0] = click[0];
                    currentPiece[1] = click[1];
                    validateMove(click[0], click[1]);
                    al_flip_display();
                }
                else if (movements[click[0]][click[1]] == 1)
                {
                    // Check for a click on a position that is a valid movement.
                    chessBoard[currentPiece[0]][currentPiece[1]] = 0;
                    chessBoard[click[0]][click[1]] = selectedPiece;
                    drawPieces(piecesBitmap);
                    al_flip_display();
                    clearMovements();
                    currentPlayer *= -1;

                    if (isInCheck())
                    {
                        if (isNoMoveAvaible())
                        {
                            fflush(stdout);
                            al_draw_filled_rectangle(200, 200, 700, 700, al_map_rgb(255, 255, 0));
                            al_draw_text(font, al_map_rgb(0, 0, 0), 250, 400, 0, "CHECKMATE");
                            al_flip_display();
                            al_rest(4.0);
                            break;
                        }
                    }
                    else if (isNoMoveAvaible())
                    {
                        fflush(stdout);
                        al_draw_filled_rectangle(200, 200, 700, 700, al_map_rgb(255, 255, 0));
                        al_draw_text(font, al_map_rgb(0, 0, 0), 250, 400, 0, "STALEMATE");
                        al_flip_display();
                        al_rest(4.0);
                        break;
                    }

                    drawPieces(piecesBitmap);
                    al_flip_display();
                }
                else
                {
                    // If none of the above, then desselect the piece and clear the possible moves selection.
                    clearMovements();
                    drawPieces(piecesBitmap);
                    al_flip_display();
                }
            }
        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break; // Exit the loop if the window is closed
        }
    }

    // Clean up resources
    al_destroy_bitmap(piecesBitmap);
    al_destroy_display(display);
    al_destroy_event_queue(eventQueue);
    al_destroy_timer(timer);

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

    // Returns the respective position of the clicked coordinate.
    position[0] = (y - 50) / bitmap_size;
    position[1] = (x - 50) / bitmap_size;

    return position;
}

// Loops through the board until it finds the king and check if it's threatened.
bool isInCheck()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (chessBoard[i][j] == K * currentPlayer)
            {
                return isThreatened(i, j);
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
    // Loops through each threat function and, if any is true, it's threatened.
    for (int i = 0; i < sizeof(threatFunctions) / sizeof(threatFunctions[0]); i++)
    {
        if (threatFunctions[i](y, x))
        {
            return true;
        }
    }
    return false;
}

// Checks if there is any knight attacking the (Y,X) coordinate, pass the current player to ensure it's a opponent knight.
bool isHorseThreat(int y, int x)
{
    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = y - 2; i <= y + 2; i += 4)
    {
        // Checks if it's not offboard, if it is, then continue to the next loop interaction.
        if (isOffBoard(i))
            continue;

        // Procedes to check the two possible moves.
        for (int j = x - 1; j <= x + 1; j += 2)
        {

            if (isOffBoard(j))
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
        if (isOffBoard(j))
            continue;

        for (int i = y - 1; i <= y + 1; i += 2)
        {
            if (isOffBoard(i))
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
    int enemyBishop = B * currentPlayer * -1;
    int enemyQueen = Q * currentPlayer * -1;
    int directions[4][2] =
        {
            {-1, -1},
            {-1, 1},
            {1, -1},
            {1, 1}};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; (!isOffBoard(i)) && (!isOffBoard(j)); i += directions[dir][0], j += directions[dir][1])
        {
            // If the value on a given position multiplied by the current player is greater than one, it's a piece of same color.
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            if (((chessBoard[i][j] != enemyBishop) && (chessBoard[i][j] != enemyQueen)) && (chessBoard[i][j] != 0))
            {
                break;
            }

            if ((chessBoard[i][j] == enemyBishop) || (chessBoard[i][j] == enemyQueen))
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
    int enemyRook = R * currentPlayer * -1;
    int enemyQueen = Q * currentPlayer * -1;
    int directions[4][2] =
        {
            {0, -1},
            {0, 1},
            {1, 0},
            {-1, 0}};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; (!isOffBoard(i)) && (!isOffBoard(j)); i += directions[dir][0], j += directions[dir][1])
        {
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            if (((chessBoard[i][j] != enemyRook) && (chessBoard[i][j] != enemyQueen)) && (chessBoard[i][j] != 0))
            {
                break;
            }

            if ((chessBoard[i][j] == enemyRook) || (chessBoard[i][j] == enemyQueen))
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
    for (int i = x - 1; i <= x + 1; i += 2)
    {
        if (isOffBoard(i) || isOffBoard(y + currentPlayer))
            continue;

        if (chessBoard[y + currentPlayer][i] == currentPlayer * -1 * P)
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
        if (isOffBoard(i))
            continue;
        for (int j = -1; j <= 1; j++)
        {
            if (isOffBoard(j))
                continue;
            if (chessBoard[i][j] == K * currentPlayer * -1)
            {
                return true;
            }
        }
    }
    return false;
}
// Function to draw and enable a pawn move.
void kingMove(int y, int x)
{
    for (int i = y - 1; i <= y + 1; i++)
    {
        if (isOffBoard(i))
            continue;

        for (int j = x - 1; j <= x + 1; j++)
        {
            if (isOffBoard(j))
                continue;

            if (!isValidMove(y, x, i, j))
            {
                continue;
            }
            movements[i][j] = 1;
            drawValidMove(i, j);
        }
    }
}

// Function to draw and enable a pawn move.
void pawnMove(int y, int x)
{
    // Normal movement for a pawn, one square to the front.
    int moveSize = 1;

    // Checks if the selected pawn was already moved before, if it wasn't, make it able to jump two squares.
    if (((currentPlayer == WHITE) && (y == 6)) || ((currentPlayer == BLACK) && (y == 1)))
    {
        moveSize++;
    }
    for (int i = 1; i <= moveSize; i++)
    {
        if ((y + i * currentPlayer) < 0 || (y + i * currentPlayer) >= 8)
        {
            continue;
        }
        if (chessBoard[y + i * currentPlayer][x] != 0)
        {
            break;
        }
        if (!isValidMove(y, x, (y + i * currentPlayer), x))
        {
            break;
        }
        movements[y + i * currentPlayer][x] = 1;
        drawValidMove((y + i * currentPlayer), x);
    }
    for (int j = x - 1; j <= x + 1; j += 2)
    {
        if (isOffBoard(j) || isOffBoard(y + 1 * currentPlayer))
            continue;

        if (chessBoard[y + 1 * currentPlayer][j] * currentPlayer < 0)
        {

            if (!isValidMove(y, x, (y + 1 * currentPlayer), (j)))
            {
                break;
            }
            movements[y + 1 * currentPlayer][j] = 1;
            drawValidMove((y + 1 * currentPlayer), j);
        }
    }
}

// Function to draw and enable the rook movement.
// Can be refactored. A function that only receives the directions and runs the loop for can remove a lot of lines.
void rookMove(int y, int x)
{
    // Direction of each possible move of the rook.
    int directions[4][2] = {
        {0, -1}, // Move left
        {0, 1},  // Move right
        {1, 0},  // Move down
        {-1, 0}  // Move up
    };

    // Loops through each direction, verifying all of them.
    for (int dir = 0; dir < 4; dir++)
    {
        // Loops throught each direction until they reach the limits of the board.
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; ((!isOffBoard(i)) && (!isOffBoard(j))); i += directions[dir][0], j += directions[dir][1])
        {
            // If the position on the direction multiplied by the current player is positive, then it's blocked by a ally piece.
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }
            if ((chessBoard[i][j] * currentPlayer) < 0)
            {
                movements[i][j] = 1;
                drawValidMove(i, j);
                break;
            }
            // Checks if the move creates a discovered check, if it does, break the loop.
            if (!isValidMove(y, x, i, j))
            {
                continue;
            }

            // Change the value on the movements array at the given position to identify that is a valid move.
            movements[i][j] = 1;
            drawValidMove(i, j);
        }
    }
}

// Function to draw and enable the bishop movement. Essentially works the same as the rookMove, only changing the directions.
void bishopMove(int y, int x)
{
    int directions[4][2] =
        {
            {-1, -1},
            {1, 1},
            {1, -1},
            {-1, 1}};

    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; ((!isOffBoard(i)) && (!isOffBoard(j))); i += directions[dir][0], j += directions[dir][1])
        {
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            if (!isValidMove(y, x, i, j))
                break;
            movements[i][j] = 1;
            drawValidMove(i, j);

            if (chessBoard[i][j] * currentPlayer < 0)
                break;
        }
    }
}

// Function to draw and enable the horse movement.
void horseMove(int y, int x)
{

    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = y - 2; i <= y + 2; i += 4)
    {
        // Checks if it's not offboard, if it is, then continue to the next loop iteraction.
        if (isOffBoard(i))
            continue;

        // Proceds to check the two movement options to the given direction.
        for (int j = x - 1; j <= x + 1; j += 2)
        {

            // Checking for offboard.
            if (isOffBoard(j))
                continue;

            // Do the validation of the move, if it's not a valid one, just continue to the next loop iteration.
            if (!isValidMove(y, x, i, j))
            {
                continue;
            }

            // Change the value on the movements array at the given position to identify that is a valid move.
            movements[i][j] = 1;

            // Draw a red dot above the avaible moves.
            drawValidMove(i, j);
        }
    }
    // Does the same as the code above, only changing the direction.
    for (int j = x - 2; j <= x + 2; j += 4)
    {
        if (isOffBoard(j))
            continue;

        for (int i = y - 1; i <= y + 1; i += 2)
        {

            if (isOffBoard(i))
                continue;

            if (!isValidMove(y, x, i, j))
                continue;

            movements[i][j] = 1;

            drawValidMove(i, j);
        }
    }
}

// Receives a coordinate and draws a red dot on the respective position on the board.
void drawValidMove(int y, int x)
{
    al_draw_filled_circle(100 + (bitmap_size * x), 100 + (bitmap_size * y), 25, al_map_rgb(255, 0, 0));
}

// Checks if a move creates a discovered check.
bool isValidMove(int curY, int curX, int nextY, int nextX)
{
    // Check if the current position multiplied by the next position is negative.
    // If it is, implies that they are from different colors, making the capture possible.
    // If it's a blank square, the multiplication value is going to be 0.
    if ((chessBoard[curY][curX] * chessBoard[nextY][nextX] <= 0))
    {

        // Copy the current position of the selected piece into a temporary array.
        int temp[3] = {chessBoard[curY][curX], curY, curX};
        int tempNext[3] = {chessBoard[nextY][nextX], nextY, nextX};

        // Sets the selected square to 0, to enable the checking verification.
        chessBoard[nextY][nextX] = chessBoard[curY][curX];
        chessBoard[curY][curX] = 0;

        // Assing a variable to the result of the checking of the verification of check.
        bool isChecked = isInCheck();

        // Return the square for it's original position.
        chessBoard[temp[1]][temp[2]] = temp[0];
        chessBoard[tempNext[1]][tempNext[2]] = tempNext[0];

        // Return if the king was put in check, if it was, the position turns to be invalid.
        return !isChecked;
    }

    // If the previous if conditions are not met, the move is impossible to be performed.
    return false;
}

// Receives the piece that was first selected and call the function to validate the possible moves and display them.
void validateMove(int y, int x)
{
    switch (abs(selectedPiece))
    {
    case K:
        kingMove(y, x);
        break;
    case N:
        horseMove(y, x);
        break;
    case Q:
        bishopMove(y, x);
        rookMove(y, x);

        break;
    case P:
        pawnMove(y, x);
        break;
    case R:
        rookMove(y, x);
        break;
    case B:
        bishopMove(y, x);
        break;
    }
}

// Clear the 2D array with the current avaible moviments.
void clearMovements()
{
    // Desselect any piece.
    selectedPiece = 0;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            movements[i][j] = 0;
        }
    }
}

// Verifies if there is a move avaible, usefull for checkmates and stalemates.
bool isNoMoveAvaible()
{

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (chessBoard[i][j] * currentPlayer > 0)
            {
                switch (abs(chessBoard[i][j]))
                {
                case K:
                    kingMove(i, j);
                    break;
                case N:
                    horseMove(i, j);
                    break;
                case Q:
                    rookMove(i, j);
                    bishopMove(i, j);
                    break;
                case P:
                    pawnMove(i, j);
                    break;
                case R:
                    rookMove(i, j);
                    break;
                case B:
                    bishopMove(i, j);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (movements[i][j] == 1)
            {
                clearMovements();
                return false;
            }
        }
    }
    return true;
}

// Checks if a position is offboard;
bool isOffBoard(int position)
{
    return (position < 0 || position >= 8);
}