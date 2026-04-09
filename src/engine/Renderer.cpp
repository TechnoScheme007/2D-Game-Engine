#include "engine/Renderer.h"
#include <iostream>

namespace Engine {

// Simple 5x7 bitmap font data for basic text rendering
static const uint8_t FONT_DATA[][7] = {
    // Space (32)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // ! (33)
    {0x04,0x04,0x04,0x04,0x00,0x04,0x00},
    // " - # - $ - % - & - ' - ( - ) - * - + - , - - - . - /
    {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},
    {0x0A,0x1F,0x0A,0x1F,0x0A,0x00,0x00},
    {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
    {0x18,0x19,0x02,0x04,0x08,0x13,0x03},
    {0x08,0x14,0x14,0x08,0x15,0x12,0x0D},
    {0x04,0x04,0x00,0x00,0x00,0x00,0x00},
    {0x02,0x04,0x04,0x04,0x04,0x04,0x02},
    {0x08,0x04,0x04,0x04,0x04,0x04,0x08},
    {0x00,0x0A,0x04,0x1F,0x04,0x0A,0x00},
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    {0x00,0x00,0x00,0x00,0x04,0x04,0x08},
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x04,0x00},
    {0x00,0x01,0x02,0x04,0x08,0x10,0x00},
    // 0-9 (48-57)
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
    {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    // : ; < = > ? @
    {0x00,0x04,0x00,0x00,0x04,0x00,0x00},
    {0x00,0x04,0x00,0x00,0x04,0x04,0x08},
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02},
    {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
    {0x08,0x04,0x02,0x01,0x02,0x04,0x08},
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E},
    // A-Z (65-90)
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    {0x01,0x01,0x01,0x01,0x11,0x11,0x0E},
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

static int charToIndex(char c) {
    if (c >= 'a' && c <= 'z') c -= 32; // uppercase
    if (c >= 32 && c <= 64) return c - 32;
    if (c >= 'A' && c <= 'Z') return c - 65 + 33;
    return 0; // space for unknown
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::init(const std::string& title, int width, int height, bool fullscreen) {
    width_ = width;
    height_ = height;

    Uint32 flags = SDL_WINDOW_SHOWN;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, flags);
    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    return true;
}

void Renderer::shutdown() {
    for (auto& [id, tex] : textures_) {
        SDL_DestroyTexture(tex);
    }
    textures_.clear();

    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
}

void Renderer::clear(const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer_);
}

void Renderer::present() {
    SDL_RenderPresent(renderer_);
}

SDL_Texture* Renderer::loadTexture(const std::string& id, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255)); // magenta transparency
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        textures_[id] = texture;
    }
    return texture;
}

SDL_Texture* Renderer::createSolidTexture(const std::string& id, int w, int h, const Color& color) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surface) return nullptr;
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        textures_[id] = texture;
    }
    return texture;
}

SDL_Texture* Renderer::getTexture(const std::string& id) const {
    auto it = textures_.find(id);
    return it != textures_.end() ? it->second : nullptr;
}

void Renderer::drawTexture(const std::string& textureId, const SDL_Rect& src, const SDL_Rect& dst,
                           float angle, SDL_RendererFlip flip, const Color& tint) {
    SDL_Texture* tex = getTexture(textureId);
    if (!tex) return;

    SDL_Rect screenDst = {
        dst.x - static_cast<int>(cameraPos_.x),
        dst.y - static_cast<int>(cameraPos_.y),
        dst.w, dst.h
    };

    SDL_SetTextureColorMod(tex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(tex, tint.a);

    const SDL_Rect* srcPtr = (src.w > 0 && src.h > 0) ? &src : nullptr;
    SDL_RenderCopyEx(renderer_, tex, srcPtr, &screenDst, angle, nullptr, flip);
}

void Renderer::drawRect(const Rect& rect, const Color& color, bool filled) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect r = {
        static_cast<int>(rect.x - cameraPos_.x),
        static_cast<int>(rect.y - cameraPos_.y),
        static_cast<int>(rect.w),
        static_cast<int>(rect.h)
    };
    if (filled)
        SDL_RenderFillRect(renderer_, &r);
    else
        SDL_RenderDrawRect(renderer_, &r);
}

void Renderer::drawLine(const Vec2& a, const Vec2& b, const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer_,
        static_cast<int>(a.x - cameraPos_.x), static_cast<int>(a.y - cameraPos_.y),
        static_cast<int>(b.x - cameraPos_.x), static_cast<int>(b.y - cameraPos_.y));
}

void Renderer::drawPoint(const Vec2& p, const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer_, static_cast<int>(p.x - cameraPos_.x),
                        static_cast<int>(p.y - cameraPos_.y));
}

void Renderer::drawCircle(const Vec2& center, float radius, const Color& color, bool filled) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    int cx = static_cast<int>(center.x - cameraPos_.x);
    int cy = static_cast<int>(center.y - cameraPos_.y);
    int r = static_cast<int>(radius);

    if (filled) {
        for (int dy = -r; dy <= r; dy++) {
            int dx = static_cast<int>(std::sqrt(r * r - dy * dy));
            SDL_RenderDrawLine(renderer_, cx - dx, cy + dy, cx + dx, cy + dy);
        }
    } else {
        int x = r, y = 0, err = 0;
        while (x >= y) {
            SDL_RenderDrawPoint(renderer_, cx + x, cy + y);
            SDL_RenderDrawPoint(renderer_, cx + y, cy + x);
            SDL_RenderDrawPoint(renderer_, cx - y, cy + x);
            SDL_RenderDrawPoint(renderer_, cx - x, cy + y);
            SDL_RenderDrawPoint(renderer_, cx - x, cy - y);
            SDL_RenderDrawPoint(renderer_, cx - y, cy - x);
            SDL_RenderDrawPoint(renderer_, cx + y, cy - x);
            SDL_RenderDrawPoint(renderer_, cx + x, cy - y);
            y++;
            err += 1 + 2 * y;
            if (2 * (err - x) + 1 > 0) { x--; err += 1 - 2 * x; }
        }
    }
}

void Renderer::drawText(const std::string& text, const Vec2& pos, const Color& color, int scale) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    int cursorX = static_cast<int>(pos.x);
    int cursorY = static_cast<int>(pos.y);

    for (char c : text) {
        if (c == '\n') {
            cursorX = static_cast<int>(pos.x);
            cursorY += 8 * scale;
            continue;
        }
        int idx = charToIndex(c);
        if (idx >= 0 && idx < static_cast<int>(sizeof(FONT_DATA) / sizeof(FONT_DATA[0]))) {
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (FONT_DATA[idx][row] & (1 << (4 - col))) {
                        if (scale == 1) {
                            SDL_RenderDrawPoint(renderer_, cursorX + col, cursorY + row);
                        } else {
                            SDL_Rect pixel = {cursorX + col * scale, cursorY + row * scale, scale, scale};
                            SDL_RenderFillRect(renderer_, &pixel);
                        }
                    }
                }
            }
        }
        cursorX += 6 * scale;
    }
}

} // namespace Engine
