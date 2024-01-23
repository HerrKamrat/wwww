#pragma once

#include "renderer.hpp"

class App {
  public:
    void start();
    void update();
    void render();

  private:
    Renderer renderer;
};
