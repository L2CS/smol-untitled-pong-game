#include "VideoManager.h"
#include <cstring>
#include <iostream>

extern "C" {
#include <libavutil/imgutils.h>
}

VideoManager::VideoManager()
    : formatContext(nullptr), codecContext(nullptr), codec(nullptr), frame(nullptr), rgbFrame(nullptr), swsContext(nullptr), packet(nullptr), videoStreamIndex(-1), width(0), height(0), frameRate(0.0), duration(0.0), timeBase(0.0), currentFrameTime(0.0), nextFrameTime(0.0), loaded(false), playing(false), currentTime(0.0f), lastFrameTime(0.0f), lastMusicTime(0.0f), currentTexture({ 0 }), currentImage({ 0 }), textureNeedsUpdate(false), rgbBuffer(nullptr), rgbBufferSize(0)
{}

VideoManager::~VideoManager()
{
    cleanup();
}

bool VideoManager::loadVideo(const std::string& filename)
{
    cleanup();

    // Open video file
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Error: Could not open video file: " << filename << std::endl;
        return false;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Error: Could not find stream information" << std::endl;
        cleanup();
        return false;
    }

    // Find video stream
    videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "Error: No video stream found" << std::endl;
        cleanup();
        return false;
    }

    // Get codec parameters
    AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;

    // Find decoder
    codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Error: Codec not found" << std::endl;
        cleanup();
        return false;
    }

    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "Error: Could not allocate codec context" << std::endl;
        cleanup();
        return false;
    }

    // Copy codec parameters to context
    if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
        std::cerr << "Error: Could not copy codec parameters" << std::endl;
        cleanup();
        return false;
    }

    // Open codec
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Error: Could not open codec" << std::endl;
        cleanup();
        return false;
    }

    // Get video properties
    width = codecContext->width;
    height = codecContext->height;

    AVRational timeBaseRational = formatContext->streams[videoStreamIndex]->time_base;
    timeBase = av_q2d(timeBaseRational);
    AVRational frameRateRational = formatContext->streams[videoStreamIndex]->avg_frame_rate;
    frameRate = av_q2d(frameRateRational);
    duration = formatContext->duration / (double)AV_TIME_BASE;

    // Initialize frame timing
    currentFrameTime = 0.0;
    nextFrameTime = 0.0;

    std::cout << "Video loaded: " << width << "x" << height
              << " @ " << frameRate << " fps, duration: " << duration << "s" << std::endl;

    // Allocate frames
    frame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
    if (!frame || !rgbFrame) {
        std::cerr << "Error: Could not allocate frames" << std::endl;
        cleanup();
        return false;
    }

    // Allocate packet
    packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Error: Could not allocate packet" << std::endl;
        cleanup();
        return false;
    }

    rgbBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    rgbBuffer = (uint8_t*)av_malloc(rgbBufferSize);
    if (!rgbBuffer) {
        std::cerr << "Error: Could not allocate RGB buffer" << std::endl;
        cleanup();
        return false;
    }

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbBuffer, AV_PIX_FMT_RGBA, width, height, 1);

    // Initialize scaling context
    swsContext = sws_getContext(
        width, height, codecContext->pix_fmt,
        width, height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!swsContext) {
        std::cerr << "Error: Could not initialize scaling context" << std::endl;
        cleanup();
        return false;
    }

    // Create initial Raylib image and texture
    currentImage = {
        .data = rgbBuffer,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    currentTexture = LoadTextureFromImage(currentImage);

    loaded = true;
    currentTime = 0.0f;
    lastFrameTime = 0.0f;
    lastMusicTime = 0.0f;

    return true;
}

void VideoManager::update(float musicTime)
{
    if (!loaded || !playing) return;

    // Sync video time to music time
    syncToMusicTime(musicTime);

    // Decode frame for current time
    if (decodeFrameForTime(currentTime)) {
        convertFrameToTexture();
        lastFrameTime = currentTime;
    }
}

void VideoManager::updateBasic(float deltaTime)
{
    if (!loaded || !playing) return;

    currentTime += deltaTime;

    // Check if we need to decode the next frame based on frame rate
    float targetFrameTime = 1.0f / frameRate;
    if (currentTime - lastFrameTime >= targetFrameTime) {
        if (decodeNextFrame()) {
            convertFrameToTexture();
            lastFrameTime = currentTime;
        }
        else {
            // End of video - loop back to beginning
            restart();
        }
    }
}

bool VideoManager::decodeNextFrame()
{
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            int ret = avcodec_send_packet(codecContext, packet);
            if (ret < 0) {
                av_packet_unref(packet);
                continue;
            }

            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == 0) {
                // Update frame timing
                if (frame->pts != AV_NOPTS_VALUE) {
                    currentFrameTime = frame->pts * timeBase;
                    nextFrameTime = currentFrameTime + (1.0 / frameRate);
                }
                av_packet_unref(packet);
                return true;
            }
            else if (ret == AVERROR(EAGAIN)) {
                av_packet_unref(packet);
                continue;
            }
            else {
                av_packet_unref(packet);
                return false;
            }
        }
        av_packet_unref(packet);
    }
    return false;
}

bool VideoManager::decodeFrameForTime(float targetTime)
{
    // If we already have the right frame, don't decode
    if (targetTime >= currentFrameTime && targetTime < nextFrameTime) {
        return true; // Current frame is correct
    }

    // If target time is before current frame, we need to seek
    if (targetTime < currentFrameTime) {
        // Seek to a bit before target time to ensure we get the right frame
        float seekTime = fmaxf(0.0f, targetTime - 1.0f);
        int64_t seekTarget = (int64_t)(seekTime / timeBase);

        if (av_seek_frame(formatContext, videoStreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) >= 0) {
            avcodec_flush_buffers(codecContext);
            currentFrameTime = 0.0;
            nextFrameTime = 0.0;
        }
    }

    // Decode frames until we reach the target time
    while (decodeNextFrame()) {
        if (targetTime >= currentFrameTime && targetTime < nextFrameTime) {
            return true;
        }
        if (currentFrameTime > targetTime + 1.0) {
            // gg you are gone
            break;
        }
    }

    return false;
}

void VideoManager::convertFrameToTexture()
{
    if (!frame || !swsContext) return;

    // Convert frame to RGBA
    sws_scale(
        swsContext,
        frame->data, frame->linesize,
        0, height,
        rgbFrame->data, rgbFrame->linesize);

    // Update texture
    UpdateTexture(currentTexture, rgbBuffer);
}

Texture2D VideoManager::getCurrentFrame()
{
    return currentTexture;
}

bool VideoManager::isLoaded() const
{
    return loaded;
}

bool VideoManager::isPlaying() const
{
    return playing;
}

void VideoManager::play()
{
    if (loaded) {
        playing = true;
    }
}

void VideoManager::pause()
{
    playing = false;
}

void VideoManager::stop()
{
    playing = false;
    currentTime = 0.0f;
    lastFrameTime = 0.0f;
    lastMusicTime = 0.0f;
}

void VideoManager::restart()
{
    if (!loaded) return;

    // Seek to beginning
    av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecContext);

    currentTime = 0.0f;
    lastFrameTime = 0.0f;
    lastMusicTime = 0.0f;
    currentFrameTime = 0.0;
    nextFrameTime = 0.0;
}

float VideoManager::getDuration() const
{
    return duration;
}

float VideoManager::getCurrentTime() const
{
    return currentTime;
}

void VideoManager::setCurrentTime(float time)
{
    if (!loaded) return;

    int64_t timestamp = (int64_t)(time / timeBase);
    av_seek_frame(formatContext, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecContext);

    currentTime = time;
    lastFrameTime = time;
    currentFrameTime = 0.0;
    nextFrameTime = 0.0;
}

void VideoManager::syncToMusicTime(float musicTime)
{
    if (!loaded) return;

    float timeDifference = musicTime - currentTime;
    float absDifference = fabsf(timeDifference);

    if (absDifference > 0.5f) {
        std::cout << "Video sync: major seek from " << currentTime << "s to " << musicTime << "s (diff: " << timeDifference << "s)" << std::endl;
        setCurrentTime(musicTime);
    }
    else {
        currentTime = musicTime;
    }

    lastMusicTime = musicTime;
}

void VideoManager::cleanup()
{
    if (currentTexture.id != 0) {
        UnloadTexture(currentTexture);
        currentTexture = { 0 };
    }

    if (rgbBuffer) {
        av_free(rgbBuffer);
        rgbBuffer = nullptr;
    }

    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }

    if (packet) {
        av_packet_free(&packet);
    }

    if (rgbFrame) {
        av_frame_free(&rgbFrame);
    }

    if (frame) {
        av_frame_free(&frame);
    }

    if (codecContext) {
        avcodec_free_context(&codecContext);
    }

    if (formatContext) {
        avformat_close_input(&formatContext);
    }

    loaded = false;
    playing = false;
    width = height = 0;
    frameRate = duration = timeBase = 0.0;
    currentFrameTime = nextFrameTime = 0.0;
    currentTime = lastFrameTime = lastMusicTime = 0.0f;
    videoStreamIndex = -1;
    rgbBufferSize = 0;
}