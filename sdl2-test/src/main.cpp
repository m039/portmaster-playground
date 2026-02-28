#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_ttf.h>

#include <string>

class Label {
    private:
    TTF_Font* _font;

    SDL_Surface* _textSurface;

    SDL_Renderer* _renderer;

    SDL_Texture* _texture;

    SDL_Rect _position;

    int _width;

    int _height;

    float _scale = 1.0;

    public:
    bool init(SDL_Renderer *renderer) {
        _renderer = renderer;

        _font = TTF_OpenFont("assets/arial.ttf", 12);
        if (_font == NULL) {
            SDL_Log("Can't open font: %s", SDL_GetError());
            SDL_Quit();
            return false;
        }

        return true;
    }

    void setScale(const float scale) {
        _scale = scale;
    }

    void setPosition(const float x, const float y) {
        _position.x = x;
        _position.y = y;
    }

    void setText(const std::string &s) {
        SDL_DestroyTexture(_texture);

        SDL_Color foregroundColor = { 0, 0, 0 };
        SDL_Color backgroundColor = { 255, 255, 255 };

        SDL_Surface *textSurface = TTF_RenderText_Shaded(_font, s.c_str(), foregroundColor, backgroundColor);

        _texture = SDL_CreateTextureFromSurface(_renderer, textSurface);

        SDL_FreeSurface(textSurface);

        SDL_QueryTexture(_texture, NULL, NULL, &_width, &_height);
    }

    void render() {
        if (_texture == NULL) {
            return;
        }

        _position.w = _width * _scale;
        _position.h = _height * _scale;

        SDL_RenderCopy(_renderer, _texture, NULL, &_position);
    }

    void destroy() {
        SDL_DestroyTexture(_texture);
    }
};

Label label;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_RendererInfo info;

    if (SDL_GetRenderDriverInfo(0, &info)) {
        SDL_Log("Can't get driver info: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Current video driver: %s\n", SDL_GetCurrentVideoDriver());

    printf(
        "Driver Info:\n" 
        " name: %s\n"
        " flags: %x\n",
        info.name,
        info.flags
    );

    if (TTF_Init() == -1) {
        SDL_Log("Can't init ttf library: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("SDL2 Test", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!label.init(renderer)) {
        return 1;
    }

    label.setPosition(10, 10);
    label.setScale(1.0);
    label.setText("Press a key");

    SDL_Event event;
    SDL_GameController *gamepad = NULL;

    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_CONTROLLERDEVICEADDED) {
                gamepad = SDL_GameControllerOpen(event.cdevice.which);
            } else if (event.type == SDL_KEYDOWN) {
                label.setText("Key pressed: " + std::string(SDL_GetKeyName(event.key.keysym.sym)));
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                int buttonCode = event.cbutton.button; 
                const char* buttonName = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button);
                label.setText("Gamepad's button down: " + std::string(buttonName));
            }
        }

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_BACK) &&
            SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_START)) {
            goto exit;
        }       

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        label.render();
        SDL_RenderPresent(renderer);
    }

    exit:
    label.destroy();
    SDL_GameControllerClose(gamepad);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
