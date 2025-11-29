#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

class SoundEngine {
private:
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> sounds;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> music;
    
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    
    std::string currentMusic; // Track currently playing music

public:
    SoundEngine();
    ~SoundEngine();
    
    // Initialization
    void initialize();
    
    // Sound loading and management
    bool loadSound(const std::string& name, const std::string& filename);
    bool loadSoundWithPaths(const std::string& name, const std::vector<std::string>& paths);
    bool loadMusic(const std::string& name, const std::string& filename);
    bool loadMusicWithPaths(const std::string& name, const std::vector<std::string>& paths);
    
    // Sound playback
    void playSound(const std::string& name, bool loop = false);
    void playMusic(const std::string& name, bool loop = true);
    
    // Music state management
    void switchToMusic(const std::string& name);
    void stopCurrentMusic();
    std::string getCurrentMusic() const;
    
    // Sound control
    void stopSound(const std::string& name);
    void stopMusic(const std::string& name);
    void stopAllSounds();
    void stopAllMusic();
    
    // Volume control
    void setMasterVolume(float volume); // 0.0f to 100.0f
    void setSfxVolume(float volume);
    void setMusicVolume(float volume);
    
    // Utility functions
    bool isSoundPlaying(const std::string& name);
    bool isMusicPlaying(const std::string& name);
    
    // Game-specific methods
    void loadGameAssets();
    
    // Cleanup
    void cleanup();
};

#endif