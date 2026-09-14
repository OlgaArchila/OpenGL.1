#include <iostream>                 // Permite mostrar mensajes en consola.
#include <vector>                   // Permite guardar los puntos.
#include <cstdlib>                  // Permite utilizar abs().
#include <glad/glad.h>              // Carga las funciones de OpenGL.
#include <GLFW/glfw3.h>             // Permite crear la ventana.


// VERTEX SHADER
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // Recibe la posición.

void main()
{
    gl_Position = vec4(aPos, 1.0); // Define la posición.
}
)";


// FRAGMENT SHADER
const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor; // Guarda el color.

void main()
{
    FragColor = vec4(0.7f, 0.4f, 1.0f, 1.0f); // Color morado claro.
}
)";


// ESTRUCTURA DE UN PUNTO
struct Punto
{
    int x;                          // Coordenada X.
    int y;                          // Coordenada Y.
};


// ALGORITMO DE BRESENHAM
void bresenham(
    int x0,                        // X inicial.
    int y0,                        // Y inicial.
    int x1,                        // X final.
    int y1,                        // Y final.
    std::vector<Punto>& puntos     // Guarda los puntos.
)
{
    int dx = abs(x1 - x0);         // Distancia horizontal.
    int dy = abs(y1 - y0);         // Distancia vertical.

    int sx = (x0 < x1) ? 1 : -1;   // Dirección en X.
    int sy = (y0 < y1) ? 1 : -1;   // Dirección en Y.

    int error = dx - dy;            // Error inicial.

    while (true)
    {
        puntos.push_back({ x0, y0 }); // Guarda el punto.

        if (x0 == x1 && y0 == y1)   // Comprueba el final.
            break;

        int e2 = 2 * error;         // Calcula el error.

        if (e2 > -dy)
        {
            error -= dy;            // Ajusta el error.
            x0 += sx;               // Avanza en X.
        }

        if (e2 < dx)
        {
            error += dx;            // Ajusta el error.
            y0 += sy;               // Avanza en Y.
        }
    }
}


// CONVIERTE X A OPENGL
float convertirX(int x)
{
    return (2.0f * x / 800.0f) - 1.0f; // Convierte X.
}


// CONVIERTE Y A OPENGL
float convertirY(int y)
{
    return (2.0f * y / 600.0f) - 1.0f; // Convierte Y.
}


int main()
{
    glfwInit();                     // Inicializa GLFW.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Usa OpenGL 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Usa OpenGL 3.3.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa Core.

    GLFWwindow* window = glfwCreateWindow(
        800,                        // Ancho.
        600,                        // Alto.
        "OlgaOpenGL",               // Nombre.
        NULL,                       // No pantalla completa.
        NULL                        // No comparte contexto.
    );

    if (window == NULL)             // Comprueba la ventana.
    {
        std::cout << "No se pudo crear la ventana" << std::endl;
        glfwTerminate();            // Cierra GLFW.
        return -1;                  // Termina.
    }

    glfwMakeContextCurrent(window); // Activa la ventana.

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Carga GLAD.
    {
        std::cout << "No se pudo inicializar GLAD" << std::endl;
        glfwTerminate();            // Cierra GLFW.
        return -1;                  // Termina.
    }

    glViewport(0, 0, 800, 600);    // Define el área de dibujo.


    // VERTEX SHADER
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // Crea shader.

    glShaderSource(
        vertexShader,               // Shader.
        1,                          // Cantidad.
        &vertexShaderSource,        // Código.
        NULL                        // Sin longitud.
    );

    glCompileShader(vertexShader);  // Compila shader.


    // FRAGMENT SHADER
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Crea shader.

    glShaderSource(
        fragmentShader,             // Shader.
        1,                          // Cantidad.
        &fragmentShaderSource,      // Código.
        NULL                        // Sin longitud.
    );

    glCompileShader(fragmentShader); // Compila shader.


    // PROGRAMA DE SHADERS
    unsigned int shaderProgram = glCreateProgram(); // Crea programa.

    glAttachShader(shaderProgram, vertexShader);    // Agrega vertex shader.
    glAttachShader(shaderProgram, fragmentShader);  // Agrega fragment shader.

    glLinkProgram(shaderProgram);                   // Une los shaders.

    glDeleteShader(vertexShader);                   // Elimina shader.
    glDeleteShader(fragmentShader);                 // Elimina shader.


    // CONFIGURACIÓN DE LA GRILLA

    int tamanoCuadrado = 40;        // Tamaño de cada celda.

    int numeroColumnas = 20;        // Cantidad de columnas.

    int numeroFilas = 15;           // Cantidad de filas.

    int anchoGrilla = numeroColumnas * tamanoCuadrado; // Ancho total.

    int altoGrilla = numeroFilas * tamanoCuadrado;     // Alto total.


    // PUNTOS DE LA GRILLA

    std::vector<Punto> puntosGrilla; // Guarda los puntos.


    // LÍNEAS VERTICALES

    for (int i = 0; i <= numeroColumnas; i++) // Recorre columnas.
    {
        int x = i * tamanoCuadrado;            // Calcula X.

        bresenham(
            x,                                  // X inicial.
            0,                                  // Y inicial.
            x,                                  // X final.
            altoGrilla,                         // Y final.
            puntosGrilla                        // Guarda puntos.
        );
    }


    // LÍNEAS HORIZONTALES

    for (int i = 0; i <= numeroFilas; i++)    // Recorre filas.
    {
        int y = i * tamanoCuadrado;            // Calcula Y.

        bresenham(
            0,                                  // X inicial.
            y,                                  // Y inicial.
            anchoGrilla,                        // X final.
            y,                                  // Y final.
            puntosGrilla                        // Guarda puntos.
        );
    }


    // PUNTOS DE LA LÍNEA

    std::vector<Punto> puntosLinea;            // Guarda las celdas seleccionadas.


    // CALCULA LAS CELDAS DE LA LÍNEA CON BRESENHAM

    bresenham(
        0,                                      // Columna inicial.
        0,                                      // Fila inicial.
        numeroColumnas - 1,                     // Columna final.
        numeroFilas - 1,                        // Fila final.
        puntosLinea                             // Guarda las celdas.
    );


    // CONVIERTE LOS PUNTOS A OPENGL

    std::vector<float> vertices;       // Guarda los vértices.


    for (const Punto& punto : puntosGrilla) // Recorre los puntos.
    {
        float x = convertirX(punto.x); // Convierte X.
        float y = convertirY(punto.y); // Convierte Y.

        vertices.push_back(x);         // Guarda X.
        vertices.push_back(y);         // Guarda Y.
        vertices.push_back(0.0f);      // Guarda Z.
    }


    // CREA LOS CUADRADOS DE LA LÍNEA

    std::vector<float> verticesLinea;  // Guarda los vértices de los cuadrados.

    for (const Punto& punto : puntosLinea) // Recorre las celdas seleccionadas.
    {
        int x0 = punto.x * tamanoCuadrado; // Calcula X inicial del cuadrado.
        int y0 = punto.y * tamanoCuadrado; // Calcula Y inicial del cuadrado.

        int x1 = x0 + tamanoCuadrado;      // Calcula X final del cuadrado.
        int y1 = y0 + tamanoCuadrado;      // Calcula Y final del cuadrado.


        float izquierda = convertirX(x0);  // Convierte lado izquierdo.
        float derecha = convertirX(x1);    // Convierte lado derecho.

        float abajo = convertirY(y0);      // Convierte lado inferior.
        float arriba = convertirY(y1);     // Convierte lado superior.


        // PRIMER TRIÁNGULO DEL CUADRADO

        verticesLinea.push_back(izquierda); // Guarda X.
        verticesLinea.push_back(abajo);    // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.

        verticesLinea.push_back(derecha);   // Guarda X.
        verticesLinea.push_back(abajo);    // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.

        verticesLinea.push_back(derecha);   // Guarda X.
        verticesLinea.push_back(arriba);   // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.


        // SEGUNDO TRIÁNGULO DEL CUADRADO

        verticesLinea.push_back(izquierda); // Guarda X.
        verticesLinea.push_back(abajo);    // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.

        verticesLinea.push_back(derecha);   // Guarda X.
        verticesLinea.push_back(arriba);   // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.

        verticesLinea.push_back(izquierda); // Guarda X.
        verticesLinea.push_back(arriba);   // Guarda Y.
        verticesLinea.push_back(0.0f);     // Guarda Z.
    }


    // VAO Y VBO

    unsigned int VAO;                  // Identificador del VAO.
    unsigned int VBO;                  // Identificador del VBO.

    glGenVertexArrays(1, &VAO);        // Crea VAO.
    glGenBuffers(1, &VBO);             // Crea VBO.

    glBindVertexArray(VAO);            // Activa VAO.

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Activa VBO.

    glBufferData(
        GL_ARRAY_BUFFER,               // Tipo.
        vertices.size() * sizeof(float), // Tamaño.
        vertices.data(),               // Datos.
        GL_STATIC_DRAW                 // Datos estáticos.
    );


    // CONFIGURACIÓN DE LOS VÉRTICES

    glVertexAttribPointer(
        0,                              // Ubicación.
        3,                              // X, Y, Z.
        GL_FLOAT,                       // Tipo.
        GL_FALSE,                       // Sin normalizar.
        3 * sizeof(float),              // Tamaño.
        (void*)0                        // Inicio.
    );

    glEnableVertexAttribArray(0);       // Activa atributo.


    // VAO Y VBO DE LOS CUADRADOS PINTADOS

    unsigned int VAOLinea;              // Identificador del VAO de la línea.
    unsigned int VBOLinea;              // Identificador del VBO de la línea.

    glGenVertexArrays(1, &VAOLinea);    // Crea VAO de la línea.
    glGenBuffers(1, &VBOLinea);        // Crea VBO de la línea.

    glBindVertexArray(VAOLinea);        // Activa VAO de la línea.

    glBindBuffer(GL_ARRAY_BUFFER, VBOLinea); // Activa VBO de la línea.

    glBufferData(
        GL_ARRAY_BUFFER,                // Tipo.
        verticesLinea.size() * sizeof(float), // Tamaño.
        verticesLinea.data(),           // Datos.
        GL_STATIC_DRAW                  // Datos estáticos.
    );


    // CONFIGURACIÓN DE LOS VÉRTICES DE LOS CUADRADOS

    glVertexAttribPointer(
        0,                              // Ubicación.
        3,                              // X, Y, Z.
        GL_FLOAT,                       // Tipo.
        GL_FALSE,                       // Sin normalizar.
        3 * sizeof(float),              // Tamaño.
        (void*)0                        // Inicio.
    );

    glEnableVertexAttribArray(0);       // Activa atributo.


    glPointSize(1.5f);                  // Tamaño de los puntos.


    // BUCLE PRINCIPAL

    while (!glfwWindowShouldClose(window)) // Repite hasta cerrar.
    {
        glClearColor(
            0.02f,                      // Rojo.
            0.05f,                      // Verde.
            0.10f,                      // Azul.
            1.0f                        // Opacidad.
        );

        glClear(GL_COLOR_BUFFER_BIT);   // Limpia pantalla.

        glUseProgram(shaderProgram);    // Activa shaders.


        // DIBUJA LOS CUADRADOS DE LA LÍNEA

        glBindVertexArray(VAOLinea);    // Activa VAO de los cuadrados.

        glDrawArrays(
            GL_TRIANGLES,               // Dibuja los cuadrados con triángulos.
            0,                          // Primer vértice.
            verticesLinea.size() / 3    // Cantidad de vértices.
        );


        // DIBUJA LA GRILLA

        glBindVertexArray(VAO);         // Activa VAO de la grilla.

        glDrawArrays(
            GL_POINTS,                  // Dibuja puntos.
            0,                          // Primer punto.
            vertices.size() / 3         // Cantidad.
        );


        glfwSwapBuffers(window);        // Actualiza pantalla.
        glfwPollEvents();               // Procesa eventos.
    }


    // LIBERA LOS RECURSOS

    glDeleteVertexArrays(1, &VAO);      // Elimina VAO.
    glDeleteBuffers(1, &VBO);           // Elimina VBO.

    glDeleteVertexArrays(1, &VAOLinea); // Elimina VAO de los cuadrados.
    glDeleteBuffers(1, &VBOLinea);      // Elimina VBO de los cuadrados.

    glDeleteProgram(shaderProgram);     // Elimina programa.

    glfwDestroyWindow(window);          // Destruye ventana.
    glfwTerminate();                    // Finaliza GLFW.

    return 0;                           // Finaliza correctamente.
}