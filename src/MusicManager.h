#pragma once

#include <raylib.h>
#include <fftw3.h>
#include <vector>
#include <string>

// Forward declaration to avoid circular dependency
struct TimingPoint;

class MusicManager {
public:
    // Audio processing constants
    static const int FFT_SIZE = 1024;
    static const int SAMPLE_RATE = 44100;
    
    // Constructor and destructor
    MusicManager();
    ~MusicManager();
    
    // Core functionality
    bool loadAudioFile(const char* filename);
    void setSongStartTime();
    void update(float deltaTime);
    void loadBeatMapFromOsu(const std::vector<struct TimingPoint>& timingPoints);
    
    // Sound effects
    bool initializeSounds();
    void playHitSound();
    void onPaddleHit(); // Called when ball hits any paddle
    void onNonPaddleHit(); // Called when ball hits wall/boundary
    
    // Getters
    float getCurrentBeatIntensity() const { return currentBeatIntensity; }
    float getMusicDuration() const { return musicDuration; }
    float getCurrentSongTime() const { return visualizerTime - songStartTime; }
    bool isGameEnded() const;
    const std::vector<float>& getMagnitudes() const { return magnitudes; }
    
    // Static constants access
    static int getFFTSize() { return FFT_SIZE; }
    static int getSampleRate() { return SAMPLE_RATE; }

private:
    // Private methods
    void performFFT();
    void getRealAudioData();
    float calculateBeatIntensity();
    void loadBeatMap();
    void cleanup();
    
    // Audio file data
    std::vector<float> audioFileData;
    int audioFileSampleRate;
    int audioFileChannels;
    float musicDuration;
    
    // Timing
    float visualizerTime;
    float songStartTime;
    
    // FFT analysis
    std::vector<float> audioBuffer;
    std::vector<float> magnitudes;
    double* fftw_input;
    fftw_complex* fftw_output;
    fftw_plan fftw_plan_forward;
    int currentAudioPosition;
    int bufferIndex;
    
    // Beat detection
    std::vector<float> beatTimes;
    int lastBeatIndex;
    float currentBeatIntensity;
    float lastBeatTime;
    
    bool soundsLoaded;
    
    // Hit tracking for consecutive hits
    bool lastHitWasPaddle;
    float lastPaddleHitTime;
    static const float CONSECUTIVE_HIT_TIMEOUT; // Time window for consecutive hits
};