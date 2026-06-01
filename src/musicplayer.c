#include "raylib.h"
#include "musicplayer.h"
#include <stdio.h>

static Music bgMusic = {0};
static bool musicLoaded = false;

int InitBackgroundMusic(const char* musicPath) {
    bgMusic = LoadMusicStream(musicPath);
    
    printf("Loading music from: %s\n", musicPath);
    printf("Music frameCount: %d\n", bgMusic.frameCount);
    
    if (bgMusic.frameCount > 0) {
        musicLoaded = true;
        bgMusic.looping = true;
        PlayMusicStream(bgMusic);
        SetMusicVolume(bgMusic, 0.3f);
        printf("Music loaded and playing!\n");
        return 1;
    }
    
    printf("Failed to load music!\n");
    musicLoaded = false;
    return 0;
}

void UpdateBackgroundMusic(void) {
    if (musicLoaded) {
        UpdateMusicStream(bgMusic);
    }
}

void SetBackgroundMusicVolume(float volume) {
    if (musicLoaded && volume >= 0.0f && volume <= 1.0f) {
        SetMusicVolume(bgMusic, volume);
    }
}

void PauseBackgroundMusic(void) {
    if (musicLoaded) {
        PauseMusicStream(bgMusic);
    }
}

void ResumeBackgroundMusic(void) {
    if (musicLoaded) {
        ResumeMusicStream(bgMusic);
    }
}

void CloseBackgroundMusic(void) {
    if (musicLoaded) {
        UnloadMusicStream(bgMusic);
        musicLoaded = false;
    }
}