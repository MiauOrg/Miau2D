#include "SpriteAnimation.hpp"

#include <cmath>
#include <utility>

bool SpriteAnimation::AddClip(std::string name, std::vector<Rectangle> frames,
                              float framesPerSecond, bool loop) {
    if (name.empty() || frames.empty() || !std::isfinite(framesPerSecond) || framesPerSecond <= 0.0f) {
        return false;
    }

    for (std::size_t index = 0; index < clips_.size(); ++index) {
        if (clips_[index].name == name) {
            clips_[index].frames = std::move(frames);
            clips_[index].framesPerSecond = framesPerSecond;
            clips_[index].loop = loop;
            if (hasCurrentClip_ && currentClip_ == index) {
                currentFrame_ = 0;
                elapsed_ = 0.0f;
                playing_ = true;
            }
            return true;
        }
    }

    clips_.push_back({ std::move(name), std::move(frames), framesPerSecond, loop });
    if (!hasCurrentClip_) {
        currentClip_ = 0;
        currentFrame_ = 0;
        hasCurrentClip_ = true;
        playing_ = true;
    }
    return true;
}

bool SpriteAnimation::Play(const std::string& name) {
    for (std::size_t index = 0; index < clips_.size(); ++index) {
        if (clips_[index].name != name) {
            continue;
        }
        if (!hasCurrentClip_ || currentClip_ != index) {
            currentClip_ = index;
            currentFrame_ = 0;
            elapsed_ = 0.0f;
        }
        hasCurrentClip_ = true;
        playing_ = true;
        return true;
    }
    return false;
}

void SpriteAnimation::Update(float deltaTime) {
    if (!hasCurrentClip_ || !playing_ || !std::isfinite(deltaTime) || deltaTime <= 0.0f) {
        return;
    }

    const Clip& clip = clips_[currentClip_];
    if (clip.frames.size() <= 1) {
        return;
    }

    const float frameDuration = 1.0f / clip.framesPerSecond;
    elapsed_ += deltaTime;
    const std::size_t steps = static_cast<std::size_t>(elapsed_ / frameDuration);
    if (steps == 0) {
        return;
    }
    elapsed_ -= static_cast<float>(steps) * frameDuration;

    if (clip.loop) {
        currentFrame_ = (currentFrame_ + steps) % clip.frames.size();
        return;
    }

    const std::size_t lastFrame = clip.frames.size() - 1;
    if (steps >= lastFrame - currentFrame_) {
        currentFrame_ = lastFrame;
        playing_ = false;
    } else {
        currentFrame_ += steps;
    }
}

bool SpriteAnimation::HasFrames() const {
    return hasCurrentClip_ && currentClip_ < clips_.size() && !clips_[currentClip_].frames.empty();
}

Rectangle SpriteAnimation::GetSourceRectangle() const {
    if (!HasFrames()) {
        return {};
    }
    return clips_[currentClip_].frames[currentFrame_];
}
