#include "SoundEngine.h"
#include <iostream>

SoundEngine::SoundEngine() 
    : masterVolume(100.0f), sfxVolume(100.0f), musicVolume(100.0f), currentMusic("") {
}

SoundEngine::~SoundEngine() {
    cleanup();
}

void SoundEngine::initialize() {
    std::cout << "Initializing Sound Engine..." << std::endl;
    
    // Load all game assets
    loadGameAssets();
    
    std::cout << "Sound Engine initialized successfully!" << std::endl;
}

bool SoundEngine::loadSound(const std::string& name, const std::string& filename) {
    auto soundBuffer = std::make_unique<sf::SoundBuffer>();
    
    if (!soundBuffer->loadFromFile(filename)) {
        std::cout << "Failed to load sound: " << filename << std::endl;
        return false;
    }
    
    // Create sound with the buffer directly to avoid default constructor issue
    auto sound = std::make_unique<sf::Sound>(*soundBuffer);
    
    soundBuffers[name] = std::move(soundBuffer);
    sounds[name] = std::move(sound);
    
    std::cout << "Successfully loaded sound: " << name << " from " << filename << std::endl;
    return true;
}

bool SoundEngine::loadSoundWithPaths(const std::string& name, const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        if (loadSound(name, path)) {
            return true;
        }
    }
    std::cout << "Failed to load sound: " << name << " from any provided path" << std::endl;
    return false;
}

bool SoundEngine::loadMusic(const std::string& name, const std::string& filename) {
    auto musicTrack = std::make_unique<sf::Music>();
    
    if (!musicTrack->openFromFile(filename)) {
        std::cout << "Failed to load music: " << filename << std::endl;
        return false;
    }
    
    music[name] = std::move(musicTrack);
    
    std::cout << "Successfully loaded music: " << name << " from " << filename << std::endl;
    return true;
}

bool SoundEngine::loadMusicWithPaths(const std::string& name, const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        if (loadMusic(name, path)) {
            return true;
        }
    }
    std::cout << "Failed to load music: " << name << " from any provided path" << std::endl;
    return false;
}

void SoundEngine::playSound(const std::string& name, bool loop) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        // it->second->setLoop(loop);
        it->second->setVolume(sfxVolume * (masterVolume / 100.0f));
        it->second->play();
    } else {
        std::cout << "Sound not found: " << name << std::endl;
    }
}

void SoundEngine::playMusic(const std::string& name, bool loop) {
    auto it = music.find(name);
    if (it != music.end()) {
        // it->second->setLoop(loop); // Commented due to SFML version compatibility
        it->second->setVolume(musicVolume * (masterVolume / 100.0f));
        it->second->play();
        currentMusic = name;
    } else {
        std::cout << "Music not found: " << name << std::endl;
    }
}

void SoundEngine::switchToMusic(const std::string& name) {
    // Stop current music if playing
    stopCurrentMusic();
    
    // Play new music
    playMusic(name, true);
}

void SoundEngine::stopCurrentMusic() {
    if (!currentMusic.empty()) {
        stopMusic(currentMusic);
        currentMusic = "";
    }
}

std::string SoundEngine::getCurrentMusic() const {
    return currentMusic;
}

void SoundEngine::stopSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->stop();
    }
}

void SoundEngine::stopMusic(const std::string& name) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->stop();
    }
}

void SoundEngine::stopAllSounds() {
    for (auto& pair : sounds) {
        pair.second->stop();
    }
}

void SoundEngine::stopAllMusic() {
    for (auto& pair : music) {
        pair.second->stop();
    }
}

void SoundEngine::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(100.0f, volume));
    
    // Update all currently playing sounds and music
    for (auto& pair : sounds) {
        pair.second->setVolume(sfxVolume * (masterVolume / 100.0f));
    }
    for (auto& pair : music) {
        pair.second->setVolume(musicVolume * (masterVolume / 100.0f));
    }
}

void SoundEngine::setSfxVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(100.0f, volume));
    
    // Update all currently playing sounds
    for (auto& pair : sounds) {
        pair.second->setVolume(sfxVolume * (masterVolume / 100.0f));
    }
}

void SoundEngine::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(100.0f, volume));
    
    // Update all currently playing music
    for (auto& pair : music) {
        pair.second->setVolume(musicVolume * (masterVolume / 100.0f));
    }
}

bool SoundEngine::isSoundPlaying(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        return it->second->getStatus() == sf::Sound::Status::Playing;
    }
    return false;
}

bool SoundEngine::isMusicPlaying(const std::string& name) {
    auto it = music.find(name);
    if (it != music.end()) {
        return it->second->getStatus() == sf::Music::Status::Playing;
    }
    return false;
}

void SoundEngine::loadGameAssets() {
    std::cout << "Loading game audio assets..." << std::endl;
    
    // Define common path patterns for assets
    std::vector<std::string> basePaths = {
        "resources/sounds/",
        "../resources/sounds/", 
        "../../resources/sounds/"
    };
    
    // Load Menu Music
    std::vector<std::string> menuMusicPaths;
    for (const auto& basePath : basePaths) {
        menuMusicPaths.push_back(basePath + "menu.wav");
        menuMusicPaths.push_back(basePath + "menu.ogg");
        menuMusicPaths.push_back(basePath + "menu.mp3");
    }
    loadMusicWithPaths("menu", menuMusicPaths);
    
    // Load Battle Music
    std::vector<std::string> battleMusicPaths;
    for (const auto& basePath : basePaths) {
        battleMusicPaths.push_back(basePath + "battle.wav");
        battleMusicPaths.push_back(basePath + "battle.ogg");
        battleMusicPaths.push_back(basePath + "battle.mp3");
    }
    loadMusicWithPaths("battle", battleMusicPaths);
    
    // Load Sound Effects
    // Footstep sounds
    std::vector<std::string> footstepPaths;
    for (const auto& basePath : basePaths) {
        footstepPaths.push_back(basePath + "footstep.wav");
    }
    loadSoundWithPaths("footstep", footstepPaths);

    // Footstep fast sounds
    std::vector<std::string> footstepFastPaths;
    for (const auto& basePath : basePaths) {
        footstepFastPaths.push_back(basePath + "footstep_fast.wav");
    }
    loadSoundWithPaths("footstep_fast", footstepFastPaths);
    
    // Sword attack sounds
    std::vector<std::string> swordPaths;
    for (const auto& basePath : basePaths) {
        swordPaths.push_back(basePath + "sword_hit.wav");
    }
    loadSoundWithPaths("sword_hit", swordPaths);
    
    // Block/Shield sounds
    std::vector<std::string> blockPaths;
    for (const auto& basePath : basePaths) {
        blockPaths.push_back(basePath + "block.wav");
    }
    loadSoundWithPaths("block", blockPaths);

    // Perfect guard/parry sound
    std::vector<std::string> parryPaths;
    for (const auto& basePath : basePaths) {
        parryPaths.push_back(basePath + "parry_3.wav");
    }
    loadSoundWithPaths("parry", parryPaths);
    
    // Dodge/Roll sounds
    std::vector<std::string> dodgePaths;
    for (const auto& basePath : basePaths) {
        dodgePaths.push_back(basePath + "dodge.wav");
    }
    loadSoundWithPaths("dodge", dodgePaths);

    // Got hit sounds
    std::vector<std::string> hitPaths;
    for (const auto& basePath : basePaths) {
        hitPaths.push_back(basePath + "got_hit.wav");
    }
    loadSoundWithPaths("got_hit", hitPaths);
    
    // Test sound (for your current T key test)
    std::vector<std::string> testPaths;
    for (const auto& basePath : basePaths) {
        testPaths.push_back(basePath + "test.wav");
    }
    loadSoundWithPaths("test", testPaths);
    
    // Set default volumes
    setMusicVolume(30.0f); // Menu music volume
    setSfxVolume(80.0f);   // Sound effects volume
    
    std::cout << "Finished loading game audio assets." << std::endl;
}

void SoundEngine::cleanup() {
    stopAllSounds();
    stopAllMusic();
    sounds.clear();
    music.clear();
    soundBuffers.clear();
    currentMusic = "";
}