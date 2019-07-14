#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <SDL.h>

#define FPS 120
#define SLEEPTIME (1000 / FPS)

#define SCALE 1 / 2
#define WIDTH 800
#define HEIGHT 600

typedef uint32_t pixel_t;

#define BLACK 0x000000ff
#define WHITE 0xffffffff

char *cells_a = NULL;
char *cells_b = NULL;

pixel_t *pixels = NULL;

char *ruleset = NULL;

#define cell_a(x) cells_a[1 + x]
#define cell_b(x) cells_b[1 + x]

int window_w  = WIDTH,
    window_h  = HEIGHT;

/* SCALE must divide window dimensions */
int texture_w = WIDTH * SCALE,
    texture_h = HEIGHT * SCALE;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

#define pixel(x, y) pixels[(texture_w * (y)) + (x)]

#define ctoi(c) (int)((c) - '0')

void swap(char **a, char **b)
{
    char *c = *a;
    *a = *b;
    *b = c;
}

char eval_stencil(char *s)
{
    int i = ctoi(s[0]) * (int)pow(2, 2)
        + ctoi(s[1]) * (int)pow(2, 1) + ctoi(s[2]);
    return ruleset[7 - i];
}

void rule_combo(char *a, char *b, int n, int i, int k)
{
    static int j = 0;
    if (i == n)
    {
        if (j == k)
            strcpy(b, a);
        j++;
        return;
    }
    a[i] = '0';
    rule_combo(a, b, n, i + 1, k);
    a[i] = '1';
    rule_combo(a, b, n, i + 1, k);
}

char * get_ruleset(int k)
{
    const int n = 8;
    char a[n + 1];
    char *b = (char *)malloc((n + 1) * sizeof(char));
    rule_combo(a, b, n, 0, k);
    return b;
}

void init(SDL_Texture *texture)
{
    int x = texture_w / 2;
    cell_a(x) = '1';
    pixel(x, 0) = BLACK;
    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

void iterate(SDL_Texture *texture)
{
    static int y = 1;

    if (y > texture_h - 1)
        y = 0;

    for (int x = 0; x < texture_w; x++)
    {
        char s[3];
        s[0] = cell_a(x - 1);
        s[1] = cell_a(x);
        s[2] = cell_a(x + 1);
        char r = eval_stencil(s);
        cell_b(x) = r;
        pixel(x, y) = (r == '1' ? BLACK : WHITE);
    }

    y++;

    swap(&cells_b, &cells_a);

    memset(cells_b, '0', (texture_w + 2) * sizeof(char));

    SDL_UpdateTexture(texture, NULL, pixels, texture_w * sizeof(pixel_t));
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc - 1 == 0)
        ruleset = get_ruleset(150);
    else
        ruleset = get_ruleset(atoi(argv[1]));

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

    cells_a = (char *)calloc((texture_w + 2), sizeof(char));
    cells_b = (char *)calloc((texture_w + 2), sizeof(char));
    memset(cells_a, '0', (texture_w + 2) * sizeof(char));
    memset(cells_b, '0', (texture_w + 2) * sizeof(char));

    pixels = (pixel_t *)malloc(texture_w * texture_h * sizeof(pixel_t));
    memset(pixels, 0xffffffff, texture_w * texture_h * sizeof(pixel_t));

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
    SDL_SetWindowTitle(window, "Cellular Automaton");
    SDL_SetWindowSize(window, window_w, window_h);
    if (getenv("SDL_FULLSCREEN") != NULL)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    else
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    init(texture);

    SDL_bool done = SDL_FALSE;
    while (!done)
    {
        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(SLEEPTIME);
        iterate(texture);

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
