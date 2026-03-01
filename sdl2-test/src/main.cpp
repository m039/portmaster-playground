#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>

#include <iostream>
#include <vector>
#include <string>
#include <GLES3/gl3.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

GLuint compileShaders(const char *vshader, const char *fshader);

class Label {
    private:
    TTF_Font* _font;

    SDL_Surface* _textSurface;

    SDL_Renderer* _renderer;

    GLuint _textureId = 0;

    float _x;
    
    float _y;

    float _width;

    float _height;

    float _scale = 1.0;

    GLuint _vao, _vbo, _ebo;

    GLuint _shader;

    inline static const float textQuadVertices[] = {
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f
    };

    inline static const unsigned int textQuadIndices[] = {0, 1, 2, 2, 3, 0};

    inline static const char * vshader = R"(#version 300 es
    precision mediump float;
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }

    )";

    inline static const char * fshader = R"(#version 300 es
    precision mediump float;
    out vec4 FragColor;
    in vec2 TexCoord;

    uniform sampler2D textTexture;
    uniform vec4 textColor;

    void main() {
        FragColor = texture(textTexture, TexCoord) * textColor;
    }
    )";

    public:
    bool init(SDL_Renderer *renderer) {
        _renderer = renderer;

        _font = TTF_OpenFont("assets/arial.ttf", 12);
        if (_font == NULL) {
            SDL_Log("Can't open font: %s", SDL_GetError());
            SDL_Quit();
            return false;
        }

        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ebo);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(textQuadIndices), textQuadIndices, GL_STATIC_DRAW);

        _shader = compileShaders(vshader, fshader);

        return true;
    }

    void setScale(const float scale) {
        _scale = scale;
    }

    void setPosition(const float x, const float y) {
        _x = x;
        _y = y;
    }

    void setText(const std::string &s) {
        SDL_Color foregroundColor = { 255, 255, 255, 255};

        SDL_Surface *tmp = TTF_RenderText_Blended(_font, s.c_str(), foregroundColor);
        if (!tmp)
        {
            SDL_Log("Can't create text surface: %s", SDL_GetError());
            return;
        }

        GLenum mode = GL_RGBA;
        SDL_PixelFormat * target = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
        SDL_Surface * textSurface = SDL_ConvertSurface(tmp, target, 0);
        SDL_FreeSurface(tmp);
        SDL_FreeFormat(target);

        _width = textSurface->w;
        _height = textSurface->h;

        if (!_textureId) {
            glGenTextures(1, &_textureId);
        }
        glBindTexture(GL_TEXTURE_2D, _textureId);

        // Загрузка данных текстуры
        glTexImage2D(GL_TEXTURE_2D, 0, mode, textSurface->w, textSurface->h, 0,
                    mode, GL_UNSIGNED_BYTE, textSurface->pixels);

        // Настройки фильтрации
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        SDL_FreeSurface(textSurface);
    }

    void render() {
        if (!_textureId) {
            return;
        }

        float x = _x;
        float y = _y;
        float width = _width * _scale;
        float height = _height * _scale;

        glDisable(GL_DEPTH_TEST); // Отключаем тест глубины для 2D-элементов
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Активируем шейдер текста
        glUseProgram(_shader);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

        // Позиции вершин
        GLuint aPosLoc = glGetAttribLocation(_shader, "aPos");
        glEnableVertexAttribArray(aPosLoc);
        glVertexAttribPointer(aPosLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // Текстурные координаты
        GLuint aTexCoordLoc = glGetAttribLocation(_shader, "aTexCoord");
        glEnableVertexAttribArray(aTexCoordLoc);
        glVertexAttribPointer(aTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glm::mat4 viewMatrix = glm::mat4(1.0);
        GLint viewLoc = glGetUniformLocation(_shader, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        // Устанавливаем ортогональную проекцию для 2D
        glm::mat4 textProjection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
        GLint textProjLoc = glGetUniformLocation(_shader, "projection");
        glUniformMatrix4fv(textProjLoc, 1, GL_FALSE, glm::value_ptr(textProjection));

        // Позиция текста (верхний левый угол + отступ)
        glm::mat4 textScale = glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0));
        glm::mat4 textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 0.0f)) * textScale;
        GLint textModelLoc = glGetUniformLocation(_shader, "model");
        glUniformMatrix4fv(textModelLoc, 1, GL_FALSE, glm::value_ptr(textModel));

        // Цвет текста (белый)
        GLint colorLoc = glGetUniformLocation(_shader, "textColor");
        glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f); // RGBA

        // Активируем текстуру текста
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        GLint texLoc = glGetUniformLocation(_shader, "textTexture");
        glUniform1i(texLoc, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(aPosLoc);
        glDisableVertexAttribArray(aTexCoordLoc);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void destroy() {
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        glDeleteProgram(_shader);
        glDeleteTextures(1, &_textureId);
    }
};

Label label;

class Planet {
    struct MeshData {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        int indexCount;
        int indexComponentType;
        int vertexComponentType;
    };

    struct Model {
        std::vector<MeshData> meshes;
        // Здесь можно добавить материалы, текстуры и т. д.
    };

    Model _model;

    GLuint _shader;

    glm::vec3 _rotation;

    public:
    bool init() {
        // Загрузка модели
        if (!loadGLBModel("assets/planet.glb", _model)) {
            std::cerr << "Failed to load model" << std::endl;
            return false;
        }

        // Компиляция шейдеров
        _shader = compileShaders(vertexShaderSource, fragmentShaderSource);
        if (_shader == 0) {
            std::cerr << "Failed to compile shaders" << std::endl;
            return false;
        }

        return true;
    }

    void setXRotation(float value) {
        _rotation.x = -180 * value;
    }

    void setYRotation(float value) {
        _rotation.z = -180 * value;
    }

    void render() {
        float aspect = (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT;

        // Включение глубины для 3D-рендеринга
        glEnable(GL_DEPTH_TEST);

        // Настройка матриц
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::rotate(modelMatrix, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 20.0f, 0.0f),  // камера
            glm::vec3(0.0f, 0.0f, 0.0f),  // цель
            glm::vec3(1.0f, 0.0f, 0.0f)   // вверх
        );

        float size = 20.0;

        glm::mat4 projection = glm::ortho(-size / 2.0, +size / 2.0, -(size / aspect) / 2.0, +(size / aspect) / 2.0, -200.0, +200.0);

        // Передача матриц в шейдер
        glUseProgram(_shader);

        GLint modelLoc = glGetUniformLocation(_shader, "model");
        GLint viewLoc = glGetUniformLocation(_shader, "view");
        GLint projLoc = glGetUniformLocation(_shader, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Рендеринг модели
        for (const auto& mesh : _model.meshes) {
            GLuint aPosLoc = glGetAttribLocation(_shader, "aPos");

            if (mesh.vbo) {
                glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
                glVertexAttribPointer(aPosLoc, 3, mesh.vertexComponentType, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(aPosLoc);
            }

            if (mesh.ebo) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
                glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexComponentType, 0);
            } else {
                // Если нет индексов, используем glDrawArrays
            }

            glDisableVertexAttribArray(aPosLoc);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    void destroy() {
        for (const auto& mesh : _model.meshes) {
            glDeleteBuffers(1, &mesh.ebo);
            glDeleteBuffers(1, &mesh.vbo);
        }
        glDeleteProgram(_shader);
    }
private:

    void loadVertexBuffer(const tinygltf::Model& model, int accessorIdx, GLuint& vbo, int& vertexComponentType) {
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

        vertexComponentType = accessor.componentType;

        // Настройка атрибутов вершин (пример для позиций)
        GLuint index = glGetAttribLocation(_shader, "aPos");
        glVertexAttribPointer(index, 3, vertexComponentType, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(index);

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

                // Загрузка вершинных данных
                if (primitive.attributes.count("POSITION") > 0) {
                    int posAccessor = primitive.attributes.at("POSITION");
                    loadVertexBuffer(gltfModel, posAccessor, meshData.vbo, meshData.vertexComponentType);
                } else {
                    std::cerr << "No position in model" << std::endl;
                }

                // Загрузка индексных данных (если есть)
                if (primitive.indices >= 0) {
                    loadIndexBuffer(gltfModel, primitive.indices, meshData.ebo, meshData.indexCount, meshData.indexComponentType);
                } else {
                    std::cerr << "No indeces in model" << std::endl;
                }

                model.meshes.push_back(meshData);
            }
        }

        return true;
    }
    // Простой вершинный шейдер
    const char* vertexShaderSource = R"(#version 300 es
    precision mediump float;
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    )";

    // Простой фрагментный шейдер
    const char* fragmentShaderSource = R"(#version 300 es
    precision mediump float;

    out vec4 FragColor;

    void main() {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    )";

};

Planet planet;

GLuint compileShaders(const char *vshader, const char *fshader) {
    // Компиляция вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vshader, nullptr);
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
    glShaderSource(fragmentShader, 1, &fshader, nullptr);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

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

    printf(
        "Program info:\n" 
        " driver-name: %s\n"
        " driver-flags: %x\n"
        " video-driver: %s\n",
        info.name,
        info.flags,
        SDL_GetCurrentVideoDriver()
    );

    if (TTF_Init() == -1) {
        SDL_Log("Can't init ttf library: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int screenWidth = WINDOW_WIDTH;
    int screenHeight = WINDOW_HEIGHT;
    float aspect = (float) screenWidth / screenHeight;

    SDL_Window* window = SDL_CreateWindow("SDL2 Test", 0, 0, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Unable to init renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Создание контекста OpenGL
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_Log("Unable to create context: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    if (!planet.init()) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!label.init(renderer)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    label.setPosition(10, 10);
    label.setScale(2.0);
    label.setText("Press a key");

    SDL_Event event;
    SDL_GameController *gamepad = NULL;
    Uint32 lastTime = SDL_GetTicks();

    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_CONTROLLERDEVICEADDED) {
                gamepad = SDL_GameControllerOpen(event.cdevice.which);
            } else if (event.type == SDL_KEYDOWN) {
                label.setText("Key pressed: " + std::string(SDL_GetKeyName(event.key.keysym.sym)));
             } else if (event.type == SDL_CONTROLLERAXISMOTION) {
                const Sint16 MAX_STICK_VALUE = 32768;

                if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    planet.setXRotation(static_cast<float>(SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX)) / MAX_STICK_VALUE);
                }

                if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    planet.setYRotation(static_cast<float>(SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY)) / MAX_STICK_VALUE);
                }
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

        planet.render();
        label.render();

        // Обмен буферами
        SDL_GL_SwapWindow(window);

        // Ограничение FPS (60 FPS)
        SDL_Delay(16);
    }

    // Очистка ресурсов
    label.destroy();
    planet.destroy();
    SDL_GameControllerClose(gamepad);
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
