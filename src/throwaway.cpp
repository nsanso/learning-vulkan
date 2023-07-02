#include <SDL.h>

int sample_window(int argc, char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  // SDL_Surface *surface;
  SDL_Event event;

  int err;

  err = SDL_Init(SDL_INIT_VIDEO);
  if (err < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return err;
  }

  err = SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_RESIZABLE, &window,
                                    &renderer);
  if (err < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't create window and render: %s", SDL_GetError());
    return err;
  }

  while (true) {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      goto QUIT;
    }

    SDL_SetRenderDrawColor(renderer, 0x2f, 0x0f, 0x4f, 0x6f);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

QUIT:
  SDL_Log("Closing the application...");
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
