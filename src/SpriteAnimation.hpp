#pragma once

#include <raylib.h>

#include <cstddef>
#include <string>
#include <vector>

class SpriteAnimation {
public:
    bool AddClip(std::string name, std::vector<Rectangle> frames,
                 float framesPerSecond = 8.0f, bool loop = true);
    bool Play(const std::string& name);
    void Update(float deltaTime);
    [[nodiscard]] bool HasFrames() const;
    [[nodiscard]] Rectangle GetSourceRectangle() const;

private:
    struct Clip {
        std::string name;
        std::vector<Rectangle> frames;
        float framesPerSecond = 8.0f;
        bool loop = true;
    };

    std::vector<Clip> clips_;
    std::size_t currentClip_ = 0;
    std::size_t currentFrame_ = 0;
    float elapsed_ = 0.0f;
    bool hasCurrentClip_ = false;
    bool playing_ = false;
};
