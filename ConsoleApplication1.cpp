#include <iostream>                 // Permite mostrar mensajes en la consola usando std::cout.
#include <glad/glad.h>              // Incluye GLAD, que permite cargar las funciones de OpenGL.
#include <GLFW/glfw3.h>             // Incluye GLFW, que permite crear y manejar la ventana.
#include <cmath>                    // Permite utilizar funciones matemáticas, como sqrt() para calcular raíces cuadradas.

// VERTEX SHADER
// Guarda el código que se ejecutará para cada vértice.
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // Recibe la posición del vértice.

void main()
{
    gl_Position = vec4(aPos, 1.0); // Convierte la posición en un vector de 4 componentes.
}
)";
// FRAGMENT SHADER
// Guarda el código que determina el color de cada fragmento.
const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor; // Variable de salida que almacena el color final.

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.0f, 1.0f); // Define el color naranja.
}
)";

int main()
{
    // Inicializa GLFW.
    glfwInit();

    // Indica que se utilizará OpenGL 3.3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Indica que se utilizará el perfil Core de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crea una ventana de 800 píxeles de ancho y 600 píxeles de alto.
    GLFWwindow* window = glfwCreateWindow(800, 600, "OlgaOpenGL", NULL, NULL);

    // Verifica si la ventana fue creada correctamente.
    if (window == NULL)
    {
        std::cout << "No se pudo crear la ventana" << std::endl;

        // Finaliza GLFW si hubo un error.
        glfwTerminate();

        return -1;
    }

    // Hace que la ventana creada sea el contexto actual de OpenGL.
    glfwMakeContextCurrent(window);

    // Inicializa GLAD para poder utilizar las funciones de OpenGL.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "No se pudo inicializar GLAD" << std::endl;

        return -1;
    }

    // Define el área de la ventana donde se dibujará.
    glViewport(0, 0, 800, 600);

    // VERTEX SHADER
    // Crea un objeto para almacenar el vertex shader.
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Envía el código del vertex shader a OpenGL.
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // Compila el vertex shader.
    glCompileShader(vertexShader);

    // FRAGMENT SHADER
    // Crea un objeto para almacenar el fragment shader.
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Envía el código del fragment shader a OpenGL.
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // Compila el fragment shader.
    glCompileShader(fragmentShader);

    // PROGRAMA DE SHADERS
    // Crea un programa para unir los dos shaders.
    unsigned int shaderProgram = glCreateProgram();

    // Une el vertex shader al programa.
    glAttachShader(shaderProgram, vertexShader);

    // Une el fragment shader al programa.
    glAttachShader(shaderProgram, fragmentShader);

    // Conecta los shaders para formar el programa final.
    glLinkProgram(shaderProgram);

    // Elimina el vertex shader porque ya está unido al programa.
    glDeleteShader(vertexShader);

    // Elimina el fragment shader porque ya está unido al programa.
    glDeleteShader(fragmentShader);


    // VÉRTICES DEL CUADRADO
    // Cada vértice tiene una posición en X, Y y Z.
    float vertices[] = {

        // Vértice 0: esquina inferior izquierda.
        -0.5f, -0.5f, 0.0f,

        // Vértice 1: esquina inferior derecha.
         0.5f, -0.5f, 0.0f,

         // Vértice 2: esquina superior derecha.
          0.5f,  0.5f, 0.0f,

          // Vértice 3: esquina superior izquierda.
          -0.5f,  0.5f, 0.0f
    };

    // ÍNDICES
    // Indican qué vértices se utilizarán para formar los dos triángulos.
    unsigned int indices[] = {

        // Primer triángulo.
        0, 1, 2,

        // Segundo triángulo.
        0, 2, 3
    };


    // VAO
    // Almacena la configuración de los atributos de los vértices.
    unsigned int VAO;

    // VBO
    // Almacena los datos de los vértices.
    unsigned int VBO;

    // EBO
    // Almacena los índices de los vértices.
    unsigned int EBO;


    // Genera un Vertex Array Object.
    glGenVertexArrays(1, &VAO);

    // Genera un Vertex Buffer Object.
    glGenBuffers(1, &VBO);

    // Genera un Element Buffer Object.
    glGenBuffers(1, &EBO);


    // Activa el VAO.
    glBindVertexArray(VAO);


    // Activa el VBO.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copia los datos de los vértices al VBO.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    // Activa el EBO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Copia los índices al EBO.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Indica a OpenGL cómo debe interpretar los datos de los vértices.
    glVertexAttribPointer(
        0,                          // Ubicación del atributo.
        3,                          // Cantidad de valores por vértice: X, Y y Z.
        GL_FLOAT,                   // Tipo de dato.
        GL_FALSE,                   // Indica que no se normalizarán los datos.
        3 * sizeof(float),          // Tamaño total de cada vértice.
        (void*)0                    // Posición donde comienzan los datos.
    );

    // Activa el atributo de posición de los vértices.
    glEnableVertexAttribArray(0);

    // BUCLE PRINCIPAL
    // Se ejecuta mientras la ventana no se cierre.
    while (!glfwWindowShouldClose(window))
    {
        // Limpia la ventana utilizando el color indicado.
        glClearColor(0.12f, 0.07f, 0.17f, 1.0f);

        // Borra el contenido anterior de la pantalla.
        glClear(GL_COLOR_BUFFER_BIT);


        // Activa el programa de shaders.
        glUseProgram(shaderProgram);


        // Activa el VAO que contiene la configuración del cuadrado.
        glBindVertexArray(VAO);

        // DIBUJA EL CUADRADO
        // Utiliza los índices almacenados en el EBO.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Intercambia el búfer de dibujo con el que se muestra en pantalla.
        glfwSwapBuffers(window);

        // Revisa y procesa los eventos de la ventana.
        glfwPollEvents();
    }


    // Elimina el VAO.
    glDeleteVertexArrays(1, &VAO);

    // Elimina el VBO.
    glDeleteBuffers(1, &VBO);

    // Elimina el EBO.
    glDeleteBuffers(1, &EBO);

    // Elimina el programa de shaders.
    glDeleteProgram(shaderProgram);

    // Destruye la ventana.
    glfwDestroyWindow(window);

    // Finaliza GLFW.
    glfwTerminate();

    return 0;
}