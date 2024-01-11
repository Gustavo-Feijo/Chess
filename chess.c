#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>

void draw_pieces(int chessPieces[8][8], ALLEGRO_BITMAP *bitmap);
bool validateMoviment(int chessPieces[8][8],int currentX,int currentY, int destinationX,int destinationY);
int *pieceBitmap(int chessPieces);

int main()
{
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
        fprintf(stderr, "Failed to initialize Allegro image addon!\n");
        return -1;
    }

    int window_width = 900;
    int window_height = 900;
    ALLEGRO_DISPLAY *display = al_create_display(window_width, window_height);

    if (!display)
    {
        printf("Failed to create display!\n");
        return -1;
    }

    al_set_window_title(display, "Chess");

    // Set up event queue to handle window close event
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_TIMER *timer2 = al_create_timer(1.0 / 60);

    ALLEGRO_BITMAP *piecesBitmap = al_load_bitmap("chessPieces.png");

    // Set the background color to white
    al_clear_to_color(al_map_rgb(17, 17, 0));

    // Draw a filled red rectangle
    al_draw_filled_rectangle(50, 50, 850, 850, al_map_rgb(255, 255, 255));

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            ALLEGRO_COLOR color = (i + j) % 2 == 0 ? al_map_rgb(255, 255, 255) : al_map_rgb(51, 20, 0);
            al_draw_filled_rectangle(50 + (j * 100), 50 + (i * 100), 50 + (j * 100) + 100, 50 + (i * 100) + 100, color);
        }
    }

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
    while (1)
    {
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
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

void draw_pieces(int chessPieces[8][8], ALLEGRO_BITMAP *bitmap)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int *bitmapPositions = pieceBitmap(chessPieces[i][j]);
            al_draw_bitmap_region(bitmap, bitmapPositions[0], bitmapPositions[1], 100, 100, 50 + (j * 100), 50 + (i * 100), 0);
            free(bitmapPositions);
        }
    }
    al_flip_display();
}

int *pieceBitmap(int chessPieces)
{
    int color = 0;
    if (chessPieces > 0)
    {
        color = 1;
    }

    int *position = malloc(2 * sizeof(int)); // Allocate memory for 4 integers
    if (position == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

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
    position[1] = 0 + (color * 100);
    return position;
}
bool validateMoviment(int chessPieces[8][8],int currentX,int currentY, int destinationX,int destinationY)
{
    int *kingPosition= findInBoard(*chessPieces,2);
}
int findInBoard(int chessPieces[8][8],int target)
{
    for(int i = 0; i < 8;i++)
    {
        for(int j = 0; j <8; j++)
        {
            if(abs(chessPieces[i][j]) == target)
            {
                int position[2]={i,j};
                return position;
            }
        }
    }
}
int findInDiagonal(int chessPieces[8][8],int x, int y)
{

}
int findInLine(int chessPieces[8][8],int x, int y)
{

}
int findInL(int chessPieces[8][8],int x, int y)
{

}