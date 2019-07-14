#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <SDL.h>

#define FPS 120
#define SLEEPTIME (1000 / FPS)

#define CAP_STRENGTH 5

typedef uint32_t pixel_t;

typedef struct {
    int color;
    int strength;
} cell_t;

enum {
    WHITE,
    ROCK,
    PAPER,
    SCISSOR,
    NUM_OPTIONS
};

pixel_t colors[NUM_OPTIONS] = {
    [WHITE]   = 0xffffffff,
    [ROCK]    = 0x404040ff,
    [PAPER]   = 0x8c8c8cff,
    [SCISSOR] = 0xd9d9d9ff
};

cell_t *cell_grid_a = NULL; /* Always points to the last modified grid */
cell_t *cell_grid_b = NULL;

pixel_t *pixels = NULL;

#define WIDTH 800
#define HEIGHT 600

int window_w  = WIDTH,
    window_h  = HEIGHT;

int texture_w = WIDTH / 4,
    texture_h = HEIGHT / 4;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

#define grid_a(x, y) cell_grid_a[(texture_w * (y)) + (x)]
#define grid_b(x, y) cell_grid_b[(texture_w * (y)) + (x)]
#define pixel(x, y) pixels[(texture_w * (y)) + (x)]

void swap(cell_t **a, cell_t **b)
{
    cell_t *c = *a;
    *a = *b;
    *b = c;
}

void perturbate_cell_grid_tri(SDL_Texture *texture)
{
    int cx, cy, dx, dy;

    cx = texture_w / 2;
    cy = texture_h / 2;
    dx = texture_w / 4;
    dy = texture_h / 4;

    grid_a(cx - dx, cy - dy).color = ROCK;
    pixel(cx - dx, cy - dy)        = colors[ROCK];
    grid_a(cx + dx, cy - dy).color = PAPER;
    pixel(cx + dx, cy - dy)        = colors[PAPER];
    grid_a(cx, cy + dy).color      = SCISSOR;
    pixel(cx, cy + dy)             = colors[SCISSOR];

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

void perturbate_cell_grid_rand(SDL_Texture *texture)
{
    int x, y, option;
    int num_cells_init = 50;
    for (int i = 0; i < num_cells_init; i++)
    {
        x = rand() % texture_w;
        y = rand() % texture_h;
        option = (rand() % (NUM_OPTIONS - 1)) + 1; /* Exclude white */
        grid_a(x, y).color = option;
        pixel(x, y) = colors[option];
    }

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

int beats(cell_t current, cell_t neighbor)
{
    return
        ((current.color == SCISSOR) && (neighbor.color == PAPER)) ||
        ((current.color == PAPER) && (neighbor.color == ROCK))    ||
        ((current.color == ROCK) && (neighbor.color == SCISSOR))  ||
        (current.color == neighbor.color);
}

cell_t next_cell(int x, int y)
{
#ifdef RPS_CELL_ALL
    int d[3] = { -1, 0, 1 };
    int dx = x + d[rand() % 3];
    int dy = y + d[rand() % 3];
    if (dx == 0 && dy == 0)
        dy++;
#elif RPS_CELL_DIAG
    int d[2] = { -1, 1 };
    int dx = x + d[rand() % 2];
    int dy = y + d[rand() % 2];
#endif /* RPS_RULE_ALL */

    if (dx < 0)
        dx = 1;
    else if (dx >= texture_w)
        dx = texture_w - 2;
    if (dy < 0)
        dy = 1;
    else if (dy >= texture_h)
        dy = texture_h - 2;

    cell_t current = grid_a(x, y);
    cell_t neighbor = grid_a(dx, dy);

    if (neighbor.color == WHITE)
        return current;

    cell_t next = current;

    if (current.color == WHITE)
    {
        next.strength = 1;
        next.color = neighbor.color;
    }
    else
    {
        if (beats(current, neighbor))
            next.strength++;
        else
            next.strength--;
    }

    if (next.strength == 0)
    {
        next.color = neighbor.color;
        next.strength = 1;
    }

    if (next.strength >= CAP_STRENGTH)
        next.strength = CAP_STRENGTH;

    return next;
}

void evaluate_cell_grid(SDL_Texture *texture)
{
    for (int y = 0; y < texture_h; y++)
        for (int x = 0; x < texture_w; x++)
        {
            cell_t next = next_cell(x, y);
            grid_b(x, y) = next;
            pixel(x, y) = colors[next.color];
        }

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
    swap(&cell_grid_a, &cell_grid_b);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    Uint32 flags = SDL_WINDOW_HIDDEN;

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "SDL_Init(SDL_INIT_VIDEO) failed: %s\n",
            SDL_GetError());
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(0, 0, flags, &window, &renderer) < 0)
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer() failed: %s\n",
            SDL_GetError());
        return 1;
    }

    pixels = (pixel_t *)malloc(texture_w * texture_h * sizeof(pixel_t));

    cell_grid_a = (cell_t *)calloc(texture_w * texture_h, sizeof(cell_t));
    cell_grid_b = (cell_t *)calloc(texture_w * texture_h, sizeof(cell_t));

    /* Initialize white */
    memset(pixels, colors[WHITE], texture_w * texture_h * sizeof(pixel_t));

    /* Configure texture */
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING, texture_w, texture_h);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    /* Stretch texture to rect */
    SDL_Rect texture_rect;
    texture_rect.w = window_w;
    texture_rect.h = window_h;
    texture_rect.x = 0;
    texture_rect.y = 0;

    /* Configure renderer */
    SDL_SetRenderTarget(renderer, texture);

    /* Configure window */
    SDL_SetWindowTitle(window, argv[1]);
    SDL_SetWindowSize(window, window_w, window_h);
    if (getenv("SDL_FULLSCREEN") != NULL)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    else
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

#ifdef RPS_INIT_TRI
    perturbate_cell_grid_tri(texture);
#elif RPS_INIT_RAND
    perturbate_cell_grid_rand(texture);
#endif /* RPS_INIT_TRI */

    SDL_bool done = SDL_FALSE;
    while (!done)
    {
        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(SLEEPTIME);
        evaluate_cell_grid(texture);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_q:
                    done = SDL_TRUE;
                    break;
                }
            }
        }
    }

    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
