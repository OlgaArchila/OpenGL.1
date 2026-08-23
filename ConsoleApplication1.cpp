#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main()
{
    // Inicializar GLFW
    if (!glfwInit())
    {
        std::cout << "Error al iniciar GLFW" << std::endl;
        return -1;
    }

    // Crear la ventana
    GLFWwindow* ventana = glfwCreateWindow(
        800,
        600,
        "Mi primera ventana OpenGL",
        nullptr,
        nullptr
    );

    if (ventana == nullptr)
    {
        std::cout << "Error al crear la ventana" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Seleccionar la ventana como contexto de OpenGL
    glfwMakeContextCurrent(ventana);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error al iniciar GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "GLFW funciona correctamente" << std::endl;
    std::cout << "GLAD funciona correctamente" << std::endl;

    std::cout << "Version de OpenGL: "
        << glGetString(GL_VERSION)
        << std::endl;

    // Mantener abierta la ventana
    while (!glfwWindowShouldClose(ventana))
    {
        glfwSwapBuffers(ventana);
        glfwPollEvents();
    }

    // Finalizar GLFW
    glfwTerminate();

    return 0;
}