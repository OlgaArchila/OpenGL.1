#include <iostream>                 // Permite mostrar mensajes en la consola usando std::cout.
#include <glad/glad.h>              // Incluye GLAD, que permite cargar las funciones de OpenGL.
#include <GLFW/glfw3.h>             // Incluye GLFW, que permite crear y manejar la ventana.

int main()                          // Función principal: el programa comienza a ejecutarse aquí.
{
    glfwInit();                     // Inicializa GLFW para poder utilizar sus funciones.

    // Indica a GLFW que queremos utilizar la versión mayor 3 de OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    // Indica a GLFW que queremos utilizar la versión menor 3 de OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Indica que queremos utilizar el perfil CORE de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // CORE significa que trabajaremos con las funciones modernas de OpenGL.

    // Crea una ventana de 800 píxeles de ancho y 800 píxeles de alto.
    // El tercer dato es el título que aparecerá en la parte superior de la ventana.
    // NULL significa que no utilizamos pantalla completa.
    // NULL significa que no compartimos recursos con otra ventana.
    GLFWwindow* window = glfwCreateWindow(800, 800, "OlgaOpenGL", NULL, NULL);

    // Comprueba si la ventana fue creada correctamente.
    if (window == NULL)
    {
        // Muestra este mensaje en la consola si la ventana no se pudo crear.
        std::cout << "Failed to create GLFW window" << std::endl;

        // Finaliza GLFW y libera los recursos que estaba utilizando.
        glfwTerminate();

        // Finaliza el programa indicando que ocurrió un error.
        return -1;
    }


    // Establece esta ventana como el contexto actual de OpenGL.
    // Esto permite que las siguientes instrucciones de OpenGL trabajen sobre esta ventana.
    glfwMakeContextCurrent(window);


    // Carga las funciones de OpenGL utilizando GLAD.
    // Sin esta línea no podríamos utilizar correctamente muchas funciones de OpenGL.
    gladLoadGL();

    // Define el área de la ventana donde OpenGL puede dibujar.
    // Los primeros dos valores indican dónde comienza: x = 0 y y = 0.
    // Los últimos dos valores indican el tamaño: 800 píxeles de ancho y 800 de alto.
    glViewport(0, 0, 800, 800);

    // Define el color de fondo que tendrá la ventana.
    // Los valores representan: rojo, verde, azul y transparencia.
    // La letra f indica que los valores son números de tipo float.
    glClearColor(0.10f, 0.08f, 0.17f, 1.0f);

    // Limpia el búfer de color utilizando el color que acabamos de configurar.
    glClear(GL_COLOR_BUFFER_BIT);

    // Muestra en pantalla lo que se encuentra en el búfer trasero.
    glfwSwapBuffers(window);

    // Mantiene el programa funcionando mientras el usuario no cierre la ventana.
    while (!glfwWindowShouldClose(window))
    {
        // Revisa y procesa los eventos de GLFW.
        // Por ejemplo: movimientos del mouse, teclado o cerrar la ventana.
        glfwPollEvents();
    }

    // Destruye la ventana antes de finalizar el programa.
    glfwDestroyWindow(window);

    // Finaliza GLFW y libera los recursos utilizados.
    glfwTerminate();

    // Indica que el programa terminó correctamente.
    return 0;
}