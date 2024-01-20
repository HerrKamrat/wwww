#pragma once

#include "renderer.hpp"

class App {
  public:
    void start();
    void update();

  private:
    Renderer renderer;
};
