#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define bitmap_size 100
#define BLACK 1
#define WHITE -1
#define screen_size 900

typedef bool (*PatternFunction)(int, int, bool);

bool isThreatened(int y, int x);
bool isInCheck();

bool isValidMove(int curY, int curX, int nextY, int nextX);
int validateMove(int y, int x);

void drawPieces(ALLEGRO_BITMAP *bitmap);
void drawValidMove(int y, int x);

int *pieceBitmapCoordinates(int chessPieces);
int *getClickPosition(int y, int x);

bool diagonalPattern(int y, int x, bool verifyCheck);
bool horsePattern(int x, int y, bool verifyCheck);
bool kingPattern(int y, int x, bool verifyCheck);
bool linePattern(int y, int x, bool verifyCheck);
bool pawnPattern(int y, int x, bool verifyCheck);

bool isOffBoard(int y, int x);

void clearMovements();

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
bool movements[8][8] =
    {
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

// Variable that holds the current selected piece position.
//[0] = y; [1] = x;
// Coordinates: (Y,X);
int currentPiece[2] = {0, 0};

// Value of the selected current selected piece.
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

    // Declare the display, event queue, timer and bitmap. Initialize the event and mouse.
    ALLEGRO_DISPLAY *display = al_create_display(screen_size, screen_size);
    ALLEGRO_EVENT_QUEUE *eventQueue = al_create_event_queue();
    ALLEGRO_BITMAP *piecesBitmap = al_load_bitmap("chessPieces.png");
    ALLEGRO_EVENT event;
    ALLEGRO_MOUSE_STATE mouse;
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    al_set_window_title(display, "Chess");

    // Confirms everything was created successfully.
    if (!display)
    {
        printf("Failed to create display!\n");
        return -1;
    }
    if (!eventQueue)
    {
        printf("Failed to create eventQueue!\n");
        return -1;
    }
    if (!timer)
    {
        printf("Failed to create timer!\n");
        return -1;
    }
    if (!piecesBitmap)
    {
        printf("Failed to create bitmap!\n");
        return -1;
    }

    // Set up event queue to handle events.
    al_register_event_source(eventQueue, al_get_display_event_source(display));
    al_register_event_source(eventQueue, al_get_mouse_event_source());

    // Draw the initial state of the display.
    drawPieces(piecesBitmap);

    while (timer)
    {
        al_wait_for_event(eventQueue, &event);

        // Get a mouse click event.
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            // Gets the current mouse position on the board, ranging from 0 to 7.
            al_get_mouse_state(&mouse);

            // If the click was on the board range, gets the click position.
            if ((mouse.x > 50 && mouse.x < 850) && (mouse.y > 50 && mouse.y < 850))
            {
                int *click = getClickPosition(mouse.y, mouse.x);
                // selectedPiece is the current selected piece, 0 means there is no selected piece, so it's possible to select one.
                // The second condition is a checking if the value of the piece on the click position
                if (selectedPiece == 0 && ((chessBoard[click[0]][click[1]] * currentPlayer) > 0))
                {
                    // Sets the selected piece value and position as the clicked one.
                    selectedPiece = chessBoard[click[0]][click[1]];
                    currentPiece[0] = click[0];
                    currentPiece[1] = click[1];

                    // Calls the function to validate the possible moves.
                    validateMove(click[0], click[1]);

                    // Flip the display to show the possible moves.
                    al_flip_display();
                }
                else if (movements[click[0]][click[1]] == 1)
                {

                    // Does the move and clear the movements array.
                    chessBoard[currentPiece[0]][currentPiece[1]] = 0;
                    chessBoard[click[0]][click[1]] = selectedPiece;
                    clearMovements();

                    // Draws the actualized board.
                    drawPieces(piecesBitmap);
                    al_flip_display();

                    // Changes the player.
                    currentPlayer *= -1;
                }
                else
                {
                    // If none of the above, resets the current possible moves and redraw the board.
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
    al_destroy_display(display);
    al_destroy_event_queue(eventQueue);
    al_destroy_bitmap(piecesBitmap);
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
    for (int col = 0; col < 8; col++)
    {
        for (int row = 0; row < 8; row++)
        {
            // Calculates the color of each rectangle in order to draw the border.
            ALLEGRO_COLOR color = (col + row) % 2 == 0 ? al_map_rgb(255, 255, 255) : al_map_rgb(51, 20, 0);
            al_draw_filled_rectangle(50 + (row * bitmap_size), 50 + (col * bitmap_size), 50 + (row * bitmap_size) + bitmap_size, 50 + (col * bitmap_size) + bitmap_size, color);

            int *bitmapPositions;

            // Get the piece to draw with the bitmaps position.
            bitmapPositions = pieceBitmapCoordinates(chessBoard[col][row]);
            al_draw_bitmap_region(bitmap, bitmapPositions[0], bitmapPositions[1], bitmap_size, bitmap_size, 50 + (row * bitmap_size), 50 + (col * bitmap_size), 0);

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

    // If it's a positive piece, then it's a black piece.
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
    // Creates a static int that represents the position of the click.
    static int position[2] = {-1, -1};

    // Sets it's values.
    position[0] = (y - 50) / bitmap_size;
    position[1] = (x - 50) / bitmap_size;

    return position;
}

// Loops through the board until it finds the king and check if it's checked.
bool isInCheck()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // If the position is the king of the current player, then check is it's threatened.
            if (chessBoard[i][j] == K * currentPlayer)
            {
                return isThreatened(i, j);
            }
        }
    }
    return false;
}

// Call all pattern functions to check if a given position is threatened or not.
bool isThreatened(int y, int x)
{
    PatternFunction patternFunction[] = {
        diagonalPattern,
        horsePattern,
        kingPattern,
        linePattern,
        pawnPattern};
    for (int i = 0; i < sizeof(patternFunction) / sizeof(patternFunction[0]); i++)
    {
        if (patternFunction[i](y, x, true))
        {
            return true;
        }
    }
    return false;
}

// Receives a coordinate and draws a red dot on the respective position on the board.
void drawValidMove(int y, int x)
{
    al_draw_filled_circle(100 + (bitmap_size * x), 100 + (bitmap_size * y), 25, al_map_rgb(255, 0, 0));
}

// Checks if a the move block a check or create a discovered check.
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
int validateMove(int y, int x)
{
    switch (abs(selectedPiece))
    {
    case K:
        kingPattern(y, x, false);
        break;
    case N:
        horsePattern(y, x, false);
        break;
    case Q:
        linePattern(y, x, false);
        diagonalPattern(y, x, false);
        break;
    case P:
        pawnPattern(y, x, false);
        break;
    case R:
        linePattern(y, x, false);
        break;
    case B:
        diagonalPattern(y, x, false);
        break;
    }
}

// Checks if a position is offboard.
bool isOffBoard(int y, int x)
{
    return (x < 0 || x > 7 || y < 0 || y > 7);
}

// Clear the 2D array with the current avaible moviments.
void clearMovements()
{
    // Resets the selected piece.
    selectedPiece = 0;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            movements[i][j] = 0;
        }
    }
}

bool horsePattern(int y, int x, bool verifyCheck)
{
    // i = y - 2 checks for the positions below the current, then increases by 4 to check the above positions.
    for (int i = y - 2; i <= y + 2; i += 4)
    {
        // Procedes to check the two possible moves.
        for (int j = x - 1; j <= x + 1; j += 2)
        {

            if (isOffBoard(i, j))
                continue;

            // Switch case to determinate which is the function of the current call.
            // If it's false, then it's a validation of a movement to be done by a horse.
            // If it's false, it checks for a horse checking the current position.
            switch (verifyCheck)
            {
            case 0:
                // Do the validation of the move, if it's not a valid one, just continue to the next loop iteration.
                if (!isValidMove(y, x, i, j))
                {

                    continue;
                }

                // Change the value on the movements array at the given position to identify that is a valid move.
                movements[i][j] = 1;
                drawValidMove(i, j);
                break;
            case 1:
                // Checks if the position is equal to a knight of the opposite color. Player * -1 indicates that is from the opponent.
                if (chessBoard[i][j] == currentPlayer * N * -1)
                {
                    return true;
                }
            }
        }
    }
    // Does the same as above, but for the horizontal options.
    for (int j = x - 2; j <= x + 2; j += 4)
    {
        for (int i = y - 1; i <= y + 1; i += 2)
        {
            if (isOffBoard(i, j))
                continue;
            switch (verifyCheck)
            {
            case 0:
                // Do the validation of the move, if it's not a valid one, just continue to the next loop iteration.
                if (!isValidMove(y, x, i, j))
                {
                    continue;
                }

                // Change the value on the movements array at the given position to identify that is a valid move.
                movements[i][j] = 1;
                drawValidMove(i, j);
                break;
            case 1:
                // Checks if the position is equal to a knight of the opposite color. Player * -1 indicates that is from the opponent.
                if (chessBoard[i][j] == currentPlayer * N * -1)
                {

                    return true;
                }
            }
        }
    }

    // If there is no threat, returns false.
    return false;
}
// Loops throught the diagonals.
// The verifyCheck variable determines whether it looks for a valid move or a check on the passed coordinates.
bool diagonalPattern(int y, int x, bool verifyCheck)
{
    // Set the value of the enemy bishop and queen to verify checks.
    int bishop = B * currentPlayer * -1;
    int queen = Q * currentPlayer * -1;

    // Directios that the bishop can go into.
    int directions[4][2] =
        {
            {-1, -1},
            {-1, 1},
            {1, -1},
            {1, 1}};

    // Loops through each direction and do the verifications.
    for (int dir = 0; dir < 4; dir++)
    {
        for (int i = y + directions[dir][0], j = x + directions[dir][1]; (i < 8 && i >= 0) && (j < 8 && j >= 0); i += directions[dir][0], j += directions[dir][1])
        {
            if ((chessBoard[i][j] * currentPlayer) > 0)
            {
                break;
            }

            switch (verifyCheck)
            {
            case 0:
            {
                if (chessBoard[i][j] * currentPlayer < 0)
                {
                    movements[i][j] = 1;
                    drawValidMove(i, j);
                    break;
                }
                if (!isValidMove(y, x, i, j))
                    break;
                movements[i][j] = 1;
                drawValidMove(i, j);
            }
            break;
            case 1:
            {
                if (((chessBoard[i][j] != bishop) && (chessBoard[i][j] != queen)) && (chessBoard[i][j] != 0))
                {
                    break;
                }

                if ((chessBoard[i][j] == bishop) || (chessBoard[i][j] == queen))
                {
                    return true;
                }
            }
            break;
            }
        }
    }
    return false;
}

//Loops throught each direction.
//If verifyCheck is false, check the avaible movements of the piece.
//If verifyCheck is true, verify if the position have a threat on the same line.
bool linePattern(int y, int x, bool verifyCheck)
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

            switch (verifyCheck)
            {
            case 0:
            {
                // Checks if the move creates a discovered check, if it does, break the loop.
                if (!isValidMove(y, x, i, j))
                    break;

                // Change the value on the movements array at the given position to identify that is a valid move.
                movements[i][j] = 1;

                drawValidMove(i, j);
            }
            break;

            case 1:
            {
                if (((chessBoard[i][j] != rook) && (chessBoard[i][j] != queen)) && (chessBoard[i][j] != 0))
                {
                    break;
                }

                if ((chessBoard[i][j] == rook) || (chessBoard[i][j] == queen))
                {
                    return true;
                }
            }
            break;
            }
        }
    }
    return false;
}

//Verify if a pawn move is avaible or if a given position is threated by a pawn.
bool pawnPattern(int y, int x, bool verifyCheck)
{
    switch (verifyCheck)
    {
    case 0:
    { // Normal movement for a pawn, one square to the front.
        int moveSize = 1;

        // Checks if the selected pawn was already moved before, if it wasn't, make it able to jump two squares.
        if (((currentPlayer == WHITE) && (y == 6)) || ((currentPlayer == BLACK) && (y == 1)))
        {
            moveSize++;
        }
        for (int i = 1; i <= moveSize; i++)
        {

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
        for (int j = -1; j <= 1; j += 2)
        {
            if (chessBoard[y + 1 * currentPlayer][x + j] * currentPlayer < 0)
            {

                if (!isValidMove(y, x, (y + 1 * currentPlayer), (x + j)))
                {
                    break;
                }
                movements[y + 1 * currentPlayer][x + j] = 1;
                drawValidMove((y + 1 * currentPlayer), x + j);
            }
        }
    }
    break;
    case 1:
    {
        for (int i = -1; i <= 1; i++)
        {
            if (((x + i) > 8 || (x + i) < 0) || ((y + currentPlayer) > 8 || (y + currentPlayer) < 0))
            {
                continue;
            }
            if (chessBoard[y + currentPlayer][x + i] == currentPlayer * -1 * P)
            {
                return true;
            }
        }
    }
    break;
    }
    return false;
}

// Checks for threats of the king on a given position and the king moves.
bool kingPattern(int y, int x, bool verifyCheck)
{
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (isOffBoard((y + i), (x + j)))
            {
                continue;
            }
            switch (verifyCheck)
            {
            case 0:
            {
                if (!isValidMove(y, x, y + i, x + j))
                {

                    continue;
                }
                movements[y + i][x + j] = 1;
                drawValidMove((y + i), (x + j));
            }
            break;
            case 1:
            {
                if (chessBoard[i][j] == K * currentPlayer * -1)
                {
                    return true;
                }
            }
            break;
            }
        }
    }
    return false;
}