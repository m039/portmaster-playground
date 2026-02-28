#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

class Label {
    private:
    TTF_Font* _font;

    SDL_Surface* _textSurface;

    SDL_Renderer* _renderer;

    SDL_Texture* _texture;

    SDL_FRect _position;

    float _width;

    float _height;

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
        SDL_Color backgroundColor = { 0, 0, 255 };

        SDL_Surface *textSurface = TTF_RenderText_Shaded(_font, s.c_str(), s.length(), foregroundColor, backgroundColor);

        _texture = SDL_CreateTextureFromSurface(_renderer, textSurface);

        SDL_DestroySurface(textSurface);

        SDL_GetTextureSize(_texture, &_width, &_height);
    }

    void render() {
        if (_texture == NULL) {
            return;
        }

        _position.w = _width * _scale;
        _position.h = _height * _scale;

        SDL_RenderTexture(_renderer, _texture, NULL, &_position);
    }

    void destroy() {
        SDL_DestroyTexture(_texture);
    }
};

Label label;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!TTF_Init()) {
        SDL_Log("Can't init ttf library: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("SDL3 Test", 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
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
    SDL_Gamepad *gamepad = NULL;

    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                gamepad = SDL_OpenGamepad(event.gdevice.which);
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                label.setText("Key pressed: " + std::string(SDL_GetKeyName(event.key.key)));
            } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                label.setText("Gamepad's button down: " + std::string(SDL_GetGamepadStringForButton((SDL_GamepadButton)event.gbutton.button)));
            }
        }

        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK) &&
            SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START)) {
            goto exit;
        }       

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        label.render();
        SDL_RenderPresent(renderer);
    }

    exit:
    label.destroy();
    SDL_CloseGamepad(gamepad);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
