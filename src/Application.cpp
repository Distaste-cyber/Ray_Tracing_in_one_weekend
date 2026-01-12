

#include <GL/eglew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

#include "IndexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "Textures.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"

int main(void) {
  GLFWwindow *window;

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK)
    std::cout << "Error!" << "endl";
  {
    float positions[] = {100.0f, 100.0f, 0.0f,   0.0f,   200.0f, 100.0f,
                         1.0f,   0.0f,   200.0f, 200.0f, 1.0f,   1.0f,
                         100.0f, 200.0f, 0.0f,   1.0f

    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    VertexArray va;
    VertexBuffer vb(positions, 4 * 4 * sizeof(float));

    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);

    va.AddBuffer(vb, layout);

    IndexBuffer ib(indices, 6);

    glm::mat4 proj = glm::ortho(0.0f, 640.0f, 480.0f, 0.0f, -1.0f, 1.0f);
    glm::mat4 view =
        glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, 0.0f, 0.0f));

    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 200.0f, 0.0f));

    glm::mat4 mvp = proj * view * model;

    Shader shader("res/shaders/Basic.shader");
    shader.Bind();
    shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);
    shader.SetUniformMat("u_MVP", mvp);

    Textures texture("res/textures/cherno.png");
    texture.Bind();
    shader.SetUniform1i("u_Texture", 0);

    va.Unbind();

    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    Renderer renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    // Initialize backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float r = 0.0f;
    float increment = 0.05f;

    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
      /* Render here */
      renderer.Clear();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      shader.Bind();
      shader.SetUniform4f("u_Color", r, g, b, a);

      ImGui::Begin("Demo Window");
      ImGui::Text("Hello from ImGui!");
      ImGui::End();

      renderer.Draw(va, ib, shader);

      if (r > 1.0f)
        increment = -0.05f;
      else if (r < 0.0f)
        increment = 0.05f;

      r += increment;
      g += increment;
      b += increment;
      a += increment;

      ImGui::Render();

      // Clear screen (you can set any color)
      glClear(GL_COLOR_BUFFER_BIT);

      // Render ImGui on top of your OpenGL scene
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // Swap
      glfwSwapBuffers(window);

      /* Poll for and process events */
      glfwPollEvents();
    }
  }
  glfwTerminate();
  return 0;
}