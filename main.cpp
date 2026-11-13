#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

struct Components{

    std :: string name, category;
    double mass, powerGeneration, powerConsumption, cost, dataGenerationRate, dataTransmissionRate, dataStorageCapacity;

};

class spaceCraft{
    private:
    std :: vector<Components> components;

    public:
    void addComponent(const Components& component);
    void removeComponent(int index);
    double getTotalMass();
    double getTotalCost();
    double getPowerConsumption();
    double getPowerGeneration();
    double getDataTransmission();
    double getDataGeneration();
    double getStorage();

};

struct missionConfig{

    std :: string objective, orbit, launchVehicle;
    int duration;
    spaceCraft spaceCraft;

};

struct missionConstraints{

    double maxBudget, maxMass, minDataTransmissionRate;
    int minDuration, maxDuration;

};

int main(){

    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    };
    float WIDTH = 800.0f, HEIGHT = 600.0f;
    SDL_Window* window = SDL_CreateWindow("Space Shooter", WIDTH, HEIGHT, 0);
    
    if (!window) {
       SDL_Log("Window creation failed: %s", SDL_GetError());
       SDL_Quit();
       return 1;
   }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
 
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;

    while(running){

        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;

}