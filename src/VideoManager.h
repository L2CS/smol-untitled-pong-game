#pragma once

#include "raylib.h"
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoManager {
public:
    VideoManager();
    ~VideoManager();

    bool loadVideo(const std::string& filename);
    void update(float musicTime);
    void updateBasic(float deltaTime);
    Texture2D getCurrentFrame();
    bool isLoaded() const;
    bool isPlaying() const;
    void play();
    void pause();
    void stop();
    void restart();
    float getDuration() const;
    float getCurrentTime() const;
    void setCurrentTime(float time);
    void syncToMusicTime(float musicTime);
    float getSyncDifference() const
    {
        return lastMusicTime - currentTime;
    }

private:
    void cleanup();
    bool decodeNextFrame();
    bool decodeFrameForTime(float targetTime);
    void convertFrameToTexture();

    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    const AVCodec* codec;
    AVFrame* frame;
    AVFrame* rgbFrame;
    SwsContext* swsContext;
    AVPacket* packet;

    // Video properties
    int videoStreamIndex;
    int width;
    int height;
    double frameRate;
    double duration;
    double timeBase;

    // Frame timing
    double currentFrameTime;
    double nextFrameTime;

    // Playback state
    bool loaded;
    bool playing;
    float currentTime;
    float lastFrameTime;
    float lastMusicTime;

    // Raylib texture
    Texture2D currentTexture;
    Image currentImage;
    bool textureNeedsUpdate;

    // Frame buffer for RGB conversion
    uint8_t* rgbBuffer;
    int rgbBufferSize;
};
