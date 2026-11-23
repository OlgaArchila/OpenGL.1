🎨 La primera pincelada – My Paint
1. Información del proyecto
Estudiante: Olga Lucía Guacaneme Archila
Código: 6014476
Lenguaje: C++
Tecnología gráfica: OpenGL 3.3
2. Descripción del proyecto
La primera pincelada – My Paint es una aplicación de dibujo digital desarrollada en C++ utilizando OpenGL y Dear ImGui. El programa cuenta con un lienzo interactivo compuesto por una cuadrícula de 80 × 80 celdas, en el que el usuario puede realizar dibujos libres y crear diferentes figuras geométricas.

Uno de los aspectos principales del proyecto es la implementación del algoritmo de Bresenham para el trazado de líneas y circunferencias, que permite representar estas figuras mediante puntos enteros sobre la cuadrícula. También se implementa el algoritmo Flood Fill para rellenar regiones, además de herramientas de dibujo, borrado, selección de colores, rotación y deshacer acciones.
3. Objetivo
Desarrollar una aplicación de dibujo que permita aplicar conceptos de programación en C++, geometría y computación gráfica mediante herramientas interactivas y algoritmos de rasterización.
4. Tecnologías utilizadas
- C++: desarrollo de la lógica del programa.
- OpenGL: representación gráfica del lienzo.
- GLFW: creación y administración de la ventana.
- GLAD: carga de las funciones de OpenGL.
- Dear ImGui: creación de la interfaz gráfica.
- GLSL: programación de los shaders.
5. Arquitectura del proyecto
El programa está organizado en diferentes secciones:
- Inicialización: configuración de GLFW, GLAD y OpenGL.
- Cuadrícula: construcción del lienzo de 80 × 80 celdas.
- Buffers: utilización de VAO, VBO y EBO para organizar los vértices e índices.
- Shaders: procesamiento de posiciones y colores mediante Vertex Shader y Fragment Shader.
- Herramientas: funciones para dibujar y modificar las figuras.
- Interfaz: controles y botones desarrollados con Dear ImGui.
6. Algoritmos implementados
- Bresenham para líneas: permite dibujar líneas seleccionando las celdas más cercanas a la trayectoria.
- Bresenham para círculos: permite representar circunferencias aprovechando la simetría de sus puntos.
- Flood Fill: permite rellenar regiones conectadas que comparten un mismo color.
- Rotación: utiliza operaciones trigonométricas para transformar las coordenadas de una figura.
7. Funcionalidades
- Dibujo libre con lápiz.
- Trazado de líneas.
- Dibujo de rectángulos, cuadrados y triángulos.
- Dibujo de circunferencias.
- Relleno de figuras.
- Borrador y selección de colores.
- Rotación de figuras.
- Limpiar el lienzo y deshacer acciones.
8. Conclusión
Este proyecto permite aplicar conocimientos de C++, OpenGL y computación gráfica mediante el desarrollo de una herramienta de dibujo interactiva. La implementación de los algoritmos de Bresenham y Flood Fill permite comprender y aplicar técnicas de rasterización y procesamiento de figuras sobre una cuadrícula.
9. Evidencias
Video de presentación https://drive.google.com/file/d/1gYGIkHHAMdxs-4yMIxNiipm4tETG3QOm/view?usp=sharing

Desarrollado por: Olga Lucía Guacaneme Archila
Código: 6014476
