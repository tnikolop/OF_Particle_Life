#pragma once

#include "ofMain.h"

class Particle {
	public:
	//glm vector -> better performance for graphics
	glm::vec2 position; 	// 2D vector representing the position (x, y)
	glm::vec2 velocity;		// 2D vector representing the velocity (vx, vy)
	ofColor color;			// Type of the particle (for interaction rules)

	Particle(float x, float y, ofColor color);
	// ~Particle();

	void update();
	void draw();
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

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
		short FORCE_RANGE = 200;
		short number_of_particles = 450;    // per type (color)
		short total_particles = number_of_particles*NUM_TYPES;

		std::array<ofColor,3> color_types = {
			ofColor::red,	// 0
			ofColor::green,	// 1
			ofColor::yellow	// 2
		};

		vector<Particle> particles;		// vector containing all particles;
		
		void Create_particles();
};
