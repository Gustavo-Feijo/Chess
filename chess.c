#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>

void draw_pieces(int chessPieces[8][8], ALLEGRO_BITMAP *bitmap);
int *pieceBitmapCoordinates(int chessPieces);
void draw_possible_movements(int chessPieces[8][8], int chessPiece, int x, int y);
int *getClickPosition(int x, int y);

int main()
{

    // Allegro 5 initialization functions.
    if (!al_init())
    {
        printf("Failed to initialize Allegro!\n");
        fflush(stdout);
        return -1;
    }
    if (!al_install_mouse())
    {
        printf("Failed to initialize mouse!\n");
        fflush(stdout);
        return -1;
    }
    if (!al_init_primitives_addon())
    {
        printf("Failed to initialize Allegro primitives addon!\n");
        fflush(stdout);
        return -1;
    }
    if (!al_init_image_addon())
    {
        printf("Failed to initialize Allegro image addon!\n");
        fflush(stdout);
        return -1;
    }

    // Create a 900x900 display.
    int window_width = 900;
    int window_height = 900;
    ALLEGRO_DISPLAY *display = al_create_display(window_width, window_height);

    // Confirms the display was created and set the title.
    if (!display)
    {
        printf("Failed to create display!\n");
        return -1;
    }
    al_set_window_title(display, "Chess");

    // Set up event queue to handle events.
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());

    // Innitializes the bitmap for the chess pieces.
    ALLEGRO_BITMAP *piecesBitmap = al_load_bitmap("chessPieces.png");

    int pieces[8][8] = {
        {6, 5, 4, 3, 2, 4, 5, 6},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-6, -5, -4, -3, -2, -4, -5, -6}};

    draw_pieces(pieces, piecesBitmap);

    ALLEGRO_EVENT event;
    ALLEGRO_MOUSE_STATE mouse;

    int current_player = -1;
    int current_piece;
    while (1)
    {
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {

            al_get_mouse_state(&mouse);

            int *clickPosition = getClickPosition(mouse.x, mouse.y);
            current_piece = pieces[clickPosition[0]][clickPosition[1]];
            draw_pieces(pieces, piecesBitmap);
        }

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break; // Exit the loop if the window is closed
        }
    }

    // Clean up resources
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}

// Loops throught the array and draws the pieces.
void draw_pieces(int chessBoard[8][8], ALLEGRO_BITMAP *bitmap)
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
            al_draw_filled_rectangle(50 + (j * 100), 50 + (i * 100), 50 + (j * 100) + 100, 50 + (i * 100) + 100, color);

            int *bitmapPositions;

            // Get the piece to draw with the bitmaps position.
            bitmapPositions = pieceBitmapCoordinates(chessBoard[i][j]);
            al_draw_bitmap_region(bitmap, bitmapPositions[0], bitmapPositions[1], 100, 100, 50 + (j * 100), 50 + (i * 100), 0);

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
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Switch case with the absolute value of the piece, getting it's X coordinate.
    switch (abs(chessPieces))
    {
    case 1:
        position[0] = 500;
        break;
    case 2:
        position[0] = 0;
        break;
    case 3:
        position[0] = 100;
        break;
    case 4:
        position[0] = 200;
        break;
    case 5:
        position[0] = 300;
        break;
    case 6:
        position[0] = 400;
        break;
    }

    // Gets the Y coordinate based on the color, if it's black, gets the second row of the bitmap.
    position[1] = 0 + (color * 100);
    return position;
}

//Returns which coordinate (X,Y) was clicked on the board.
int *getClickPosition(int x, int y)
{
    static int position[2] = {-1, -1};

    if ((x > 50 && x < 850) && (y > 50 && y < 850))
    {
        position[0] = (x - 50) / 100;
        position[1] = (y - 50) / 100;
    }
    else
    {
        position[0] = -1;
        position[1] = -1;
    }
    return position;
}
