#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"

const short MAP_BORDER = 10;     // This is used so the particles can not be on the edge of the screen for better visibility 
const float MAX_FORCE = 25;
const float WALL_REPEL_FORCE_MAX = 10;
const short WALL_REPEL_BOUND = MAP_BORDER+4;  // the wall starts repelling particles if they're closer than WALL_REPEL_BOUND pixels
#define RED 0
#define GREEN 1
#define YELLOW 2

class Particle {
	public:
	//glm vector -> better performance for graphics apperantly
	glm::vec2 position; 	// 2D vector representing the position (x, y)
	glm::vec2 velocity;		// 2D vector representing the velocity (vx, vy)
	int type;				// Type of the particle (for interaction rules)

	Particle(float x, float y, int color);

	void update(bool toggle);
	void draw();
	void apply_WallRepel(float force);
	void compute_Force(const Particle& acting_particle);
	ofFloatColor getColor() const;
};

class ParticleThread : public ofThread {
public:
    std::vector<Particle>* particles;
    int startIdx, endIdx, total_particles;
	float Wall_Repel_force;

    ParticleThread(std::vector<Particle>* particles, int start, int end, int total_particles, float Wall_Repel_force)
        : particles(particles), startIdx(start), endIdx(end), total_particles(total_particles), Wall_Repel_force(Wall_Repel_force) {}

	~ParticleThread() {
    	// ofLog() << "ParticleThread destructor called" << "!" <<this->getThreadId()<<"@";
	}
	
	void threadedFunction() {
		try
		{
			for (int i = startIdx; i < endIdx; i++) {
				for (int j = 0; j < total_particles; j++) {
					if (i != j) {
						(*particles)[i].compute_Force((*particles)[j]);
					}
				}
				// cerr << this->getThreadName() +  ": FLAG 1" << endl;
				(*particles)[i].apply_WallRepel(Wall_Repel_force);
				// cerr << this->getThreadName() +  ": FLAG 2" << endl;
			}
		}
		catch (const std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
		}
		catch (...) {
        std::cerr << "Unknown exception in thread." << std::endl;
    	}
		
	}
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);

		vector<Particle> all_particles;		// vector containing all particles;
		vector<glm::vec2> all_positions;	// this is needed for the vbo	
		vector<ofFloatColor> all_colors;  	// also for vbo
		
		void Create_particles();
		void initialize_forces(float min, float max);
		void initialize_color_force_range(short min, short max);
		void restart();
		void shuffle();
		void save_settings();
		void create_settings_dir();
		void load_settings(ofFile &file);
	
	ofxPanel gui;
	ofxGuiGroup RedSettings, GreenSettings, YellowSettings, SimSettings;
	ofxButton button_restart, button_shuffle, button_save_settings;
	ofxToggle toggle_reverse_velocity;
	ofxFloatSlider	sliderRR,sliderRG,sliderRY, sliderGR,sliderGG,sliderGY, sliderYR,sliderYG,sliderYY, 
					slider_viscosity, slider_wall_repel_force;
	ofxIntSlider slider_rangeRR, slider_rangeRG, slider_rangeRY,
				 slider_rangeGR, slider_rangeGG, slider_rangeGY, slider_rangeYR, slider_rangeYG, slider_rangeYY;
	ofxIntField field_n_particles;
	ofVbo vbo;								// more efficient batch drawing

	ofxDirDropdown dropdown;
	ofxInputField<string> field_get_name;	// input field to enter the name for saving the settings
	ofxLabel feedback;						// visual feedback for saving/loading settings
	// vector<string> presetFiles;

};
