#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <SDL.h>

#define FPS 120
#define SLEEPTIME (1000 / FPS)

typedef struct {
    int state;
} cell_t;

typedef struct {
    int x;
    int y;
    int d;
} ant_t;

enum { LEFT, RIGHT };

enum { BLACK, WHITE, RED, NUM_STATES };

#define N 0
#define E 90
#define S 180
#define W 270

typedef uint32_t pixel_t;

pixel_t colors[NUM_STATES] = {
    [WHITE] = 0xffffffff,
    [BLACK] = 0x404040ff,
    [RED]   = 0xff4040ff
};

cell_t *cell_grid = NULL;

pixel_t *pixels = NULL;

ant_t *ant = NULL;

#define SCALE 4

#define WIDTH 800
#define HEIGHT 600

int window_w  = WIDTH,
    window_h  = HEIGHT;

/* Divisors must divide the window dimensions */
int texture_w = WIDTH / SCALE,
    texture_h = HEIGHT / SCALE;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

#define grid(x, y) cell_grid[(texture_w * (y)) + (x)]
#define pixel(x, y) pixels[(texture_w * (y)) + (x)]

/* Rotate within range [0, 360) */
void rotate(ant_t *ant, int motion)
{
    switch (motion)
    {
    case LEFT:
        ant->d -= 90;
        if (ant->d < 0)
            ant->d = 360 + ant->d;
        break;
    case RIGHT:
        ant->d += 90;
        if (ant->d >= 360)
            ant->d = 0;
        break;
    }
}

/* Move one cell in current direction, wrapping world edges */
void move(ant_t *ant)
{
    switch (ant->d)
    {
    case N:
        ant->y -= 1;
        if (ant->y < 0)
            ant->y = texture_h - 1;
        break;
    case E:
        ant->x += 1;
        if (ant->x >= texture_w)
            ant->x = 0;
        break;
    case S:
        ant->y += 1;
        if (ant->y >= texture_h)
            ant->y = 0;
        break;
    case W:
        ant->x -= 1;
        if (ant->x < 0)
            ant->x = texture_w - 1;
        break;
    }
}

void init(ant_t *ant, int d, SDL_Texture *texture)
{
    ant->x = texture_w / 2;
    ant->y = texture_h / 2;
    ant->d = d;
    grid(ant->x, ant->y).state = BLACK;
    pixel(ant->x, ant->y) = colors[RED];

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

void iterate(ant_t *ant, SDL_Texture *texture)
{
    cell_t *curr_cell = &grid(ant->x, ant->y);
    pixel(ant->x, ant->y) = colors[curr_cell->state];

    move(ant);
    pixel(ant->x, ant->y) = colors[RED];

    cell_t *next_cell = &grid(ant->x, ant->y);

    switch (next_cell->state)
    {
    case WHITE:
        next_cell->state = BLACK;
        rotate(ant, RIGHT);
        break;
    case BLACK:
        next_cell->state = WHITE;
        rotate(ant, LEFT);
        break;
    }

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
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

    cell_grid = (cell_t *)calloc(texture_w * texture_h, sizeof(cell_t));

    pixels = (pixel_t *)malloc(texture_w * texture_h * sizeof(pixel_t));

    ant = (ant_t *)malloc(sizeof(ant_t));

    /* Initialize cell states */
    for (int x = 0; x < texture_w; x++)
        for (int y = 0; y < texture_h; y++)
            grid(x, y).state = WHITE;
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
#ifdef RPS_FULLSCREEN
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
#else
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);
#endif /* RSP_FULLSCREEN */
    SDL_ShowWindow(window);

    init(ant, N, texture);

    SDL_bool done = SDL_FALSE;
    while (!done)
    {
        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(SLEEPTIME);
        iterate(ant, texture);

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
