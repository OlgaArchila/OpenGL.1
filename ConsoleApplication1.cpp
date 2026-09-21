#include <iostream>                                      // Permite usar cout y cerr
#include <vector>                                        // Permite usar std::vector
#include <cstddef>                                       // Permite usar offsetof
#include <cmath>                                         // Permite usar funciones matemáticas
#include <cstdlib>                                       // Permite usar std::abs
#include <glad/glad.h>                                   // Carga las funciones modernas de OpenGL
#include <GLFW/glfw3.h>                                  // Permite crear la ventana y manejar GLFW

struct VertexSquare {                                    // Estructura que representa un vértice de un cuadrito
    GLfloat pos[3];                                      // Posición X, Y y Z del vértice
    GLfloat color[4];                                    // Color RGBA del vértice
};

struct OriginVertex {                                    // Estructura para los vértices de los ejes
    GLfloat pos[3];                                      // Posición X, Y y Z del vértice
    GLfloat color[4];                                    // Color RGBA del vértice
};

static GLuint g_VAO_sq = 0;                              // VAO utilizado para guardar la configuración de la grilla
static GLuint g_VBO_sq = 0;                              // VBO utilizado para guardar los vértices de la grilla
static GLuint g_EBO_sq = 0;                              // EBO utilizado para guardar los índices de la grilla
static GLuint g_VAO_origin = 0;                         // VAO utilizado para los ejes
static GLuint g_VBO_origin = 0;                         // VBO utilizado para los ejes

static std::vector<VertexSquare> g_gridVerts;            // Vector que contiene los vértices de todos los cuadritos
static std::vector<GLuint> g_gridIndices;                // Vector que contiene los índices de todos los cuadritos

static int g_gridCols = 80;                              // Cantidad de columnas de la grilla
static int g_gridRows = 80;                              // Cantidad de filas de la grilla

static GLfloat g_baseColor[4] = {                       // Color base de los cuadritos
    0.2f, 0.2f, 0.2f, 1.0f                             // Rojo, verde, azul y transparencia
};

static GLfloat g_lineColor[4] = {                        // Color de la línea de Bresenham
    0.8f, 0.5f, 1.0f, 1.0f                             // Morado claro en formato RGBA
};

static int g_line_x0 = 10;                               // Coordenada X inicial de la línea
static int g_line_y0 = 10;                               // Coordenada Y inicial de la línea
static int g_line_x1 = 70;                               // Coordenada X final de la línea
static int g_line_y1 = 40;                               // Coordenada Y final de la línea

static GLsizei g_squareIndexCount = 0;                   // Cantidad total de índices que se dibujarán

GLuint CompileShader(GLenum type, const char* source) {  // Función que compila un shader
    GLuint shader = glCreateShader(type);                // Crea un shader de OpenGL
    glShaderSource(shader, 1, &source, nullptr);         // Le entrega el código fuente al shader
    glCompileShader(shader);                              // Compila el shader

    GLint success = 0;                                   // Variable que guardará si la compilación fue correcta
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);  // Consulta el estado de compilación

    if (!success) {                                      // Comprueba si hubo un error
        char infoLog[512];                               // Espacio para guardar el mensaje de error
        glGetShaderInfoLog(shader, 512, nullptr, infoLog); // Obtiene el mensaje de error
        std::cerr << "Error compilando shader:\n" << infoLog << std::endl; // Muestra el error
    }

    return shader;                                       // Devuelve el shader compilado
}

GLuint CreateShaderProgram() {                           // Función que crea el programa de shaders
    const char* vertexShaderSource =                    // Código del vertex shader
        "#version 330 core\n"                             // Indica la versión de GLSL
        "layout(location = 0) in vec3 aPos;\n"            // Recibe la posición del vértice
        "layout(location = 1) in vec4 aColor;\n"          // Recibe el color del vértice
        "out vec4 vertexColor;\n"                         // Envía el color al fragment shader
        "void main()\n"                                   // Comienza la función principal
        "{\n"                                             // Abre el bloque principal
        "    gl_Position = vec4(aPos, 1.0);\n"            // Coloca el vértice en la pantalla
        "    vertexColor = aColor;\n"                     // Envía el color
        "}\n";                                            // Termina el vertex shader

    const char* fragmentShaderSource =                   // Código del fragment shader
        "#version 330 core\n"                             // Indica la versión de GLSL
        "in vec4 vertexColor;\n"                          // Recibe el color del vertex shader
        "out vec4 FragColor;\n"                           // Define el color final del píxel
        "void main()\n"                                   // Comienza la función principal
        "{\n"                                             // Abre el bloque principal
        "    FragColor = vertexColor;\n"                  // Pinta usando el color recibido
        "}\n";                                            // Termina el fragment shader

    GLuint vertexShader = CompileShader(                 // Compila el vertex shader
        GL_VERTEX_SHADER,                                 // Indica que es un vertex shader
        vertexShaderSource                                // Envía su código
    );

    GLuint fragmentShader = CompileShader(               // Compila el fragment shader
        GL_FRAGMENT_SHADER,                               // Indica que es un fragment shader
        fragmentShaderSource                              // Envía su código
    );

    GLuint shaderProgram = glCreateProgram();            // Crea un programa de shaders
    glAttachShader(shaderProgram, vertexShader);          // Añade el vertex shader al programa
    glAttachShader(shaderProgram, fragmentShader);        // Añade el fragment shader al programa
    glLinkProgram(shaderProgram);                         // Une los shaders en un programa

    GLint success = 0;                                   // Variable para comprobar el enlace
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Consulta el estado del programa

    if (!success) {                                      // Comprueba si hubo un error
        char infoLog[512];                               // Espacio para guardar el mensaje
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Obtiene el error
        std::cerr << "Error enlazando shaders:\n" << infoLog << std::endl; // Muestra el error
    }

    glDeleteShader(vertexShader);                        // Elimina el vertex shader temporal
    glDeleteShader(fragmentShader);                      // Elimina el fragment shader temporal

    return shaderProgram;                                // Devuelve el programa terminado
}

static void CreateSquareVAO(                             // Crea el VAO, VBO y EBO de la grilla
    GLuint& VAO,                                         // Referencia al VAO
    GLuint& VBO,                                         // Referencia al VBO
    GLuint& EBO,                                         // Referencia al EBO
    const VertexSquare* verts,                           // Arreglo de vértices
    size_t vertCount,                                    // Cantidad de vértices
    const GLuint* indices,                               // Arreglo de índices
    size_t indexCount)                                   // Cantidad de índices
{
    glGenVertexArrays(1, &VAO);                          // Genera el VAO
    glGenBuffers(1, &VBO);                               // Genera el VBO
    glGenBuffers(1, &EBO);                               // Genera el EBO

    glBindVertexArray(VAO);                              // Activa el VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);                  // Activa el VBO
    glBufferData(                                           // Envía los vértices a la GPU
        GL_ARRAY_BUFFER,                                  // Tipo de buffer
        vertCount * sizeof(VertexSquare),                 // Tamaño de los datos
        verts,                                            // Datos de los vértices
        GL_STATIC_DRAW                                    // Los datos no cambian constantemente
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);           // Activa el EBO
    glBufferData(                                           // Envía los índices a la GPU
        GL_ELEMENT_ARRAY_BUFFER,                           // Tipo de buffer
        indexCount * sizeof(GLuint),                      // Tamaño de los índices
        indices,                                           // Índices
        GL_STATIC_DRAW                                    // Los índices permanecen iguales
    );

    GLsizei stride = static_cast<GLsizei>(sizeof(VertexSquare)); // Distancia entre vértices

    glVertexAttribPointer(                                // Configura el atributo de posición
        0,                                                 // Ubicación 0
        3,                                                 // Tres valores X,Y,Z
        GL_FLOAT,                                          // Los datos son float
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(VertexSquare, pos)               // Ubicación de la posición
    );

    glEnableVertexAttribArray(0);                         // Activa el atributo de posición

    glVertexAttribPointer(                                // Configura el atributo de color
        1,                                                 // Ubicación 1
        4,                                                 // Cuatro valores RGBA
        GL_FLOAT,                                          // Los datos son float
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(VertexSquare, color)             // Ubicación del color
    );

    glEnableVertexAttribArray(1);                         // Activa el atributo de color

    glBindBuffer(GL_ARRAY_BUFFER, 0);                     // Desactiva el VBO
    glBindVertexArray(0);                                 // Desactiva el VAO
}

static void CreateGridSquares(                           // Crea todos los cuadritos de la grilla
    std::vector<VertexSquare>& outVerts,                 // Vector donde se guardan los vértices
    std::vector<GLuint>& outIndices,                     // Vector donde se guardan los índices
    int cols,                                             // Cantidad de columnas
    int rows,                                             // Cantidad de filas
    const GLfloat color[4])                              // Color de los cuadritos
{
    outVerts.clear();                                    // Limpia los vértices anteriores
    outIndices.clear();                                  // Limpia los índices anteriores

    outVerts.reserve(static_cast<size_t>(cols) * rows * 4); // Reserva memoria para los vértices
    outIndices.reserve(static_cast<size_t>(cols) * rows * 6); // Reserva memoria para los índices

    float cellW = 2.0f / static_cast<float>(cols);       // Calcula el ancho de cada cuadrito
    float cellH = 2.0f / static_cast<float>(rows);       // Calcula el alto de cada cuadrito

    for (int y = 0; y < rows; ++y) {                     // Recorre las filas
        for (int x = 0; x < cols; ++x) {                 // Recorre las columnas

            float left = -1.0f + x * cellW;              // Calcula el lado izquierdo
            float right = left + cellW;                  // Calcula el lado derecho
            float bottom = -1.0f + y * cellH;            // Calcula el lado inferior
            float top = bottom + cellH;                  // Calcula el lado superior

            VertexSquare v0 = {                          // Crea el vértice superior derecho
                { right, top, 0.0f },                    // Posición del vértice
                { color[0], color[1], color[2], color[3] } // Color del vértice
            };

            VertexSquare v1 = {                          // Crea el vértice inferior derecho
                { right, bottom, 0.0f },                 // Posición del vértice
                { color[0], color[1], color[2], color[3] } // Color del vértice
            };

            VertexSquare v2 = {                          // Crea el vértice inferior izquierdo
                { left, bottom, 0.0f },                   // Posición del vértice
                { color[0], color[1], color[2], color[3] } // Color del vértice
            };

            VertexSquare v3 = {                          // Crea el vértice superior izquierdo
                { left, top, 0.0f },                     // Posición del vértice
                { color[0], color[1], color[2], color[3] } // Color del vértice
            };

            GLuint base = static_cast<GLuint>(outVerts.size()); // Guarda el índice inicial del cuadrito

            outVerts.push_back(v0);                       // Guarda el vértice 0
            outVerts.push_back(v1);                       // Guarda el vértice 1
            outVerts.push_back(v2);                       // Guarda el vértice 2
            outVerts.push_back(v3);                       // Guarda el vértice 3

            outIndices.push_back(base + 0);               // Primer vértice del primer triángulo
            outIndices.push_back(base + 1);               // Segundo vértice del primer triángulo
            outIndices.push_back(base + 2);               // Tercer vértice del primer triángulo
            outIndices.push_back(base + 0);               // Primer vértice del segundo triángulo
            outIndices.push_back(base + 2);               // Segundo vértice del segundo triángulo
            outIndices.push_back(base + 3);               // Tercer vértice del segundo triángulo
        }
    }
}

static void PaintLineBresenham(                          // Pinta una línea usando Bresenham
    std::vector<VertexSquare>& verts,                    // Vértices de la grilla
    int cols,                                            // Cantidad de columnas
    int rows,                                            // Cantidad de filas
    int x0,                                               // Coordenada X inicial
    int y0,                                               // Coordenada Y inicial
    int x1,                                               // Coordenada X final
    int y1,                                               // Coordenada Y final
    const GLfloat color[4])                              // Color de la línea
{
    auto setCellColor = [&](int cx, int cy) {            // Función para pintar una celda
        if (cx < 0 || cx >= cols || cy < 0 || cy >= rows) return; // Evita salir de la grilla

        size_t cellIndex = static_cast<size_t>(cy) * cols + static_cast<size_t>(cx); // Calcula la celda
        size_t base = cellIndex * 4;                     // Cada celda tiene cuatro vértices

        for (size_t i = 0; i < 4; ++i) {                 // Recorre los cuatro vértices
            verts[base + i].color[0] = color[0];          // Cambia rojo
            verts[base + i].color[1] = color[1];          // Cambia verde
            verts[base + i].color[2] = color[2];          // Cambia azul
            verts[base + i].color[3] = color[3];          // Cambia transparencia
        }
        };

    int dx = std::abs(x1 - x0);                           // Calcula la diferencia horizontal
    int sx = x0 < x1 ? 1 : -1;                            // Determina hacia dónde avanza X
    int dy = -std::abs(y1 - y0);                          // Calcula la diferencia vertical negativa
    int sy = y0 < y1 ? 1 : -1;                            // Determina hacia dónde avanza Y
    int err = dx + dy;                                    // Calcula el error inicial

    while (true) {                                        // Repite hasta llegar al punto final
        setCellColor(x0, y0);                             // Pinta el cuadrito actual

        if (x0 == x1 && y0 == y1) break;                 // Termina cuando llega al final

        int e2 = 2 * err;                                 // Calcula el doble del error

        if (e2 >= dy) {                                   // Comprueba si debe avanzar en X
            err += dy;                                    // Actualiza el error
            x0 += sx;                                     // Avanza X
        }

        if (e2 <= dx) {                                   // Comprueba si debe avanzar en Y
            err += dx;                                    // Actualiza el error
            y0 += sy;                                     // Avanza Y
        }
    }
}

static void CreateOriginVAO()                             // Crea los ejes X e Y
{
    OriginVertex originVertices[] = {                    // Define los vértices de los ejes

        { { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Inicio del eje X
        { {  1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Final del eje X

        { { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // Inicio del eje Y
        { { 0.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }  // Final del eje Y
    };

    glGenVertexArrays(1, &g_VAO_origin);                 // Genera el VAO de los ejes
    glGenBuffers(1, &g_VBO_origin);                      // Genera el VBO de los ejes

    glBindVertexArray(g_VAO_origin);                     // Activa el VAO de los ejes
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO_origin);          // Activa el VBO de los ejes

    glBufferData(                                           // Envía los ejes a la GPU
        GL_ARRAY_BUFFER,                                  // Tipo de buffer
        sizeof(originVertices),                            // Tamaño de los datos
        originVertices,                                    // Datos de los ejes
        GL_STATIC_DRAW                                    // Los datos no cambian
    );

    GLsizei stride = static_cast<GLsizei>(sizeof(OriginVertex)); // Distancia entre vértices

    glVertexAttribPointer(                                // Configura la posición
        0,                                                 // Ubicación 0
        3,                                                 // Tres valores X,Y,Z
        GL_FLOAT,                                          // Tipo float
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(OriginVertex, pos)               // Posición dentro de la estructura
    );

    glEnableVertexAttribArray(0);                         // Activa la posición

    glVertexAttribPointer(                                // Configura el color
        1,                                                 // Ubicación 1
        4,                                                 // Cuatro valores RGBA
        GL_FLOAT,                                          // Tipo float
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(OriginVertex, color)             // Color dentro de la estructura
    );

    glEnableVertexAttribArray(1);                         // Activa el color

    glBindBuffer(GL_ARRAY_BUFFER, 0);                     // Desactiva el VBO
    glBindVertexArray(0);                                 // Desactiva el VAO
}

int main() {                                               // Función principal del programa

    if (!glfwInit()) {                                      // Inicializa GLFW
        std::cerr << "Error al iniciar GLFW" << std::endl; // Muestra el error
        return -1;                                         // Termina el programa
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);         // Solicita OpenGL 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);         // Solicita OpenGL 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa el perfil moderno

    GLFWwindow* window = glfwCreateWindow(                // Crea la ventana
        800,                                                // Ancho
        600,                                                // Alto
        "Grilla Bresenham",                                // Título
        nullptr,                                            // Sin pantalla completa
        nullptr                                             // Sin ventana compartida
    );

    if (window == nullptr) {                                // Comprueba si se creó
        std::cerr << "Error creando la ventana" << std::endl; // Muestra el error
        glfwTerminate();                                    // Cierra GLFW
        return -1;                                          // Termina el programa
    }

    glfwMakeContextCurrent(window);                         // Hace actual el contexto OpenGL

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { // Carga las funciones de OpenGL
        std::cerr << "Error cargando GLAD" << std::endl;    // Muestra el error
        glfwDestroyWindow(window);                          // Destruye la ventana
        glfwTerminate();                                    // Cierra GLFW
        return -1;                                          // Termina el programa
    }

    glViewport(0, 0, 800, 600);                            // Define el área de dibujo

    GLuint shaderProgram = CreateShaderProgram();           // Crea nuestro programa de shaders

    CreateGridSquares(                                      // Construye la grilla
        g_gridVerts,                                        // Vector de vértices
        g_gridIndices,                                      // Vector de índices
        g_gridCols,                                         // Número de columnas
        g_gridRows,                                         // Número de filas
        g_baseColor                                         // Color base
    );

    PaintLineBresenham(                                     // Aplica Bresenham sobre la grilla
        g_gridVerts,                                        // Vértices de la grilla
        g_gridCols,                                         // Columnas
        g_gridRows,                                         // Filas
        g_line_x0,                                          // X inicial
        g_line_y0,                                          // Y inicial
        g_line_x1,                                          // X final
        g_line_y1,                                          // Y final
        g_lineColor                                         // Color de la línea
    );

    g_squareIndexCount = static_cast<GLsizei>(g_gridIndices.size()); // Guarda la cantidad de índices

    CreateSquareVAO(                                        // Crea el VAO de la grilla
        g_VAO_sq,                                           // VAO
        g_VBO_sq,                                           // VBO
        g_EBO_sq,                                           // EBO
        g_gridVerts.data(),                                 // Vértices
        g_gridVerts.size(),                                 // Cantidad de vértices
        g_gridIndices.data(),                               // Índices
        g_gridIndices.size()                                // Cantidad de índices
    );

    CreateOriginVAO();                                     // Crea los ejes X e Y

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);                 // Define el color del fondo

    while (!glfwWindowShouldClose(window)) {                // Comienza el ciclo de renderizado

        glClear(GL_COLOR_BUFFER_BIT);                        // Limpia la pantalla

        glUseProgram(shaderProgram);                        // Activa los shaders

        glBindVertexArray(g_VAO_sq);                        // Activa la grilla

        glDrawElements(                                      // Dibuja todos los cuadritos
            GL_TRIANGLES,                                   // Dibuja triángulos
            g_squareIndexCount,                             // Cantidad de índices
            GL_UNSIGNED_INT,                                // Tipo de índice
            nullptr                                         // Comienza desde el índice 0
        );

        glBindVertexArray(g_VAO_origin);                    // Activa los ejes

        glLineWidth(2.0f);                                   // Hace los ejes un poco más gruesos

        glDrawArrays(                                        // Dibuja los ejes
            GL_LINES,                                        // Dibuja líneas
            0,                                               // Primer vértice
            4                                                // Cuatro vértices
        );

        glfwSwapBuffers(window);                             // Muestra el resultado en pantalla

        glfwPollEvents();                                    // Procesa eventos de la ventana
    }

    glDeleteVertexArrays(1, &g_VAO_sq);                     // Elimina el VAO de la grilla
    glDeleteBuffers(1, &g_VBO_sq);                          // Elimina el VBO de la grilla
    glDeleteBuffers(1, &g_EBO_sq);                          // Elimina el EBO de la grilla

    glDeleteVertexArrays(1, &g_VAO_origin);                 // Elimina el VAO de los ejes
    glDeleteBuffers(1, &g_VBO_origin);                      // Elimina el VBO de los ejes

    glDeleteProgram(shaderProgram);                         // Elimina el programa de shaders

    glfwDestroyWindow(window);                              // Destruye la ventana
    glfwTerminate();                                        // Cierra GLFW

    return 0;                                                // Indica que terminó correctamente
}