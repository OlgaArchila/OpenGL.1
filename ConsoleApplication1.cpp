#include <iostream>                 // Permite mostrar mensajes en consola.
#include <vector>                   // Permite guardar los puntos de Bresenham.
#include <cstdlib>                  // Permite utilizar abs().
#include <glad/glad.h>              // Carga las funciones de OpenGL.
#include <GLFW/glfw3.h>             // Permite crear y manejar la ventana.


// VERTEX SHADER
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // Recibe la posición del punto.

void main()
{
    gl_Position = vec4(aPos, 1.0); // Define la posición del punto.
}
)";


// FRAGMENT SHADER
const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor; // Guarda el color del punto.

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.0f, 1.0f); // Color naranja.
}
)";


// ESTRUCTURA DE UN PUNTO
struct Punto
{
    int x;                          // Coordenada X.
    int y;                          // Coordenada Y.
};


// ALGORITMO DE BRESENHAM
void bresenham(int x0, int y0, int x1, int y1, std::vector<Punto>& puntos)
{
    int dx = abs(x1 - x0);          // Calcula la distancia horizontal.
    int dy = abs(y1 - y0);          // Calcula la distancia vertical.

    int sx = (x0 < x1) ? 1 : -1;    // Define la dirección en X.
    int sy = (y0 < y1) ? 1 : -1;    // Define la dirección en Y.

    int error = dx - dy;            // Calcula el error inicial.

    while (true)
    {
        puntos.push_back({ x0, y0 }); // Guarda el punto actual.

        if (x0 == x1 && y0 == y1)   // Comprueba si llegó al final.
            break;

        int e2 = 2 * error;         // Calcula el doble del error.

        if (e2 > -dy)
        {
            error -= dy;            // Corrige el error.
            x0 += sx;                // Avanza en X.
        }

        if (e2 < dx)
        {
            error += dx;            // Corrige el error.
            y0 += sy;                // Avanza en Y.
        }
    }
}


// CONVIERTE X DE PIXELES A OPENGL
float convertirX(int x)
{
    return (2.0f * x / 800.0f) - 1.0f; // Convierte X a -1 hasta 1.
}


// CONVIERTE Y DE PIXELES A OPENGL
float convertirY(int y)
{
    return (2.0f * y / 600.0f) - 1.0f; // Convierte Y a -1 hasta 1.
}


int main()
{
    glfwInit();                     // Inicializa GLFW.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Usa OpenGL 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Usa OpenGL 3.3.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa Core.

    GLFWwindow* window = glfwCreateWindow(
        800,                         // Ancho de la ventana.
        600,                         // Alto de la ventana.
        "OlgaOpenGL",                // Nombre de la ventana.
        NULL,                        // No usa pantalla completa.
        NULL                         // No comparte contexto.
    );

    if (window == NULL)              // Comprueba si se creó la ventana.
    {
        std::cout << "No se pudo crear la ventana" << std::endl;
        glfwTerminate();             // Cierra GLFW.
        return -1;                   // Termina el programa.
    }

    glfwMakeContextCurrent(window);  // Activa la ventana.

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Carga GLAD.
    {
        std::cout << "No se pudo inicializar GLAD" << std::endl;
        glfwTerminate();             // Cierra GLFW.
        return -1;                   // Termina el programa.
    }

    glViewport(0, 0, 800, 600);     // Define el área de dibujo.


    // VERTEX SHADER
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // Crea shader.

    glShaderSource(
        vertexShader,                // Shader que recibe el código.
        1,                           // Cantidad de códigos.
        &vertexShaderSource,         // Código del vertex shader.
        NULL                         // Sin longitudes específicas.
    );

    glCompileShader(vertexShader);   // Compila el vertex shader.


    // FRAGMENT SHADER
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Crea shader.

    glShaderSource(
        fragmentShader,              // Shader que recibe el código.
        1,                           // Cantidad de códigos.
        &fragmentShaderSource,       // Código del fragment shader.
        NULL                         // Sin longitudes específicas.
    );

    glCompileShader(fragmentShader); // Compila el fragment shader.


    // PROGRAMA DE SHADERS
    unsigned int shaderProgram = glCreateProgram(); // Crea el programa.

    glAttachShader(shaderProgram, vertexShader);    // Agrega vertex shader.
    glAttachShader(shaderProgram, fragmentShader);  // Agrega fragment shader.

    glLinkProgram(shaderProgram);                   // Une los shaders.

    glDeleteShader(vertexShader);                   // Elimina el shader.
    glDeleteShader(fragmentShader);                 // Elimina el shader.


    // CONFIGURACIÓN DE LA GRILLA

    int numeroCuadradosX = 4;       // Número de columnas.
    int numeroCuadradosY = 3;       // Número de filas.

    int tamanoCuadrado = 200;       // Tamaño de cada cuadrado.

    int anchoGrilla = numeroCuadradosX * tamanoCuadrado; // Ancho total.
    int altoGrilla = numeroCuadradosY * tamanoCuadrado;  // Alto total.

    int inicioX = 0;                // Inicio horizontal.
    int inicioY = 0;                // Inicio vertical.


    // PUNTOS DE LA GRILLA
    std::vector<Punto> puntosGrilla; // Guarda todos los puntos.


    // LÍNEAS VERTICALES

    for (int i = 0; i <= numeroCuadradosX; i++) // Recorre las columnas.
    {
        int x = inicioX + i * tamanoCuadrado; // Calcula X de la línea.

        bresenham(
            x,                          // Punto inicial X.
            inicioY,                    // Punto inicial Y.
            x,                          // Punto final X.
            inicioY + altoGrilla,       // Punto final Y.
            puntosGrilla                // Guarda los puntos.
        );
    }


    // LÍNEAS HORIZONTALES

    for (int i = 0; i <= numeroCuadradosY; i++) // Recorre las filas.
    {
        int y = inicioY + i * tamanoCuadrado; // Calcula Y de la línea.

        bresenham(
            inicioX,                    // Punto inicial X.
            y,                          // Punto inicial Y.
            inicioX + anchoGrilla,      // Punto final X.
            y,                          // Punto final Y.
            puntosGrilla                // Guarda los puntos.
        );
    }


    // VÉRTICES PARA OPENGL

    std::vector<float> vertices;      // Guarda las coordenadas OpenGL.


    for (const Punto& punto : puntosGrilla) // Recorre los puntos.
    {
        float x = convertirX(punto.x); // Convierte X a OpenGL.
        float y = convertirY(punto.y); // Convierte Y a OpenGL.

        vertices.push_back(x);         // Guarda X.
        vertices.push_back(y);         // Guarda Y.
        vertices.push_back(0.0f);      // Guarda Z.
    }


    // VAO
    unsigned int VAO;                 // Identificador del VAO.

    // VBO
    unsigned int VBO;                 // Identificador del VBO.

    glGenVertexArrays(1, &VAO);       // Crea el VAO.
    glGenBuffers(1, &VBO);            // Crea el VBO.

    glBindVertexArray(VAO);           // Activa el VAO.

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Activa el VBO.

    glBufferData(
        GL_ARRAY_BUFFER,              // Tipo de buffer.
        vertices.size() * sizeof(float), // Tamaño de los datos.
        vertices.data(),              // Datos de los vértices.
        GL_STATIC_DRAW                // Los datos no cambian.
    );


    // CONFIGURACIÓN DE LOS VÉRTICES

    glVertexAttribPointer(
        0,                             // Ubicación del atributo.
        3,                             // X, Y y Z.
        GL_FLOAT,                      // Tipo de dato.
        GL_FALSE,                      // No normaliza.
        3 * sizeof(float),             // Tamaño de cada vértice.
        (void*)0                       // Inicio de los datos.
    );

    glEnableVertexAttribArray(0);      // Activa el atributo.


    glPointSize(2.0f);                 // Define el tamaño de los puntos.


    // BUCLE PRINCIPAL

    while (!glfwWindowShouldClose(window)) // Repite hasta cerrar.
    {
        glClearColor(
            0.12f,                     // Rojo.
            0.07f,                     // Verde.
            0.17f,                     // Azul.
            1.0f                       // Transparencia.
        );

        glClear(GL_COLOR_BUFFER_BIT);  // Limpia la pantalla.

        glUseProgram(shaderProgram);   // Activa los shaders.

        glBindVertexArray(VAO);        // Activa el VAO.

        glDrawArrays(
            GL_POINTS,                 // Dibuja los puntos.
            0,                         // Comienza desde el primer punto.
            vertices.size() / 3        // Cantidad de puntos.
        );

        glfwSwapBuffers(window);       // Muestra el dibujo.
        glfwPollEvents();              // Procesa eventos.
    }


    // LIBERA LOS RECURSOS

    glDeleteVertexArrays(1, &VAO);     // Elimina el VAO.
    glDeleteBuffers(1, &VBO);          // Elimina el VBO.
    glDeleteProgram(shaderProgram);    // Elimina el programa.

    glfwDestroyWindow(window);         // Destruye la ventana.
    glfwTerminate();                   // Finaliza GLFW.

    return 0;                          // Finaliza correctamente.
}

