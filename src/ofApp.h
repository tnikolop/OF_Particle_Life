#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <ParticleThread.h>

// Constant Variables
const short MAP_BORDER = 5;
const short MAP_WIDTH = 800 + MAP_BORDER;
const short MAP_HEIGHT = 800 + MAP_BORDER;
const short ATOM_WIDTH = 1;
const float MAX_FORCE = 25;
const short WALL_REPEL_BOUND = MAP_BORDER+4;  // the wall starts repelling particles if they're closer than WALL_REPEL_BOUND pixels
const float WALL_REPEL_FORCE = 0.1;
const int NUM_TYPES = 3;    // Number of different particle types
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
	// ~Particle();

	void update();
	void draw();
	void apply_WallRepel();
	void compute_Force(const Particle& acting_particle);
	ofFloatColor getColor() const;
};

class ParticleThread : public ofThread {
public:
    std::vector<Particle>* particles;
    int startIdx, endIdx, total_particles;

    ParticleThread(std::vector<Particle>* particles, int start, int end, int total_particles)
        : particles(particles), startIdx(start), endIdx(end), total_particles(total_particles) {}

	void threadedFunction() {
		for (int i = startIdx; i < endIdx; i++) {
			for (int j = 0; j < total_particles; j++) {
				if (i != j) {
					(*particles)[i].compute_Force((*particles)[j]);
				}
			}
			(*particles)[i].apply_WallRepel();
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
		void restart();
		void shuffle();
	
	ofxPanel gui;
	ofxButton button_restart, button_shuffle;
	ofxFloatSlider	sliderRR,sliderRG,sliderRY, sliderGR,sliderGG,sliderGY, sliderYR,sliderYG,sliderYY, slider_viscosity;
	ofxIntSlider slider_force_range;
	ofxIntField field_n_particles;
	ofVbo vbo;								// more efficient batch drawing
};
