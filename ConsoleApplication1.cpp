#include <iostream>                 // Permite mostrar mensajes en la consola usando std::cout.
#include <glad/glad.h>              // Incluye GLAD, que permite cargar las funciones de OpenGL.
#include <GLFW/glfw3.h>             // Incluye GLFW, que permite crear y manejar la ventana.
#include <cmath>                    // Permite utilizar funciones matemáticas, como sqrt() para calcular raíces cuadradas.

// VERTEX SHADER
// Guarda el código del Vertex Shader como un texto que OpenGL podrá interpretar.
const char* vertexShaderSource = "#version 330 core\n"

// Indica que recibiremos una posición llamada aPos.
// layout(location = 0) significa que esta información estará en la ubicación 0.
// vec3 significa que tendrá tres valores: x, y y z.
"layout (location = 0) in vec3 aPos;\n"

"void main()\n"// Función principal del Vertex Shader.
"{\n"// Abre el cuerpo de la función.

// Define la posición final del vértice en la pantalla.
// vec4 utiliza cuatro valores: x, y, z y w.
// El valor 1.0 corresponde a la coordenada homogénea w.
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";// Cierra la función main().

// FRAGMENT SHADER
// Guarda el código del Fragment Shader como un texto que OpenGL podrá interpretar.
const char* fragmentShaderSource = "#version 330 core\n"

// Crea una variable de salida llamada FragColor.
// Esta variable guardará el color final de cada fragmento o píxel.
"out vec4 FragColor;\n"

"void main()\n"// Función principal del Fragment Shader.
"{\n"// Abre el cuerpo de la función.

// Define el color final del fragmento.
// vec4 representa rojo, verde, azul y transparencia.
// Estos valores producirán un color naranja.
"   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"

"}\n\0";// Cierra la función main().

int main()                          // Función principal. El programa comienza a ejecutarse aquí.
{
    glfwInit(); // Inicializa GLFW para poder utilizar sus funciones.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);// Indica a GLFW que queremos utilizar la versión mayor 3 de OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Indica a GLFW que queremos utilizar la versión menor 3 de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Indica que queremos utilizar el perfil CORE de OpenGL.

    // Crea una ventana de 800 píxeles de ancho y 800 píxeles de alto.
    // El tercer dato es el nombre que aparecerá en la parte superior de la ventana.
    // NULL significa que la ventana no será de pantalla completa.
    // El último NULL significa que no compartirá recursos con otra ventana.
    GLFWwindow* window = glfwCreateWindow(800, 800, "YoutubeOpenGL", NULL, NULL);

    if (window == NULL)// Comprueba si la ventana no pudo crearse.
    {
        std::cout << "Failed to create GLFW window" << std::endl; // Muestra un mensaje de error en la consola.
        glfwTerminate();// Finaliza GLFW y libera sus recursos.

        return -1;// Termina el programa indicando que ocurrió un error.
    }

    glfwMakeContextCurrent(window);// Establece esta ventana como el contexto actual de OpenGL.
    gladLoadGL();// Carga las funciones de OpenGL utilizando GLAD.

    // Define el área de la ventana donde OpenGL puede dibujar.
    // Empieza en x = 0 y y = 0.
    // Tiene 800 píxeles de ancho y 800 píxeles de alto.
    glViewport(0, 0, 800, 800);

    // CREACIÓN Y COMPILACIÓN DEL VERTEX SHADER
    // Crea un objeto para guardar el Vertex Shader.
    // GL_VERTEX_SHADER indica que este shader se encargará de los vértices.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Asigna nuestro código fuente del Vertex Shader al objeto que acabamos de crear.
    // vertexShader es el shader que recibirá el código.
    // 1 indica que estamos enviando una sola cadena de texto.
    // &vertexShaderSource indica la dirección donde está guardado el código.
    // NULL significa que OpenGL calculará automáticamente la longitud del texto.
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader); // Compila el código del Vertex Shader para que OpenGL pueda utilizarlo.

    // CREACIÓN Y COMPILACIÓN DEL FRAGMENT SHADER
    // Crea un objeto para guardar el Fragment Shader.
    // GL_FRAGMENT_SHADER indica que este shader se encargará de colorear los fragmentos.
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Asigna nuestro código fuente del Fragment Shader al objeto que acabamos de crear.
    glCompileShader(fragmentShader);// Compila el código del Fragment Shader para que OpenGL pueda utilizarlo.

    // CREACIÓN DEL SHADER PROGRAM
    // Crea un programa de shaders.
    // Este programa reunirá los diferentes shaders que vamos a utilizar.
    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader); // Conecta el Vertex Shader al programa.

    glAttachShader(shaderProgram, fragmentShader);// Conecta el Fragment Shader al programa.

    glLinkProgram(shaderProgram);  // Une o enlaza todos los shaders que pertenecen al programa.

    // Una vez que los shaders ya fueron añadidos y enlazados al programa,
    // podemos eliminar los objetos individuales del Vertex Shader.
    glDeleteShader(vertexShader);

    
    glDeleteShader(fragmentShader);// Elimina el objeto individual del Fragment Shader.

    // COORDENADAS DE LOS VÉRTICES DEL TRIÁNGULO
    // Crea un arreglo llamado vertices.
    // Cada grupo de tres números representa un punto: x, y, z.
    GLfloat vertices[] =
    {
        -0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,// Primer vértice: esquina inferior izquierda.
         0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,  // Segundo vértice: esquina inferior derecha.
         0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f     // Tercer vértice: parte superior.
    };

    // VAO Y VBO
    // Crea las variables que guardarán las referencias del VAO y del VBO.
    GLuint VAO, VBO;

    // Genera un objeto VAO.
    // El VAO guarda la información sobre cómo OpenGL debe interpretar los vértices.
    glGenVertexArrays(1, &VAO);

    // Genera un objeto VBO.
    // El VBO guardará los datos de las coordenadas de nuestros vértices.
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);// Activa o selecciona nuestro VAO para trabajar con él.

    // Activa nuestro VBO como un buffer de tipo GL_ARRAY_BUFFER.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envía los datos del arreglo vertices desde la CPU hacia el VBO.
    // sizeof(vertices) indica el tamaño total de los datos.
    // vertices es el arreglo que contiene las coordenadas.
    // GL_STATIC_DRAW indica que los datos no cambiarán constantemente.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Le explica a OpenGL cómo están organizados los datos dentro del VBO.
    // 0: ubicación que corresponde al location = 0 del Vertex Shader.
    // 3: cada vértice tiene tres valores: x, y y z.
    // GL_FLOAT: cada valor es de tipo float.
    // GL_FALSE: no necesitamos normalizar los datos.
    // 3 * sizeof(float): distancia entre un vértice y el siguiente.
    // (void*)0: el primer dato comienza desde el inicio del arreglo.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Activa el atributo de vértice ubicado en la posición 0.
    // Esto permite que OpenGL utilice esos datos.
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);// Desactiva el VBO para evitar modificarlo accidentalmente.

    glBindVertexArray(0);// Desactiva el VAO para evitar modificarlo accidentalmente.

    // BUCLE PRINCIPAL
    // Mantiene el programa funcionando mientras el usuario no cierre la ventana.
    while (!glfwWindowShouldClose(window))
    {
        // Define el color del fondo.
        // Los valores representan rojo, verde, azul y transparencia.
        glClearColor(0.12f, 0.07f, 0.17f, 1.0f);

      
        glClear(GL_COLOR_BUFFER_BIT);  // Limpia la pantalla utilizando el color de fondo seleccionado.
        glUseProgram(shaderProgram);// Indica que utilizaremos nuestro programa de shaders.
        glBindVertexArray(VAO);// Activa el VAO que contiene la información de nuestros vértices.

        // Dibuja las figuras utilizando triángulos.
        // 0 indica desde qué vértice comenzar.
        // 3 indica que utilizaremos tres vértices.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);// Muestra en pantalla lo que fue dibujado.

        // Revisa y procesa los eventos de GLFW,
        // como el teclado, mouse o cierre de la ventana.
        glfwPollEvents();
    }
    // LIMPIEZA Y FINALIZACIÓN DEL PROGRAMA
    // Elimina el VAO que creamos.
    glDeleteVertexArrays(1, &VAO);

    glDeleteBuffers(1, &VBO);// Elimina el VBO que creamos.
    glDeleteProgram(shaderProgram);// Elimina el programa de shaders que creamos.

    glfwDestroyWindow(window);// Destruye la ventana.

    glfwTerminate();// Finaliza GLFW y libera los recursos utilizados.
 
    return 0; // Indica que el programa terminó correctamente.
}