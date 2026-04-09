#pragma once
#include "Math.h"
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine {

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool init(const std::string& title, int width, int height, bool fullscreen = false);
    void shutdown();

    void clear(const Color& color = {20, 20, 30, 255});
    void present();

    // Texture management
    SDL_Texture* loadTexture(const std::string& id, const std::string& path);
    SDL_Texture* createSolidTexture(const std::string& id, int w, int h, const Color& color);
    SDL_Texture* getTexture(const std::string& id) const;

    // Drawing primitives
    void drawTexture(const std::string& textureId, const SDL_Rect& src, const SDL_Rect& dst,
                     float angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE,
                     const Color& tint = {255, 255, 255, 255});
    void drawRect(const Rect& rect, const Color& color, bool filled = false);
    void drawLine(const Vec2& a, const Vec2& b, const Color& color);
    void drawPoint(const Vec2& p, const Color& color);
    void drawCircle(const Vec2& center, float radius, const Color& color, bool filled = false);

    // Text rendering (simple bitmap-style using SDL primitives)
    void drawText(const std::string& text, const Vec2& pos, const Color& color, int scale = 1);

    // Camera
    void setCamera(const Vec2& pos) { cameraPos_ = pos; }
    Vec2 getCamera() const { return cameraPos_; }
    Vec2 screenToWorld(const Vec2& screen) const { return screen + cameraPos_; }
    Vec2 worldToScreen(const Vec2& world) const { return world - cameraPos_; }

    SDL_Renderer* getSDLRenderer() const { return renderer_; }
    SDL_Window* getSDLWindow() const { return window_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> textures_;
    Vec2 cameraPos_;
    int width_ = 800;
    int height_ = 600;
};

} // namespace Engine
