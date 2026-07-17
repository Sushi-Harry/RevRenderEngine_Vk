#include "glfw_window.hpp"
#include <iostream>

Window* Window::create(const WindowProperties& prop){
    return new glfw_window(prop);
}

glfw_window::glfw_window(const WindowProperties& winProp){
    Init(winProp);
}

glfw_window::~glfw_window(){
    Shutdown();
}

void glfw_window::onUpdate() {
    glfwPollEvents();
}

void glfw_window::Shutdown() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void glfw_window::Init(const WindowProperties& props){
    _data.title = props._name;
    _data.width = props._width;
    _data.height = props._height;

    static bool glfwInitialized = false;
    if(!glfwInitialized){
        int init = glfwInit();
        if(!init){
            std::cout << "ERROR::GLFW_INIT_FAILED\n";
            return;
        }
        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
        });
        glfwInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow((int)_data.width, (int)_data.height, _data.title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(_window, &_data);

    glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height){
        WinData& data = *(WinData*)glfwGetWindowUserPointer(window);

        data.width = width;
        data.height = height;
        WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });

    glfwSetWindowCloseCallback(_window, [](GLFWwindow* window){
        WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent e;
        data.EventCallback(e);
    });
}