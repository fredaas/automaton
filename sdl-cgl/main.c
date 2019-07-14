#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <SDL.h>

#define FPS 30
#define SLEEPTIME (1000 / FPS)

typedef uint32_t pixel_t;

typedef struct {
    int state;
} cell_t;

enum {
    DEAD,
    ALIVE,
    NUM_STATES
};

pixel_t colors[NUM_STATES] = {
    [DEAD]   = 0xffffffff,
    [ALIVE]  = 0x404040ff
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

#define set_cell_state(x, y, s) \
do {                            \
    grid_a(x, y).state = s;     \
    pixel(x, y) = colors[s];    \
} while (0)

void swap(cell_t **a, cell_t **b)
{
    cell_t *c = *a;
    *a = *b;
    *b = c;
}

void seed_cell_grid(SDL_Texture *texture)
{
    int dx = texture_w / 4;
    int dy = texture_h / 4;
    int xbound[2] = { dx, texture_w - dx };
    int ybound[2] = { dy, texture_h - dy };

    for (int y = ybound[0]; y < ybound[1]; y++)
        for (int x = xbound[0]; x < xbound[1]; x++)
            set_cell_state(x, y, ALIVE);

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

/* Transitions cell state base on the classic CGoL rules. */
void transition(cell_t *cell, int count)
{
    int new_state = -1;

    switch (cell->state)
    {
    case ALIVE:
        if (count < 2 || count > 3)
            new_state = DEAD;
        break;
    case DEAD:
        if (count == 3)
            new_state = ALIVE;
        break;
    }

    if (new_state != -1)
        cell->state = new_state;
}

cell_t next_cell(int x, int y)
{
    /* Alive neighbors */
    int count = 0;

    cell_t next = grid_a(x, y);

    /* No borders */
    if ((x > 0)
        && (y > 0)
        && (x < texture_w - 1)
        && (y < texture_h - 1))
    {
        count += grid_a(x - 1, y - 1).state;
        count += grid_a(x, y - 1).state;
        count += grid_a(x + 1, y - 1).state;
        count += grid_a(x + 1, y).state;
        count += grid_a(x + 1, y + 1).state;
        count += grid_a(x, y + 1).state;
        count += grid_a(x - 1, y).state;
        count += grid_a(x - 1, y + 1).state;

        transition(&next, count);

        return next;
    }

    /* West border */
    if (x == 0)
        count += grid_a(x + 1, y).state;
    /* East border */
    if (x == texture_w - 1)
        count += grid_a(x - 1, y).state;
    /* North border */
    if (y == 0)
        count += grid_a(x, y + 1).state;
    /* South border */
    if (y == texture_h - 1)
        count += grid_a(x, y - 1).state;
    /* North-west border */
    if ((x == 0) && (y == 0))
        count += grid_a(x + 1, y + 1).state;
    /* South-west border */
    if ((x == 0) && (y == texture_h - 1))
        count += grid_a(x + 1, y - 1).state;
    /* North-east border */
    if ((x == texture_w - 1) && (y == 0))
        count += grid_a(x - 1, y + 1).state;
    /* South-east border */
    if ((x == texture_w - 1) && (y == texture_h - 1))
        count += grid_a(x - 1, y - 1).state;

    transition(&next, count);

    return next;
}

void evaluate_cell_grid(SDL_Texture *texture)
{
    for (int y = 0; y < texture_h; y++)
        for (int x = 0; x < texture_w; x++)
        {
            cell_t next = next_cell(x, y);
            grid_b(x, y) = next;
            pixel(x, y) = colors[next.state];
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
    memset(pixels, colors[DEAD], texture_w * texture_h * sizeof(pixel_t));

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

    seed_cell_grid(texture);

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
