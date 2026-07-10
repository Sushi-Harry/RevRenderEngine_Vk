#pragma once

#include <cstdint>

class Renderer{
public:
    static void Init();
    static void Shutdown();

    static void onWindowResize(uint32_t width, uint32_t height);

    static void BeginScene();
    static void EndScene();
private:

};