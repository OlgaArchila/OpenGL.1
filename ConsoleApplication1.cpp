// ================================================================
// MI PAINT - OPENGL + GLFW + GLAD + DEAR IMGUI OLGA ARCHILA
// ================================================================
// 1. LIBRERIAS
// ================================================================

#include <iostream>                 // Permite mostrar mensajes y errores en consola.
#include <vector>                   // Permite guardar celdas, vertices y estados anteriores.
#include <array>                    // Permite almacenar los cuatro componentes de un color.
#include <cmath>                    // Permite utilizar funciones matematicas como sin, cos y round.
#include <algorithm>                // Permite utilizar min, max y otras operaciones.
#include <cstddef>                  // Permite utilizar offsetof para configurar los atributos.
#include <glad/glad.h>              // Carga las funciones modernas de OpenGL.
#include <GLFW/glfw3.h>             // Crea la ventana y recibe los eventos del mouse y teclado.
#include "imgui.h"                  // Contiene las funciones principales de Dear ImGui.
#include "imgui_impl_glfw.h"        // Conecta Dear ImGui con GLFW.
#include "imgui_impl_opengl3.h"     // Conecta Dear ImGui con OpenGL.


// ================================================================
// 2. ESTRUCTURAS DE DATOS
// ================================================================

struct VertexSquare {               // Representa un vertice de una celda del lienzo.
    GLfloat pos[3];                 // Guarda las coordenadas X, Y y Z del vertice.
    GLfloat color[4];               // Guarda el color RGBA del vertice.
};

struct Punto {                      // Representa una posicion dentro de la grilla.
    int x;                          // Guarda la columna del punto.
    int y;                          // Guarda la fila del punto.
};

using Color = std::array<float, 4>; // Define un tipo de dato para guardar colores RGBA.


// ================================================================
// 3. VARIABLES GLOBALES DE OPENGL
// ================================================================

static GLuint g_VAO = 0;            // Identificador del Vertex Array Object.
static GLuint g_VBO = 0;            // Identificador del Vertex Buffer Object.
static GLuint g_EBO = 0;            // Identificador del Element Buffer Object.
static GLuint g_shaderProgram = 0;  // Identificador del programa de shaders.
static GLsizei g_indexCount = 0;    // Cantidad de indices que se dibujaran.



// ================================================================
// 4. CONFIGURACION DEL LIENZO
// ================================================================

static const int g_cols = 80;       // Define la cantidad de columnas del lienzo.
static const int g_rows = 80;       // Define la cantidad de filas del lienzo.

static Color g_background = {       // Define el color inicial del fondo.
    0.95f, 0.95f, 0.95f, 1.0f      // Color gris muy claro.
};

static Color g_drawColor = {        // Define el color inicial del lapiz.
    0.10f, 0.35f, 0.85f, 1.0f     // Color azul.
};

static std::vector<Color> g_cells;  // Guarda el color actual de cada celda.
static std::vector<VertexSquare> g_vertices; // Guarda los cuatro vertices de cada celda.
static std::vector<GLuint> g_indices;         // Guarda los indices de los triangulos.


// ================================================================
// 5. HERRAMIENTAS DEL PROGRAMA
// ================================================================

enum Tool {                         // Enumera las herramientas disponibles.
    TOOL_PENCIL,                    // Dibujo libre siguiendo el mouse.
    TOOL_LINE,                      // Linea recta con Bresenham.
    TOOL_RECTANGLE,                 // Rectangulo.
    TOOL_SQUARE,                    // Cuadrado.
    TOOL_TRIANGLE,                  // Triangulo.
    TOOL_CIRCLE,                    // Circulo con Bresenham.
    TOOL_FILL,                      // Relleno por inundacion.
    TOOL_ERASER                     // Borrador.
};

static Tool g_tool = TOOL_PENCIL;   // Selecciona el lapiz al iniciar.

static const char* g_toolNames[] = { // Nombres que aparecen en la interfaz.
    "Lapiz",                        // Herramienta 0: dibujo libre.
    "Linea",                        // Herramienta 1: linea recta.
    "Rectangulo",                   // Herramienta 2: rectangulo.
    "Cuadrado",                     // Herramienta 3: cuadrado.
    "Triangulo",                    // Herramienta 4: triangulo.
    "Circulo",                      // Herramienta 5: circulo.
    "Rellenar (balde)",             // Herramienta 6: relleno.
    "Borrador"                      // Herramienta 7: borrador.
};

static bool g_dragging = false;     // Indica si el usuario mantiene presionado el mouse.
static Punto g_start = { 0, 0 };    // Guarda el punto inicial de una figura.
static Punto g_current = { 0, 0 };  // Guarda la posicion actual del mouse.
static Punto g_last = { -1, -1 };  // Evita pint    
static std::vector<Punto> g_polyPoints; // Guarda los puntos de una forma libre.
static std::vector<Punto> g_lastFigure; // Guarda los puntos de la ultima figura para rotarla.

static std::vector<std::vector<Color>> g_undo; // Guarda copias anteriores del lienzo.
static const size_t g_maxUndo = 30;            // Limita la cantidad de estados guardados.


// ================================================================
// 6. FUNCIONES PARA COLORES Y CELDAS
// ================================================================

bool SameColor(const Color& a, const Color& b) { // Comprueba si dos colores son iguales.
    for (int i = 0; i < 4; ++i) {                // Recorre rojo, verde, azul y transparencia.
        if (std::fabs(a[i] - b[i]) > 0.001f)     // Compara cada componente con una tolerancia.
            return false;                        // Si uno cambia, los colores son diferentes.
    }
    return true;                                 // Si todos coinciden, los colores son iguales.
}

int CellIndex(int x, int y) {                    // Convierte una coordenada 2D en una posicion del vector.
    return y * g_cols + x;                       // Multiplica la fila por columnas y suma la columna.
}

bool InsideGrid(int x, int y) {                  // Comprueba que una coordenada este dentro del lienzo.
    return x >= 0 && x < g_cols && y >= 0 && y < g_rows; // Devuelve verdadero si esta dentro.
}

void SaveUndo() {                                // Guarda el estado actual para poder deshacer.
    g_undo.push_back(g_cells);                   // Copia todos los colores del lienzo.
    if (g_undo.size() > g_maxUndo)               // Comprueba si se supero el limite de estados.
        g_undo.erase(g_undo.begin());             // Elimina el estado mas antiguo.
}

void Undo() {                                    // Recupera el estado anterior del lienzo.
    if (g_undo.empty())                          // Comprueba si hay algun estado guardado.
        return;                                  // Si no hay estados, no hace nada.
    g_cells = g_undo.back();                     // Recupera la ultima copia guardada.
    g_undo.pop_back();                           // Elimina esa copia de la pila.
}

void SetCell(int x, int y, const Color& color) { // Cambia el color de una celda.
    if (!InsideGrid(x, y))                       // Verifica que la celda exista.
        return;                                  // Evita acceder fuera del vector.
    g_cells[CellIndex(x, y)] = color;            // Guarda el nuevo color en la celda.
}

Color GetCell(int x, int y) {                   // Devuelve el color de una celda.
    if (!InsideGrid(x, y))                       // Verifica que la coordenada sea valida.
        return g_background;                    // Devuelve el color de fondo si esta fuera.
    return g_cells[CellIndex(x, y)];            // Devuelve el color guardado.
}

// ================================================================
// 7. COMPILACION DE SHADERS
// ================================================================

GLuint CompileShader(GLenum type, const char* source) { // Compila un shader individual.
    GLuint shader = glCreateShader(type);               // Crea el shader indicado.
    glShaderSource(shader, 1, &source, nullptr);        // Envia el codigo fuente al shader.
    glCompileShader(shader);                            // Compila el codigo fuente.

    GLint success = 0;                                  // Guarda el resultado de la compilacion.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Consulta si la compilacion fue correcta.

    if (!success) {                                     // Entra si ocurrio un error.
        char infoLog[1024];                             // Reserva espacio para el mensaje.
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog); // Recupera la descripcion del error.
        std::cerr << "Error compilando shader:\n" << infoLog << std::endl; // Muestra el error.
    }
    return shader;                                      // Devuelve el identificador del shader.
}

GLuint CreateShaderProgram() {                         // Crea y enlaza los dos shaders.
    const char* vertexSource =                          // Codigo del vertex shader.
        "#version 330 core\n"                            // Solicita GLSL 3.30.
        "layout(location=0) in vec3 aPos;\n"             // Recibe la posicion del vertice.
        "layout(location=1) in vec4 aColor;\n"           // Recibe el color del vertice.
        "out vec4 vertexColor;\n"                       // Envia el color al fragment shader.
        "void main(){\n"                                 // Inicia la funcion principal.
        "gl_Position=vec4(aPos,1.0);\n"                  // Ubica el vertice en pantalla.
        "vertexColor=aColor;\n"                          // Pasa el color al siguiente shader.
        "}\n";                                           // Termina el vertex shader.

    const char* fragmentSource =                        // Codigo del fragment shader.
        "#version 330 core\n"                            // Solicita GLSL 3.30.
        "in vec4 vertexColor;\n"                         // Recibe el color interpolado.
        "out vec4 FragColor;\n"                          // Declara el color final del pixel.
        "void main(){\n"                                 // Inicia la funcion principal.
        "FragColor=vertexColor;\n"                       // Pinta con el color recibido.
        "}\n";                                           // Termina el fragment shader.

    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource); // Compila el vertex shader.
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource); // Compila el fragment shader.
    GLuint program = glCreateProgram();                 // Crea el programa que une los shaders.

    glAttachShader(program, vertex);                    // Adjunta el vertex shader.
    glAttachShader(program, fragment);                  // Adjunta el fragment shader.
    glLinkProgram(program);                             // Enlaza los shaders en un programa.

    GLint success = 0;                                  // Guarda el resultado del enlace.
    glGetProgramiv(program, GL_LINK_STATUS, &success);  // Consulta si el enlace fue correcto.

    if (!success) {                                     // Entra si el programa no pudo enlazarse.
        char infoLog[1024];                             // Reserva espacio para el mensaje.
        glGetProgramInfoLog(program, 1024, nullptr, infoLog); // Obtiene el error.
        std::cerr << "Error enlazando shaders:\n" << infoLog << std::endl; // Imprime el error.
    }

    glDeleteShader(vertex);                             // Elimina el vertex shader temporal.
    glDeleteShader(fragment);                           // Elimina el fragment shader temporal.
    return program;                                     // Devuelve el programa enlazado.
}



// ================================================================
// 8. CREACION Y ACTUALIZACION DE LA GRILLA
// ================================================================

void BuildGrid() {                                     // Construye la geometria inicial del lienzo.
    g_vertices.clear();                                // Limpia los vertices anteriores.
    g_indices.clear();                                 // Limpia los indices anteriores.
    g_vertices.reserve(g_cols * g_rows * 4);            // Reserva cuatro vertices por celda.
    g_indices.reserve(g_cols * g_rows * 6);            // Reserva seis indices por celda.

    for (int y = 0; y < g_rows; ++y) {                 // Recorre las filas.
        for (int x = 0; x < g_cols; ++x) {             // Recorre las columnas.
            float gap = 0.0025f;                          // Define un espacio pequeño para que se vea la cuadricula.
            float left = -1.0f + 2.0f * x / g_cols + gap; // Calcula el borde izquierdo con separacion.
            float right = -1.0f + 2.0f * (x + 1) / g_cols - gap; // Calcula el borde derecho con separacion.
            float bottom = -1.0f + 2.0f * y / g_rows + gap; // Calcula el borde inferior con separacion.
            float top = -1.0f + 2.0f * (y + 1) / g_rows - gap; // Calcula el borde superior con separacion.

            Color c = GetCell(x, y);                  // Obtiene el color actual de la celda.
            VertexSquare v0 = { { right, top, 0.0f },{ c[0], c[1], c[2], c[3] } }; // Vertice superior derecho.
            VertexSquare v1 = { { right, bottom, 0.0f },{ c[0], c[1], c[2], c[3] } }; // Vertice inferior derecho.
            VertexSquare v2 = { { left, bottom, 0.0f },{ c[0], c[1], c[2], c[3] } }; // Vertice inferior izquierdo.
            VertexSquare v3 = { { left, top, 0.0f },{ c[0], c[1], c[2], c[3] } }; // Vertice superior izquierdo.

            GLuint base = static_cast<GLuint>(g_vertices.size()); // Obtiene el indice del primer vertice.
            g_vertices.push_back(v0);                  // Agrega el primer vertice.
            g_vertices.push_back(v1);                  // Agrega el segundo vertice.
            g_vertices.push_back(v2);                  // Agrega el tercer vertice.
            g_vertices.push_back(v3);                  // Agrega el cuarto vertice.

            g_indices.push_back(base + 0);             // Primer vertice del triangulo uno.
            g_indices.push_back(base + 1);             // Segundo vertice del triangulo uno.
            g_indices.push_back(base + 2);             // Tercer vertice del triangulo uno.
            g_indices.push_back(base + 0);             // Primer vertice del triangulo dos.
            g_indices.push_back(base + 2);             // Segundo vertice del triangulo dos.
            g_indices.push_back(base + 3);             // Tercer vertice del triangulo dos.
        }
    }
    g_indexCount = static_cast<GLsizei>(g_indices.size()); // Guarda la cantidad total de indices.
}

void UpdateVertexColors() {                            // Copia los colores de las celdas a los vertices.
    for (int y = 0; y < g_rows; ++y) {                 // Recorre cada fila.
        for (int x = 0; x < g_cols; ++x) {             // Recorre cada columna.
            Color c = GetCell(x, y);                  // Lee el color de la celda.
            size_t base = static_cast<size_t>(CellIndex(x, y)) * 4; // Calcula el primer vertice.
            for (int k = 0; k < 4; ++k) {             // Recorre los cuatro vertices.
                g_vertices[base + k].color[0] = c[0]; // Actualiza el rojo.
                g_vertices[base + k].color[1] = c[1]; // Actualiza el verde.
                g_vertices[base + k].color[2] = c[2]; // Actualiza el azul.
                g_vertices[base + k].color[3] = c[3]; // Actualiza la transparencia.
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);             // Activa el VBO del lienzo.
    glBufferSubData(GL_ARRAY_BUFFER, 0, g_vertices.size() * sizeof(VertexSquare), g_vertices.data()); // Actualiza sus datos.
    glBindBuffer(GL_ARRAY_BUFFER, 0);                 // Desactiva el VBO.
}

void CreateGridBuffers() {                             // Crea los buffers que utiliza OpenGL.
    glGenVertexArrays(1, &g_VAO);                      // Genera el VAO.
    glGenBuffers(1, &g_VBO);                           // Genera el VBO.
    glGenBuffers(1, &g_EBO);                           // Genera el EBO.

    glBindVertexArray(g_VAO);                          // Activa el VAO.
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);              // Activa el VBO.
    glBufferData(GL_ARRAY_BUFFER, g_vertices.size() * sizeof(VertexSquare), g_vertices.data(), GL_DYNAMIC_DRAW); // Reserva datos que cambiaran.

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);      // Activa el EBO.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_indices.size() * sizeof(GLuint), g_indices.data(), GL_STATIC_DRAW); // Guarda los indices.

    GLsizei stride = sizeof(VertexSquare);             // Calcula la distancia entre vertices.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexSquare, pos)); // Configura la posicion.
    glEnableVertexAttribArray(0);                      // Habilita el atributo de posicion.
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexSquare, color)); // Configura el color.
    glEnableVertexAttribArray(1);                      // Habilita el atributo de color.

    glBindVertexArray(0);                              // Desactiva el VAO.
    glBindBuffer(GL_ARRAY_BUFFER, 0);                  // Desactiva el VBO.
}

// ================================================================
// 9. LINEA DE BRESENHAM
// ================================================================

void DrawLineBresenham(int x0, int y0, int x1, int y1, const Color& color, std::vector<Punto>* output = nullptr) {
    int dx = std::abs(x1 - x0);                        // Calcula la distancia horizontal.
    int dy = std::abs(y1 - y0);                        // Calcula la distancia vertical.
    int sx = (x0 < x1) ? 1 : -1;                       // Define si X aumenta o disminuye.
    int sy = (y0 < y1) ? 1 : -1;                       // Define si Y aumenta o disminuye.
    int error = dx - dy;                               // Calcula el error inicial.

    while (true) {                                     // Repite hasta alcanzar el punto final.
        SetCell(x0, y0, color);                        // Pinta la celda actual.
        if (output) output->push_back({ x0, y0 });     // Guarda el punto si se solicito una lista.
        if (x0 == x1 && y0 == y1)                      // Comprueba si llego al final.
            break;                                     // Termina el algoritmo.

        int twiceError = 2 * error;                    // Duplica el error para decidir el avance.
        if (twiceError > -dy) {                        // Comprueba si debe avanzar horizontalmente.
            error -= dy;                               // Actualiza el error.
            x0 += sx;                                  // Avanza una columna.
        }
        if (twiceError < dx) {                         // Comprueba si debe avanzar verticalmente.
            error += dx;                               // Actualiza el error.
            y0 += sy;                                  // Avanza una fila.
        }
    }
}


// ================================================================
// 10. CIRCULO DE BRESENHAM
// ================================================================

void DrawCircleBresenham(int cx, int cy, int radius, const Color& color, std::vector<Punto>* output = nullptr) {
    if (radius < 0)                                    // Comprueba que el radio sea valido.
        return;                                        // No dibuja si el radio es negativo.

    int x = 0;                                         // Inicia la coordenada horizontal.
    int y = radius;                                    // Inicia la coordenada vertical en el radio.
    int decision = 1 - radius;                         // Calcula el parametro de decision.

    auto plot = [&](int px, int py) {                  // Define una funcion para pintar un punto.
        SetCell(px, py, color);                        // Pinta la celda correspondiente.
        if (output) output->push_back({ px, py });     // Guarda el punto si se pidio.
        };

    while (x <= y) {                                   // Recorre un octavo del circulo.
        plot(cx + x, cy + y);                          // Punto del primer octante.
        plot(cx - x, cy + y);                          // Punto simetrico.
        plot(cx + x, cy - y);                          // Punto simetrico.
        plot(cx - x, cy - y);                          // Punto simetrico.
        plot(cx + y, cy + x);                          // Punto del octante contiguo.
        plot(cx - y, cy + x);                          // Punto simetrico.
        plot(cx + y, cy - x);                          // Punto simetrico.
        plot(cx - y, cy - x);                          // Punto simetrico.

        if (decision < 0)                              // Comprueba si se conserva Y.
            decision += 2 * x + 3;                     // Actualiza el error.
        else {                                         // Si hay que reducir Y...
            decision += 2 * (x - y) + 5;               // Actualiza el error.
            --y;                                       // Baja una unidad en Y.
        }
        ++x;                                           // Avanza una unidad en X.
    }
}


// ================================================================
// 11. FORMAS GEOMETRICAS
// ================================================================

void DrawRectangle(Punto a, Punto b, const Color& color, std::vector<Punto>* output = nullptr) {
    DrawLineBresenham(a.x, a.y, b.x, a.y, color, output); // Dibuja el lado superior.
    DrawLineBresenham(b.x, a.y, b.x, b.y, color, output); // Dibuja el lado derecho.
    DrawLineBresenham(b.x, b.y, a.x, b.y, color, output); // Dibuja el lado inferior.
    DrawLineBresenham(a.x, b.y, a.x, a.y, color, output); // Dibuja el lado izquierdo.
}

void DrawSquare(Punto a, Punto b, const Color& color, std::vector<Punto>* output = nullptr) {
    int side = std::max(std::abs(b.x - a.x), std::abs(b.y - a.y)); // Toma el lado mayor.
    int endX = a.x + ((b.x >= a.x) ? side : -side); // Calcula la columna final respetando la direccion.
    int endY = a.y + ((b.y >= a.y) ? side : -side); // Calcula la fila final respetando la direccion.
    DrawRectangle(a, { endX, endY }, color, output); // Dibuja el cuadrado como un rectangulo de lados iguales.
}

void DrawTriangle(Punto a, Punto b, const Color& color, std::vector<Punto>* output = nullptr) {
    Punto top = { (a.x + b.x) / 2, std::max(a.y, b.y) }; // Calcula el vertice superior.
    DrawLineBresenham(a.x, a.y, b.x, a.y, color, output); // Dibuja la base.
    DrawLineBresenham(a.x, a.y, top.x, top.y, color, output); // Dibuja el lado izquierdo.
    DrawLineBresenham(top.x, top.y, b.x, a.y, color, output); // Dibuja el lado derecho.
}


// ================================================================
// 12. DIBUJO LIBRE Y RELLENO
// ================================================================

void DrawFreehand(Punto from, Punto to, const Color& color) { // Une dos posiciones consecutivas del mouse.
    DrawLineBresenham(from.x, from.y, to.x, to.y, color);    // Evita dejar espacios entre celdas.
}

void FloodFill(int startX, int startY, const Color& newColor) { // Rellena una region conectada.
    if (!InsideGrid(startX, startY))                         // Comprueba la semilla.
        return;                                              // Sale si esta fuera del lienzo.

    Color target = GetCell(startX, startY);                   // Guarda el color que se va a reemplazar.
    if (SameColor(target, newColor))                          // Comprueba si ya tiene el color nuevo.
        return;                                               // Evita trabajo innecesario.

    std::vector<Punto> pending;                               // Crea una pila de celdas pendientes.
    pending.push_back({ startX, startY });                    // Agrega la celda inicial.

    while (!pending.empty()) {                                // Continua mientras existan celdas.
        Punto p = pending.back();                             // Toma la ultima celda agregada.
        pending.pop_back();                                   // La elimina de la pila.

        if (!InsideGrid(p.x, p.y))                            // Comprueba que siga dentro del lienzo.
            continue;                                         // Ignora posiciones externas.
        if (!SameColor(GetCell(p.x, p.y), target))            // Comprueba si tiene el color original.
            continue;                                         // Si no coincide, no atraviesa esa celda.

        SetCell(p.x, p.y, newColor);                          // Cambia el color de la celda actual.
        pending.push_back({ p.x + 1, p.y });                 // Agrega el vecino derecho.
        pending.push_back({ p.x - 1, p.y });                 // Agrega el vecino izquierdo.
        pending.push_back({ p.x, p.y + 1 });                 // Agrega el vecino superior.
        pending.push_back({ p.x, p.y - 1 });                 // Agrega el vecino inferior.
    }
}


// ================================================================
// 13. ROTACION DE LA ULTIMA FIGURA
// ================================================================

void RotateLastFigure(float degrees) {                       // Rota los puntos de la ultima figura.
    if (g_lastFigure.empty())                                // Comprueba si existe una figura.
        return;                                              // No hace nada si no hay puntos.

    float centerX = 0.0f;                                    // Acumula las coordenadas X.
    float centerY = 0.0f;                                    // Acumula las coordenadas Y.
    for (const Punto& p : g_lastFigure) {                    // Recorre los puntos guardados.
        centerX += p.x;                                      // Suma la coordenada horizontal.
        centerY += p.y;                                      // Suma la coordenada vertical.
    }
    centerX /= static_cast<float>(g_lastFigure.size());       // Calcula el centro promedio en X.
    centerY /= static_cast<float>(g_lastFigure.size());       // Calcula el centro promedio en Y.

    const float pi = 3.14159265358979323846f;                 // Define el valor de PI.
    float angle = degrees * pi / 180.0f;                     // Convierte grados a radianes.
    float cosine = std::cos(angle);                          // Calcula el coseno del angulo.
    float sine = std::sin(angle);                            // Calcula el seno del angulo.

    SaveUndo();                                               // Guarda el lienzo antes de modificarlo.
    for (const Punto& p : g_lastFigure) {                    // Recorre los puntos originales.
        float dx = p.x - centerX;                            // Calcula X relativo al centro.
        float dy = p.y - centerY;                             // Calcula Y relativo al centro.
        int rotatedX = static_cast<int>(std::round(centerX + dx * cosine - dy * sine)); // Aplica la formula de rotacion X.
        int rotatedY = static_cast<int>(std::round(centerY + dx * sine + dy * cosine)); // Aplica la formula de rotacion Y.
        SetCell(p.x, p.y, g_background);                     // Borra la posicion original.
        SetCell(rotatedX, rotatedY, g_drawColor);             // Pinta la posicion rotada.
    }

    std::vector<Punto> rotatedPoints;                        // Crea una lista para los puntos nuevos.
    for (const Punto& p : g_lastFigure) {                    // Recorre nuevamente los puntos.
        float dx = p.x - centerX;                            // Calcula X respecto al centro.
        float dy = p.y - centerY;                             // Calcula Y respecto al centro.
        int rx = static_cast<int>(std::round(centerX + dx * cosine - dy * sine)); // Calcula X rotada.
        int ry = static_cast<int>(std::round(centerY + dx * sine + dy * cosine)); // Calcula Y rotada.
        rotatedPoints.push_back({ rx, ry });                 // Guarda el punto rotado.
    }
    g_lastFigure = rotatedPoints;                            // Reemplaza la figura anterior por la rotada.
}


// ================================================================
// 14. DIBUJAR UNA FIGURA COMPLETA
// ================================================================

void DrawCurrentShape(Punto a, Punto b, bool savePoints) {   // Dibuja la herramienta seleccionada.
    std::vector<Punto> points;                               // Guarda los puntos generados por la figura.
    Color color = (g_tool == TOOL_ERASER) ? g_background : g_drawColor; // Selecciona color de dibujo o fondo.

    if (g_tool == TOOL_LINE)                                 // Comprueba si esta seleccionada la linea.
        DrawLineBresenham(a.x, a.y, b.x, b.y, color, &points); // Dibuja una linea.
    else if (g_tool == TOOL_RECTANGLE)                       // Comprueba si esta seleccionado el rectangulo.
        DrawRectangle(a, b, color, &points);                 // Dibuja un rectangulo.
    else if (g_tool == TOOL_SQUARE)                          // Comprueba si esta seleccionado el cuadrado.
        DrawSquare(a, b, color, &points);                    // Dibuja un cuadrado.
    else if (g_tool == TOOL_TRIANGLE)                        // Comprueba si esta seleccionado el triangulo.
        DrawTriangle(a, b, color, &points);                 // Dibuja un triangulo.
    else if (g_tool == TOOL_CIRCLE) {                        // Comprueba si esta seleccionado el circulo.
        int dx = b.x - a.x;                                  // Calcula la distancia horizontal.
        int dy = b.y - a.y;                                  // Calcula la distancia vertical.
        int radius = static_cast<int>(std::round(std::sqrt(float(dx * dx + dy * dy)))); // Calcula el radio.
        DrawCircleBresenham(a.x, a.y, radius, color, &points); // Dibuja el circulo.
    }

    if (savePoints)                                         // Comprueba si se debe guardar la figura.
        g_lastFigure = points;                              // Guarda los puntos para permitir rotarlos.
}


// ================================================================
// 15. INTERFAZ GRAFICA DE DEAR IMGUI
// ================================================================

void DrawInterface(GLFWwindow* window) {                    // Construye el panel de herramientas.
    ImGui::SetNextWindowPos(ImVec2(820, 10), ImGuiCond_Once); // Ubica el panel a la derecha.
    ImGui::SetNextWindowSize(ImVec2(270, 760), ImGuiCond_Once); // Define el tamaño inicial del panel.

    ImGui::Begin("Herramientas - Mi Paint");                 // Abre la ventana de herramientas.
    ImGui::Text("MI PAINT");                                 // Muestra el titulo.
    ImGui::Separator();                                      // Dibuja una linea divisoria.
    ImGui::Text("Selecciona una herramienta:");              // Muestra una instruccion.

    for (int i = 0; i <= TOOL_ERASER; ++i) {                 // Recorre todas las herramientas.
        if (ImGui::RadioButton(g_toolNames[i], g_tool == i)) // Crea una opcion seleccionable.
            g_tool = static_cast<Tool>(i);                   // Actualiza la herramienta activa.
    }

    ImGui::Separator();                                      // Separa la zona de herramientas.
    ImGui::Text("Color de dibujo");                          // Muestra el titulo del selector.
    ImGui::ColorEdit4("Color", g_drawColor.data());          // Permite elegir un color RGBA.

    ImGui::Separator();                                      // Separa los controles de acciones.
    if (ImGui::Button("Limpiar lienzo", ImVec2(-1, 0))) {    // Crea el boton para limpiar.
        SaveUndo();                                          // Guarda el estado anterior.
        std::fill(g_cells.begin(), g_cells.end(), g_background); // Pinta todas las celdas con el fondo.
        g_lastFigure.clear();                                // Elimina la figura que podia rotarse.
    }

    if (ImGui::Button("Deshacer (Ctrl + Z)", ImVec2(-1, 0))) // Crea el boton para deshacer.
        Undo();                                              // Recupera el estado anterior.

    ImGui::Separator();                                      // Separa las opciones de rotacion.
    ImGui::Text("Rotar ultima figura");                      // Muestra el titulo de rotacion.
    if (ImGui::Button("Girar 45 grados", ImVec2(-1, 0)))     // Crea el boton de 45 grados.
        RotateLastFigure(45.0f);                             // Rota la figura 45 grados.
    if (ImGui::Button("Girar 90 grados", ImVec2(-1, 0)))     // Crea el boton de 90 grados.
        RotateLastFigure(90.0f);                             // Rota la figura 90 grados.

    ImGui::Separator();                                      // Separa la ayuda.
    ImGui::TextWrapped("Uso: selecciona una herramienta y dibuja sobre el lienzo con el mouse."); // Explica el uso.
    ImGui::TextWrapped("Para el balde, haz clic dentro de una zona cerrada."); // Explica el relleno.
    ImGui::TextWrapped("Las figuras geometricas se crean arrastrando desde el inicio hasta el final."); // Explica el arrastre.
    ImGui::TextWrapped("OLGA LUCIA GUACANEME ARCHILA - 6014476");

    if (ImGui::Button("Salir", ImVec2(-1, 0)))               // Crea el boton para cerrar.
        glfwSetWindowShouldClose(window, GLFW_TRUE);         // Solicita cerrar la ventana.

    ImGui::End();                                            // Cierra la ventana de herramientas.
}


// ================================================================
// 16. CONVERTIR EL MOUSE A COORDENADAS DE LA GRILLA
// ================================================================

bool MouseToGrid(GLFWwindow* window, int canvasX, int canvasY, int canvasSize, Punto& result) {
    double mouseX = 0.0;                                     // Guarda la posicion horizontal del mouse.
    double mouseY = 0.0;                                     // Guarda la posicion vertical del mouse.
    glfwGetCursorPos(window, &mouseX, &mouseY);              // Consulta la posicion del cursor en la ventana.

    int windowW = 1;                                         // Guarda el ancho logico de la ventana.
    int windowH = 1;                                         // Guarda el alto logico de la ventana.
    glfwGetWindowSize(window, &windowW, &windowH);           // Obtiene las dimensiones de la ventana.

    int fbW = 1;                                             // Guarda el ancho real del framebuffer.
    int fbH = 1;                                             // Guarda el alto real del framebuffer.
    glfwGetFramebufferSize(window, &fbW, &fbH);             // Obtiene las dimensiones del framebuffer.

    float scaleX = static_cast<float>(fbW) / windowW;       // Cal  cula la escala horizontal.
    float scaleY = static_cast<float>(fbH) / windowH;       // Calcula la escala vertical.
    float px = static_cast<float>(mouseX) * scaleX;         // Convierte el mouse a pixeles reales.
    float py = static_cast<float>(mouseY) * scaleY;         // Convierte el mouse a pixeles reales.
    float glY = static_cast<float>(fbH) - py;               // Convierte Y al origen inferior de OpenGL.

    if (px < canvasX || px >= canvasX + canvasSize ||       // Comprueba los limites horizontales.
        glY < canvasY || glY >= canvasY + canvasSize)       // Comprueba los limites verticales.
        return false;                                       // Informa que el mouse esta fuera del lienzo.

    result.x = static_cast<int>((px - canvasX) / canvasSize * g_cols); // Convierte X a columna.
    result.y = static_cast<int>((glY - canvasY) / canvasSize * g_rows); // Convierte Y a fila; coincide con el eje vertical de OpenGL.
    return InsideGrid(result.x, result.y);                  // Devuelve si la celda es valida.
}


// ================================================================
// 17. FUNCION PRINCIPAL
// ================================================================

int main() {                                                // Punto de entrada del programa.

    if (!glfwInit()) {                                      // Inicializa GLFW.
        std::cerr << "No se pudo iniciar GLFW." << std::endl; // Muestra el error.
        return -1;                                          // Termina con error.
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);          // Solicita OpenGL 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Solicita OpenGL 3.3.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Solicita Core Profile.

    GLFWwindow* window = glfwCreateWindow(1100, 800, "La primera pincelada – My Paint - ", nullptr, nullptr); // Crea la ventana.
    if (!window) {                                          // Comprueba si se creo.
        std::cerr << "No se pudo crear la ventana." << std::endl; // Muestra el error.
        glfwTerminate();                                    // Cierra GLFW.
        return -1;                                          // Termina con error.
    }

    glfwMakeContextCurrent(window);                         // Activa el contexto OpenGL.
    glfwSwapInterval(1);                                    // Activa sincronizacion vertical.

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { // Inicializa GLAD.
        std::cerr << "No se pudo iniciar GLAD." << std::endl; // Muestra el error.
        glfwDestroyWindow(window);                          // Destruye la ventana.
        glfwTerminate();                                    // Cierra GLFW.
        return -1;                                          // Termina con error.
    }

    IMGUI_CHECKVERSION();                                   // Comprueba la version de Dear ImGui.
    ImGui::CreateContext();                                 // Crea el contexto de ImGui.
    ImGui::StyleColorsDark();                               // Aplica el tema oscuro.
    ImGui_ImplGlfw_InitForOpenGL(window, true);             // Conecta ImGui con GLFW.
    ImGui_ImplOpenGL3_Init("#version 330");                 // Conecta ImGui con OpenGL 3.3.

    g_cells.assign(g_cols * g_rows, g_background);          // Inicializa todas las celdas con el fondo.
    BuildGrid();                                            // Construye vertices e indices.
    g_shaderProgram = CreateShaderProgram();                // Compila y enlaza los shaders.
    CreateGridBuffers();                                    // Crea los buffers de OpenGL.

    glClearColor(0.85f, 0.85f, 0.85f, 1.0f);               // Define el color que rodea al lienzo.

    while (!glfwWindowShouldClose(window)) {                // Bucle principal del programa.
        glfwPollEvents();                                   // Procesa eventos de teclado y mouse.

        ImGui_ImplOpenGL3_NewFrame();                       // Inicia un nuevo frame de OpenGL para ImGui.
        ImGui_ImplGlfw_NewFrame();                          // Actualiza los datos de GLFW para ImGui.
        ImGui::NewFrame();                                  // Inicia un nuevo frame de interfaz.

        DrawInterface(window);                              // Dibuja el panel de herramientas.

        int fbW = 1;                                        // Guarda el ancho del framebuffer.
        int fbH = 1;                                        // Guarda el alto del framebuffer.
        glfwGetFramebufferSize(window, &fbW, &fbH);         // Consulta el tamaño real de dibujo.

        int canvasSize = std::min(fbH - 20, fbW - 310);     // Calcula un lienzo cuadrado que quepa en la ventana.
        canvasSize = std::max(canvasSize, 1);               // Evita que el tamaño sea cero.
        int canvasX = 10;                                   // Deja un margen izquierdo.
        int canvasY = (fbH - canvasSize) / 2;               // Centra verticalmente el lienzo.

        ImGuiIO& io = ImGui::GetIO();                       // Obtiene los datos de entrada de ImGui.
        Punto mouseCell = { 0, 0 };                         // Guarda la celda debajo del mouse.
        bool overCanvas = MouseToGrid(window, canvasX, canvasY, canvasSize, mouseCell); // Comprueba si esta sobre el lienzo.
        bool overUI = io.WantCaptureMouse;                  // Comprueba si ImGui esta usando el mouse.

        if (ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyCtrl)  // Comprueba Ctrl + Z.
            Undo();                                         // Deshace el ultimo cambio.

        if (overCanvas && !overUI) {                        // Solo procesa dibujo sobre el lienzo.
            if (g_tool == TOOL_FILL && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { // Si se usa el balde...
                SaveUndo();                                 // Guarda el lienzo antes del relleno.
                FloodFill(mouseCell.x, mouseCell.y, g_drawColor); // Rellena la region seleccionada.
            }
             else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { // Detecta el inicio de un trazo.
                SaveUndo();                                 // Guarda el estado antes de dibujar.
                g_dragging = true;                          // Activa el modo de arrastre.
                g_start = mouseCell;                        // Guarda el punto inicial.
                g_current = mouseCell;                      // Inicializa el punto actual.
                g_last = mouseCell;                         // Inicializa el punto anterior.

                if (g_tool == TOOL_PENCIL)                  // Si la herramienta es lapiz...
                    SetCell(mouseCell.x, mouseCell.y, g_drawColor); // Pinta la primera celda.
                else if (g_tool == TOOL_ERASER)             // Si la herramienta es borrador...
                    SetCell(mouseCell.x, mouseCell.y, g_background); // Borra la primera celda.
            }

            if (g_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) { // Mientras se mantiene presionado...
                g_current = mouseCell;                      // Actualiza la posicion actual.
                if (g_tool == TOOL_PENCIL)                  // Si se dibuja con lapiz...
                    DrawFreehand(g_last, g_current, g_drawColor); // Une la posicion anterior con la actual.
                else if (g_tool == TOOL_ERASER)             // Si se utiliza borrador...
                    DrawFreehand(g_last, g_current, g_background); // Borra siguiendo el movimiento.
                g_last = g_current;                         // Guarda la posicion para el siguiente movimiento.
            }
        }

        if (g_dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { // Detecta cuando se suelta el mouse.
            if (g_tool == TOOL_LINE || g_tool == TOOL_RECTANGLE || // Comprueba herramientas geometricas.
                g_tool == TOOL_SQUARE || g_tool == TOOL_TRIANGLE || g_tool == TOOL_CIRCLE) {
                DrawCurrentShape(g_start, g_current, true); // Dibuja la figura definitiva y guarda sus puntos.
            }
            g_dragging = false;                             // Finaliza el arrastre.
            g_last = { -1, -1 };                            // Reinicia el punto anterior.
        }

        UpdateVertexColors();                               // Actualiza los colores del VBO.

        glViewport(0, 0, fbW, fbH);                          // Define el viewport general.
        glClear(GL_COLOR_BUFFER_BIT);                       // Limpia la ventana.
        glViewport(canvasX, canvasY, canvasSize, canvasSize); // Define el viewport cuadrado del lienzo.
        glUseProgram(g_shaderProgram);                      // Activa los shaders del lienzo.
        glBindVertexArray(g_VAO);                           // Activa el VAO de la grilla.
        glDrawElements(GL_TRIANGLES, g_indexCount, GL_UNSIGNED_INT, nullptr); // Dibuja todas las celdas.
        glBindVertexArray(0);                               // Desactiva el VAO.

        glViewport(0, 0, fbW, fbH);                          // Restaura el viewport para ImGui.
        ImGui::Render();                                    // Finaliza la construccion de la interfaz.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Dibuja la interfaz sobre OpenGL.

        glfwSwapBuffers(window);                            // Presenta el frame terminado.
    }

    glDeleteVertexArrays(1, &g_VAO);                        // Elimina el VAO.
    glDeleteBuffers(1, &g_VBO);                             // Elimina el VBO.
    glDeleteBuffers(1, &g_EBO);                             // Elimina el EBO.
    glDeleteProgram(g_shaderProgram);                      // Elimina el programa de shaders.

    ImGui_ImplOpenGL3_Shutdown();                           // Libera la conexion ImGui-OpenGL.
    ImGui_ImplGlfw_Shutdown();                              // Libera la conexion ImGui-GLFW.
    ImGui::DestroyContext();                                // Destruye el contexto de ImGui.

    glfwDestroyWindow(window);                              // Destruye la ventana.
    glfwTerminate();                                        // Cierra GLFW.
    return 0;                                               // Termina correctamente.
}
