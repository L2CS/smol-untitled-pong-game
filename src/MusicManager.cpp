#include "MusicManager.h"
#include "OsuParser.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Define the consecutive hit timeout (1 second)
const float MusicManager::CONSECUTIVE_HIT_TIMEOUT = 1.0f;

MusicManager::MusicManager()
    : audioFileSampleRate(44100)
    , audioFileChannels(2)
    , musicDuration(180.0f)
    , visualizerTime(0.0f)
    , songStartTime(0.0f)
    , currentAudioPosition(0)
    , bufferIndex(0)
    , lastBeatIndex(0)
    , currentBeatIntensity(0.0f)
    , lastBeatTime(0.0f)
    , soundsLoaded(false)
    , musicPlaying(false)
    , lastHitWasPaddle(false)
    , lastPaddleHitTime(0.0f)
    , fftw_input(nullptr)
    , fftw_output(nullptr)
    , fftw_plan_forward(nullptr)

{
    // Initialize FFTW3 buffers and plan
    audioBuffer.resize(FFT_SIZE, 0.0f);
    fftw_input = (double*)fftw_malloc(sizeof(double) * FFT_SIZE);
    fftw_output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (FFT_SIZE / 2 + 1));
    fftw_plan_forward = fftw_plan_dft_r2c_1d(FFT_SIZE, fftw_input, fftw_output, FFTW_ESTIMATE);
    magnitudes.resize(FFT_SIZE / 2, 0.0f);
}

MusicManager::~MusicManager()
{
    cleanup();
}

void MusicManager::cleanup()
{
    // Clean up Raylib audio resources
    if (soundsLoaded) {
        if (musicPlaying) {
            StopMusicStream(backgroundMusic);
            UnloadMusicStream(backgroundMusic);
        }
        UnloadSound(hitSound);
        soundsLoaded = false;
        musicPlaying = false;
    }
    
    // Clean up FFTW3 resources
    if (fftw_plan_forward) {
        fftw_destroy_plan(fftw_plan_forward);
        fftw_plan_forward = nullptr;
    }
    if (fftw_input) {
        fftw_free(fftw_input);
        fftw_input = nullptr;
    }
    if (fftw_output) {
        fftw_free(fftw_output);
        fftw_output = nullptr;
    }
    fftw_cleanup();
}

bool MusicManager::initializeSounds()
{
    // Initialize Raylib audio device
    if (!IsAudioDeviceReady()) {
        printf("Initializing Raylib audio device...\n");
        InitAudioDevice();
    }
    
    printf("Using Raylib audio for sound playback\n");
    
    // Load hit sound
    hitSound = LoadSound("resources/audio/hit1.wav");
    if (hitSound.frameCount == 0) {
        printf("Failed to load hit sound: resources/audio/hit1.wav\n");
        printf("Hit sound will be disabled\n");
    } else {
        printf("Hit sound loaded successfully: resources/audio/hit1.wav\n");
    }
    
    soundsLoaded = true;
    return true;
}

bool MusicManager::loadAudioFile(const char* filename)
{
    printf("Loading audio file: %s\n", filename);
    
    Wave wave = LoadWave(filename);
    
    if (wave.data == nullptr) {
        printf("Failed to load audio file: %s\n", filename);
        return false;
    }
    
    printf("Audio file loaded successfully:\n");
    printf("  Sample rate: %d Hz\n", wave.sampleRate);
    printf("  Channels: %d\n", wave.channels);
    printf("  Sample size: %d bits\n", wave.sampleSize);
    printf("  Frame count: %d\n", wave.frameCount);
    
    audioFileSampleRate = wave.sampleRate;
    audioFileChannels = wave.channels;
    
    // Convert audio data to float and store
    audioFileData.clear();
    audioFileData.reserve(wave.frameCount * wave.channels);
    
    if (wave.sampleSize == 16) {
        short* samples = (short*)wave.data;
        for (int i = 0; i < wave.frameCount * wave.channels; i++) {
            audioFileData.push_back(samples[i] / 32768.0f);
        }
    } else if (wave.sampleSize == 32) {
        float* samples = (float*)wave.data;
        for (int i = 0; i < wave.frameCount * wave.channels; i++) {
            audioFileData.push_back(samples[i]);
        }
    }
    
    musicDuration = (float)wave.frameCount / wave.sampleRate;
    printf("Music duration: %.1f seconds\n", musicDuration);
    
    UnloadWave(wave);
    printf("Audio data converted to float: %zu samples\n", audioFileData.size());
    
    // Load beat map after audio is loaded
    loadBeatMap();
    
    return true;
}

void MusicManager::setSongStartTime()
{
    songStartTime = visualizerTime;
    lastBeatIndex = 0;
    printf("Song started at time: %.2f\n", songStartTime);
}

void MusicManager::update(float deltaTime)
{
    visualizerTime += deltaTime;
    
    // Update music stream if playing
    if (musicPlaying && soundsLoaded) {
        UpdateMusicStream(backgroundMusic);
    }
    
    // Get real audio data from the loaded file
    getRealAudioData();
    
    // Perform FFT analysis
    performFFT();
    
    // Calculate beat intensity
    currentBeatIntensity = calculateBeatIntensity();
}

void MusicManager::performFFT()
{
    // Copy audio buffer to FFTW3 input buffer
    for (int i = 0; i < FFT_SIZE; i++) {
        fftw_input[i] = (double)audioBuffer[i];
    }
    
    // Perform FFT using FFTW3
    fftw_execute(fftw_plan_forward);
    
    // Calculate magnitudes from FFTW3 output
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        double real = fftw_output[i][0];
        double imag = fftw_output[i][1];
        magnitudes[i] = (float)(sqrt(real * real + imag * imag) / FFT_SIZE);
    }
}

void MusicManager::getRealAudioData()
{
    if (audioFileData.empty()) {
        // Fallback to silence if no audio data
        for (int i = 0; i < FFT_SIZE; i++) {
            audioBuffer[i] = 0.0f;
        }
        return;
    }
    
    // Calculate current position in audio file based on song time
    float currentSongTime = visualizerTime - songStartTime;
    int targetPosition = (int)(currentSongTime * audioFileSampleRate) * audioFileChannels;
    
    // Clamp to valid range
    if (targetPosition < 0) targetPosition = 0;
    if (targetPosition >= (int)audioFileData.size()) {
        targetPosition = audioFileData.size() - FFT_SIZE * audioFileChannels;
    }
    
    currentAudioPosition = targetPosition;
    
    // Extract audio samples for FFT
    for (int i = 0; i < FFT_SIZE; i++) {
        int sampleIndex = currentAudioPosition + i * audioFileChannels;
        
        if (sampleIndex < (int)audioFileData.size()) {
            if (audioFileChannels == 1) {
                // Mono
                audioBuffer[i] = audioFileData[sampleIndex];
            } else {
                // Stereo - mix both channels
                float left = audioFileData[sampleIndex];
                float right = (sampleIndex + 1 < (int)audioFileData.size()) ? 
                             audioFileData[sampleIndex + 1] : 0.0f;
                audioBuffer[i] = (left + right) * 0.5f;
            }
        } else {
            audioBuffer[i] = 0.0f;
        }
    }
}

float MusicManager::calculateBeatIntensity()
{
    if (magnitudes.empty()) return 0.0f;
    
    // Focus on bass and low-mid frequencies for beat detection (20-200 Hz)
    float bassIntensity = 0.0f;
    int bassStart = (int)(20.0f * FFT_SIZE / SAMPLE_RATE);
    int bassEnd = (int)(200.0f * FFT_SIZE / SAMPLE_RATE);
    
    bassStart = fmaxf(1, fminf(bassStart, FFT_SIZE / 2 - 1));
    bassEnd = fmaxf(bassStart + 1, fminf(bassEnd, FFT_SIZE / 2));
    
    // Calculate average bass intensity
    for (int i = bassStart; i < bassEnd; i++) {
        bassIntensity += magnitudes[i];
    }
    bassIntensity /= (bassEnd - bassStart);
    
    // Also include some mid frequencies for snare/percussion (200-2000 Hz)
    float midIntensity = 0.0f;
    int midStart = (int)(200.0f * FFT_SIZE / SAMPLE_RATE);
    int midEnd = (int)(2000.0f * FFT_SIZE / SAMPLE_RATE);
    
    midStart = fmaxf(1, fminf(midStart, FFT_SIZE / 2 - 1));
    midEnd = fmaxf(midStart + 1, fminf(midEnd, FFT_SIZE / 2));
    
    for (int i = midStart; i < midEnd; i++) {
        midIntensity += magnitudes[i];
    }
    midIntensity /= (midEnd - midStart);
    
    // Combine bass and mid with bass weighted more heavily
    float totalIntensity = bassIntensity * 0.7f + midIntensity * 0.3f;
    
    // Apply logarithmic scaling and normalize
    float beatIntensity = logf(1.0f + totalIntensity * 1000.0f) / 10.0f;
    
    // Clamp to reasonable range
    beatIntensity = fmaxf(0.0f, fminf(beatIntensity, 2.0f));
    
    return beatIntensity;
}

void MusicManager::loadBeatMap()
{
    // Fallback timing data if no osu! file is provided
    float firstBeatTime = 316.0f / 1000.0f; // Convert ms to seconds
    float beatLength = 315.789473684211f / 1000.0f; // Convert ms to seconds
    float bpm = 60.0f / beatLength; // Calculate actual BPM (~190 BPM)
    
    printf("Using fallback timing: First beat at %.3fs, BPM: %.1f\n", firstBeatTime, bpm);
    
    // Generate beats based on fallback timing for the song duration
    for (float time = firstBeatTime; time < musicDuration; time += beatLength) {
        beatTimes.push_back(time);
        
        // Add quarter beats for more responsive visualizer
        beatTimes.push_back(time + beatLength * 0.25f);
        beatTimes.push_back(time + beatLength * 0.5f);
        beatTimes.push_back(time + beatLength * 0.75f);
    }
    
    // Sort the beat times
    std::sort(beatTimes.begin(), beatTimes.end());
    
    printf("Loaded %zu beats for music sync (fallback timing)\n", beatTimes.size());
}

void MusicManager::loadBeatMapFromOsu(const std::vector<TimingPoint>& timingPoints)
{
    beatTimes.clear();
    
    if (timingPoints.empty()) {
        printf("No timing points provided, using fallback timing\n");
        loadBeatMap();
        return;
    }
    
    // Find the first non-inherited timing point (main BPM)
    const TimingPoint* mainTiming = nullptr;
    for (const auto& tp : timingPoints) {
        if (!tp.inherited && tp.beatLength > 0) {
            mainTiming = &tp;
            break;
        }
    }
    
    if (!mainTiming) {
        printf("No valid main timing point found, using fallback timing\n");
        loadBeatMap();
        return;
    }
    
    float firstBeatTime = mainTiming->time / 1000.0f; // Convert ms to seconds
    float beatLength = mainTiming->beatLength / 1000.0f; // Convert ms to seconds
    float bpm = 60.0f / beatLength;
    
    printf("Using osu! timing: First beat at %.3fs, Beat length: %.3fs, BPM: %.1f\n", 
           firstBeatTime, beatLength, bpm);
    
    // Process all timing points to handle BPM changes
    for (const auto& tp : timingPoints) {
        if (!tp.inherited && tp.beatLength > 0) {
            // This is a new BPM section
            float sectionStart = tp.time / 1000.0f;
            float sectionBeatLength = tp.beatLength / 1000.0f;
            
            // Find the end of this section (next timing point or end of song)
            float sectionEnd = musicDuration;
            for (const auto& nextTp : timingPoints) {
                if (!nextTp.inherited && nextTp.time > tp.time && nextTp.beatLength > 0) {
                    sectionEnd = nextTp.time / 1000.0f;
                    break;
                }
            }
            
            // Generate beats for this section
            for (float time = sectionStart; time < sectionEnd; time += sectionBeatLength) {
                beatTimes.push_back(time);
                
                // Add quarter beats for more responsive visualizer
                if (time + sectionBeatLength * 0.25f < sectionEnd) {
                    beatTimes.push_back(time + sectionBeatLength * 0.25f);
                }
                if (time + sectionBeatLength * 0.5f < sectionEnd) {
                    beatTimes.push_back(time + sectionBeatLength * 0.5f);
                }
                if (time + sectionBeatLength * 0.75f < sectionEnd) {
                    beatTimes.push_back(time + sectionBeatLength * 0.75f);
                }
            }
        }
    }
    
    // Sort the beat times
    std::sort(beatTimes.begin(), beatTimes.end());
    
    printf("Loaded %zu beats from osu! timing points\n", beatTimes.size());
}

bool MusicManager::isGameEnded() const
{
    // Check if music has finished playing
    if (musicPlaying && soundsLoaded) {
        return !IsMusicStreamPlaying(backgroundMusic);
    }
    
    // Fallback to time-based check
    float currentSongTime = visualizerTime - songStartTime;
    return currentSongTime >= musicDuration;
}

void MusicManager::startBackgroundMusic(const char* filename)
{
    if (!soundsLoaded) return;
    
    // Load and start background music
    backgroundMusic = LoadMusicStream(filename);
    if (backgroundMusic.frameCount == 0) {
        printf("Failed to load background music: %s\n", filename);
        return;
    }
    
    // Set volume to 30% for comfortable listening
    SetMusicVolume(backgroundMusic, 0.3f);
    
    // Start playing
    PlayMusicStream(backgroundMusic);
    musicPlaying = true;
    printf("Background music started: %s at 30%% volume using Raylib\n", filename);
}

void MusicManager::playHitSound()
{
    if (soundsLoaded && hitSound.frameCount > 0) {
        PlaySound(hitSound);
    }
}

void MusicManager::onPaddleHit()
{
    float currentTime = visualizerTime;
    
    // Check if this is a consecutive paddle hit
    if (lastHitWasPaddle && (currentTime - lastPaddleHitTime) <= CONSECUTIVE_HIT_TIMEOUT) {
        // Consecutive paddle hit - play the same sound
        printf("Consecutive paddle hit detected! Playing hit sound again.\n");
        playHitSound();
    } else {
        // First paddle hit or timeout exceeded - play normal hit sound
        printf("Paddle hit detected! Playing hit sound.\n");
        playHitSound();
    }
    
    // Update tracking variables
    lastHitWasPaddle = true;
    lastPaddleHitTime = currentTime;
}

void MusicManager::onNonPaddleHit()
{
    // Reset paddle hit tracking when ball hits something else
    lastHitWasPaddle = false;
}