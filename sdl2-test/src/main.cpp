#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_ttf.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>

#include <iostream>
#include <vector>
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

struct MeshData {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int indexCount;
    int indexComponentType;
};

struct Model {
    std::vector<MeshData> meshes;
    // Здесь можно добавить материалы, текстуры и т. д.
};

void loadVertexBuffer(const tinygltf::Model& model, int accessorIdx, GLuint& vbo) {
    const auto& accessor = model.accessors[accessorIdx];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Копируем данные из GLTF в OpenGL-буфер
    glBufferData(
        GL_ARRAY_BUFFER,
        bufferView.byteLength,
        buffer.data.data() + bufferView.byteOffset,
        GL_STATIC_DRAW
    );

    // Настройка атрибутов вершин (пример для позиций)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, accessor.componentType, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void loadIndexBuffer(const tinygltf::Model& model, int accessorIdx, GLuint& ebo, int& indexCount, int& indexComponentType) {
    const auto& accessor = model.accessors[accessorIdx];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        bufferView.byteLength,
        buffer.data.data() + bufferView.byteOffset,
        GL_STATIC_DRAW
    );

    indexCount = accessor.count;
    indexComponentType = accessor.componentType;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


bool loadGLBModel(const std::string& filename, Model& model) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Загрузка GLB-файла
    bool success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);

    if (!warn.empty()) {
        std::cout << "GLTF Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "GLTF Error: " << err << std::endl;
    }
    if (!success) {
        std::cerr << "Failed to load GLB file: " << filename << std::endl;
        return false;
    }

    // Обработка всех сеток (meshes) в модели
    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            MeshData meshData;

            // Создание VAO
            glGenVertexArrays(1, &meshData.vao);
            glBindVertexArray(meshData.vao);

            // Загрузка вершинных данных
            if (primitive.attributes.count("POSITION") > 0) {
                int posAccessor = primitive.attributes.at("POSITION");
                loadVertexBuffer(gltfModel, posAccessor, meshData.vbo);
            } else {
                std::cerr << "No position in model" << std::endl;
            }

            // Загрузка индексных данных (если есть)
            if (primitive.indices >= 0) {
                loadIndexBuffer(gltfModel, primitive.indices, meshData.ebo, meshData.indexCount, meshData.indexComponentType);
            } else {
                std::cerr << "No indeces in model" << std::endl;
            }

            glBindVertexArray(0);
            model.meshes.push_back(meshData);
        }
    }

    return true;
}
// Простой вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Простой фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
)";

GLuint compileShaders() {
    // Компиляция вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Проверка ошибок компиляции
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Компиляция фрагментного шейдера
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание шейдерной программы
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("SDL2 Test", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Создание контекста OpenGL
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Инициализация GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Проверка поддержки OpenGL 3.3
    if (!GLEW_VERSION_3_3) {
        std::cerr << "OpenGL 3.3 not supported!" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Включение глубины для 3D-рендеринга
    glEnable(GL_DEPTH_TEST);

    // Загрузка модели
    Model model;
    if (!loadGLBModel("assets/planet.glb", model)) {
        std::cerr << "Failed to load model" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Компиляция шейдеров
    GLuint shaderProgram = compileShaders();
    if (shaderProgram == 0) {
        std::cerr << "Failed to compile shaders" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    label.setPosition(10, 10);
    label.setScale(1.0);
    label.setText("Press a key");

    SDL_Event event;
    SDL_GameController *gamepad = NULL;
    float angle = 0.0f;
    Uint32 lastTime = SDL_GetTicks();

    int running = 1;
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // в секундах
        lastTime = currentTime;

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
            running = 0;
        }       

        // Очистка буферов
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Обновление угла вращения
        angle += 45.0f * deltaTime; // 45 градусов в секунду

        // Настройка матриц
        glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f),
                                          glm::radians(angle),
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 20.0f, 0.0f),  // камера
            glm::vec3(0.0f, 0.0f, 0.0f),  // цель
            glm::vec3(1.0f, 0.0f, 0.0f)   // вверх
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f
        );

        // glm::mat4 projection = glm::ortho(-10, +10, -10, +10, -10, +10);

        // Передача матриц в шейдер
        glUseProgram(shaderProgram);

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Рендеринг модели
        for (const auto& mesh : model.meshes) {
            glBindVertexArray(mesh.vao);
            if (mesh.vbo) {
                glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
            }

            if (mesh.ebo) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
                glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexComponentType, 0);
            } else {
                // Если нет индексов, используем glDrawArrays
            }
            glBindVertexArray(0);
        }

        // Обмен буферами
        SDL_GL_SwapWindow(window);

        // Ограничение FPS (60 FPS)
        SDL_Delay(16);
    }

    // Очистка ресурсов
    label.destroy();
    SDL_GameControllerClose(gamepad);
    for (const auto& mesh : model.meshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.ebo);
    }
    glDeleteProgram(shaderProgram);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
