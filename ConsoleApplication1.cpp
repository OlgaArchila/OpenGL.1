#include <iostream>                                      // Permite usar cout y cerr
#include <vector>                                        // Permite usar std::vector
#include <cstddef>                                       // Permite usar offsetof
#include <cstdlib>                                       // Permite usar std::abs
#include <glad/glad.h>                                   // Carga las funciones modernas de OpenGL
#include <GLFW/glfw3.h>                                  // Permite crear la ventana y manejar GLFW

struct VertexSquare {                                    // Estructura que representa un vértice de un cuadrito
    GLfloat pos[3];                                      // Guarda la posición X, Y y Z
    GLfloat color[4];                                    // Guarda el color RGBA
};

static GLuint g_VAO_sq = 0;                              // VAO de la grilla
static GLuint g_VBO_sq = 0;                              // VBO de la grilla
static GLuint g_EBO_sq = 0;                              // EBO de la grilla

static std::vector<VertexSquare> g_gridVerts;            // Guarda los vértices de todos los cuadritos
static std::vector<GLuint> g_gridIndices;                // Guarda los índices de todos los cuadritos

static int g_gridCols = 80;                              // Cantidad de columnas de la grilla
static int g_gridRows = 80;                              // Cantidad de filas de la grilla

static GLfloat g_gridColor[4] = {                       // Color de la grilla
    0.55f, 0.35f, 0.75f, 1.0f                          // Morado claro
};

static GLfloat g_lineColor[4] = {                       // Color de los cuadritos seleccionados por Bresenham
    1.0f, 0.75f, 0.2f, 1.0f                           // Amarillo para diferenciarlos
};

static int g_line_x0 = 10;                               // Coordenada X inicial de la línea
static int g_line_y0 = 10;                               // Coordenada Y inicial de la línea
static int g_line_x1 = 70;                               // Coordenada X final de la línea
static int g_line_y1 = 40;                               // Coordenada Y final de la línea

static GLsizei g_squareIndexCount = 0;                   // Cantidad total de índices de la grilla


GLuint CompileShader(GLenum type, const char* source) {  // Compila un shader
    GLuint shader = glCreateShader(type);                // Crea el shader
    glShaderSource(shader, 1, &source, nullptr);         // Envía el código del shader
    glCompileShader(shader);                              // Compila el shader

    GLint success = 0;                                   // Guarda el resultado de la compilación
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);  // Consulta si compiló correctamente

    if (!success) {                                      // Comprueba si hubo un error
        char infoLog[512];                               // Guarda el mensaje del error
        glGetShaderInfoLog(shader, 512, nullptr, infoLog); // Obtiene el mensaje del error
        std::cerr << "Error compilando shader:\n" << infoLog << std::endl; // Muestra el error
    }

    return shader;                                       // Devuelve el shader compilado
}


GLuint CreateShaderProgram() {                           // Crea el programa de shaders

    const char* vertexShaderSource =                    // Código del vertex shader
        "#version 330 core\n"                             // Indica la versión de GLSL
        "layout(location = 0) in vec3 aPos;\n"            // Recibe la posición del vértice
        "layout(location = 1) in vec4 aColor;\n"          // Recibe el color del vértice
        "out vec4 vertexColor;\n"                         // Envía el color al fragment shader
        "void main()\n"                                   // Función principal
        "{\n"                                             // Abre la función
        "    gl_Position = vec4(aPos, 1.0);\n"            // Coloca el vértice en la pantalla
        "    vertexColor = aColor;\n"                     // Envía el color
        "}\n";                                            // Cierra el shader

    const char* fragmentShaderSource =                   // Código del fragment shader
        "#version 330 core\n"                             // Indica la versión de GLSL
        "in vec4 vertexColor;\n"                          // Recibe el color
        "out vec4 FragColor;\n"                           // Define el color final
        "void main()\n"                                   // Función principal
        "{\n"                                             // Abre la función
        "    FragColor = vertexColor;\n"                  // Pinta el fragmento con el color recibido
        "}\n";                                            // Cierra el shader

    GLuint vertexShader = CompileShader(                 // Compila el vertex shader
        GL_VERTEX_SHADER,                                 // Indica que es un vertex shader
        vertexShaderSource                                // Código del vertex shader
    );

    GLuint fragmentShader = CompileShader(               // Compila el fragment shader
        GL_FRAGMENT_SHADER,                               // Indica que es un fragment shader
        fragmentShaderSource                              // Código del fragment shader
    );

    GLuint shaderProgram = glCreateProgram();            // Crea el programa de shaders

    glAttachShader(shaderProgram, vertexShader);          // Añade el vertex shader
    glAttachShader(shaderProgram, fragmentShader);        // Añade el fragment shader

    glLinkProgram(shaderProgram);                         // Une los dos shaders

    GLint success = 0;                                   // Guarda el resultado del enlace
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Consulta si el programa quedó bien

    if (!success) {                                      // Comprueba si hubo un error
        char infoLog[512];                               // Guarda el mensaje del error
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Obtiene el error
        std::cerr << "Error enlazando shaders:\n" << infoLog << std::endl; // Muestra el error
    }

    glDeleteShader(vertexShader);                        // Elimina el vertex shader temporal
    glDeleteShader(fragmentShader);                      // Elimina el fragment shader temporal

    return shaderProgram;                                // Devuelve el programa completo
}


static void CreateSquareVAO(                            // Crea el VAO, VBO y EBO
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
        GL_STATIC_DRAW                                    // Los datos no cambian
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);           // Activa el EBO

    glBufferData(                                           // Envía los índices a la GPU
        GL_ELEMENT_ARRAY_BUFFER,                           // Tipo de buffer
        indexCount * sizeof(GLuint),                      // Tamaño de los índices
        indices,                                           // Datos de los índices
        GL_STATIC_DRAW                                    // Los índices no cambian
    );

    GLsizei stride = static_cast<GLsizei>(sizeof(VertexSquare)); // Distancia entre vértices

    glVertexAttribPointer(                                // Configura la posición
        0,                                                 // Ubicación 0
        3,                                                 // X, Y y Z
        GL_FLOAT,                                          // Tipo de dato
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(VertexSquare, pos)               // Ubicación de la posición
    );

    glEnableVertexAttribArray(0);                         // Activa la posición

    glVertexAttribPointer(                                // Configura el color
        1,                                                 // Ubicación 1
        4,                                                 // R, G, B y A
        GL_FLOAT,                                          // Tipo de dato
        GL_FALSE,                                          // No normalizar
        stride,                                            // Distancia entre vértices
        (GLvoid*)offsetof(VertexSquare, color)             // Ubicación del color
    );

    glEnableVertexAttribArray(1);                         // Activa el color

    glBindBuffer(GL_ARRAY_BUFFER, 0);                     // Desactiva el VBO
    glBindVertexArray(0);                                 // Desactiva el VAO
}


static void CreateGridSquares(                           // Crea la grilla de cuadritos
    std::vector<VertexSquare>& outVerts,                 // Vector de vértices
    std::vector<GLuint>& outIndices,                     // Vector de índices
    int cols,                                             // Número de columnas
    int rows,                                             // Número de filas
    const GLfloat color[4])                              // Color de la grilla
{
    outVerts.clear();                                    // Limpia los vértices anteriores
    outIndices.clear();                                  // Limpia los índices anteriores

    outVerts.reserve(static_cast<size_t>(cols) * rows * 4); // Reserva memoria para los vértices
    outIndices.reserve(static_cast<size_t>(cols) * rows * 6); // Reserva memoria para los índices

    float cellW = 2.0f / static_cast<float>(cols);       // Calcula el ancho de cada cuadrito
    float cellH = 2.0f / static_cast<float>(rows);       // Calcula el alto de cada cuadrito

    for (int y = 0; y < rows; ++y) {                     // Recorre todas las filas
        for (int x = 0; x < cols; ++x) {                 // Recorre todas las columnas

            float left = -1.0f + x * cellW;              // Calcula el lado izquierdo
            float right = left + cellW;                  // Calcula el lado derecho
            float bottom = -1.0f + y * cellH;            // Calcula el lado inferior
            float top = bottom + cellH;                  // Calcula el lado superior

            VertexSquare v0 = {                          // Crea el vértice superior derecho
                { right, top, 0.0f },                    // Posición
                { color[0], color[1], color[2], color[3] } // Color
            };

            VertexSquare v1 = {                          // Crea el vértice inferior derecho
                { right, bottom, 0.0f },                 // Posición
                { color[0], color[1], color[2], color[3] } // Color
            };

            VertexSquare v2 = {                          // Crea el vértice inferior izquierdo
                { left, bottom, 0.0f },                   // Posición
                { color[0], color[1], color[2], color[3] } // Color
            };

            VertexSquare v3 = {                          // Crea el vértice superior izquierdo
                { left, top, 0.0f },                     // Posición
                { color[0], color[1], color[2], color[3] } // Color
            };

            GLuint base = static_cast<GLuint>(outVerts.size()); // Guarda el índice inicial

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


static void PaintLineBresenham(                          // Pinta los cuadritos usando Bresenham
    std::vector<VertexSquare>& verts,                    // Vértices de la grilla
    int cols,                                            // Número de columnas
    int rows,                                            // Número de filas
    int x0,                                               // X inicial
    int y0,                                               // Y inicial
    int x1,                                               // X final
    int y1,                                               // Y final
    const GLfloat color[4])                              // Color de la línea
{
    auto setCellColor = [&](int cx, int cy) {            // Función que cambia el color de una celda

        if (cx < 0 || cx >= cols || cy < 0 || cy >= rows) return; // Evita salir de la grilla

        size_t cellIndex = static_cast<size_t>(cy) * cols + static_cast<size_t>(cx); // Calcula la celda

        size_t base = cellIndex * 4;                     // Cada celda tiene cuatro vértices

        for (size_t i = 0; i < 4; ++i) {                 // Recorre los cuatro vértices

            verts[base + i].color[0] = color[0];          // Cambia el componente rojo
            verts[base + i].color[1] = color[1];          // Cambia el componente verde
            verts[base + i].color[2] = color[2];          // Cambia el componente azul
            verts[base + i].color[3] = color[3];          // Cambia la transparencia
        }
        };

    int dx = std::abs(x1 - x0);                           // Calcula la distancia horizontal

    int sx = x0 < x1 ? 1 : -1;                            // Define la dirección de X

    int dy = -std::abs(y1 - y0);                          // Calcula la distancia vertical

    int sy = y0 < y1 ? 1 : -1;                            // Define la dirección de Y

    int err = dx + dy;                                    // Calcula el error inicial

    while (true) {                                        // Repite hasta llegar al punto final

        setCellColor(x0, y0);                             // Pinta el cuadrito actual

        if (x0 == x1 && y0 == y1) break;                 // Termina cuando llega al punto final

        int e2 = 2 * err;                                 // Calcula el doble del error

        if (e2 >= dy) {                                   // Comprueba si debe avanzar en X

            err += dy;                                    // Actualiza el error

            x0 += sx;                                     // Avanza en X
        }

        if (e2 <= dx) {                                   // Comprueba si debe avanzar en Y

            err += dx;                                    // Actualiza el error

            y0 += sy;                                     // Avanza en Y
        }
    }
}


int main() {                                               // Función principal del programa

    if (!glfwInit()) {                                      // Inicializa GLFW

        std::cerr << "Error al iniciar GLFW" << std::endl; // Muestra el error

        return -1;                                         // Termina el programa
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);         // Solicita OpenGL versión 3

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);         // Solicita OpenGL versión 3.3

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa OpenGL Core Profile

    GLFWwindow* window = glfwCreateWindow(                // Crea la ventana

        800,                                                // Ancho de la ventana

        600,                                                // Alto de la ventana

        "Grilla Bresenham",                                // Nombre de la ventana

        nullptr,                                            // No usa pantalla completa

        nullptr                                             // No comparte contexto
    );

    if (window == nullptr) {                                // Comprueba si la ventana se creó

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

    GLuint shaderProgram = CreateShaderProgram();           // Crea el programa de shaders

    CreateGridSquares(                                      // Crea la grilla

        g_gridVerts,                                        // Vector de vértices

        g_gridIndices,                                      // Vector de índices

        g_gridCols,                                         // Número de columnas

        g_gridRows,                                         // Número de filas

        g_gridColor                                         // Color de la grilla
    );

    PaintLineBresenham(                                     // Aplica el algoritmo de Bresenham

        g_gridVerts,                                        // Vértices de la grilla

        g_gridCols,                                         // Número de columnas

        g_gridRows,                                         // Número de filas

        g_line_x0,                                          // X inicial

        g_line_y0,                                          // Y inicial

        g_line_x1,                                          // X final

        g_line_y1,                                          // Y final

        g_lineColor                                         // Color de la línea
    );

    g_squareIndexCount = static_cast<GLsizei>(g_gridIndices.size()); // Guarda el número de índices

    CreateSquareVAO(                                        // Crea los buffers de la grilla

        g_VAO_sq,                                           // VAO

        g_VBO_sq,                                           // VBO

        g_EBO_sq,                                           // EBO

        g_gridVerts.data(),                                 // Datos de vértices

        g_gridVerts.size(),                                 // Cantidad de vértices

        g_gridIndices.data(),                               // Datos de índices

        g_gridIndices.size()                                // Cantidad de índices
    );

    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);              // Define el color del fondo

    while (!glfwWindowShouldClose(window)) {                // Comienza el ciclo de renderizado

        glClear(GL_COLOR_BUFFER_BIT);                        // Limpia la pantalla

        glUseProgram(shaderProgram);                        // Activa el programa de shaders

        glBindVertexArray(g_VAO_sq);                        // Activa la grilla

        glDrawElements(                                      // Dibuja la grilla

            GL_TRIANGLES,                                   // Dibuja usando triángulos

            g_squareIndexCount,                             // Número de índices

            GL_UNSIGNED_INT,                                // Tipo de índice

            nullptr                                         // Empieza desde el primer índice
        );

        glfwSwapBuffers(window);                             // Muestra el resultado

        glfwPollEvents();                                    // Procesa eventos de la ventana
    }

    glDeleteVertexArrays(1, &g_VAO_sq);                     // Elimina el VAO

    glDeleteBuffers(1, &g_VBO_sq);                          // Elimina el VBO

    glDeleteBuffers(1, &g_EBO_sq);                          // Elimina el EBO

    glDeleteProgram(shaderProgram);                         // Elimina el programa de shaders

    glfwDestroyWindow(window);                              // Destruye la ventana

    glfwTerminate();                                        // Cierra GLFW

    return 0;                                                // Termina correctamente
}
