// 1. definir les chemins vers les includes
// 2. definir les chemins vers les libraries
// 3. configurer en fonction de la plateforme (optionnel)

// glew doit toujours etre le premier include OpenGL
// comme on link en static, il faut le specifier 
#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"
#include <GLFW/glfw3.h>

#include "Application.h"

int main(void)
{
    Application app;

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "ShadowPBR", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    app.setWindow(window);
    app.setSize(1280, 720);
    app.initialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        app.setSize(width, height);

        app.setElapsedTime(glfwGetTime());

        app.update();
        /* Render here */
        app.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    app.deinitialize();

    glfwTerminate();
    return 0;
}
