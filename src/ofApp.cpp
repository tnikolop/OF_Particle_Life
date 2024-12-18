#include "ofApp.h"

short FORCE_RANGE = 200;
short number_of_particles = 450;    // per type (color)
short total_particles = number_of_particles*NUM_TYPES;
float force_matrix[NUM_TYPES][NUM_TYPES]{{0}};

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0,0,0);
    Create_particles();
    initialize_forces(-MAX_FORCE,MAX_FORCE);

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
for (int i = 0; i < particles.size(); i++)
    {
        particles[i].draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

Particle::Particle(float x, float y, int color) {
    position = glm::vec2(x,y);          // Initialize position with input coordinates
    velocity = glm::vec2(0.0f,0.0f);    // Start with zero velocity
    type = color;                     // Assign the particle type
}

void Particle::update(){}
void Particle::apply_WallRepel(){}
void Particle::draw() {
    ofColor color;
    if (this->type == RED)
        color = ofColor::red;
    else if (this->type == GREEN)
        color = ofColor::green;
    else if (this->type == YELLOW)
        color = ofColor::yellow;

    ofSetColor(color);
    ofDrawCircle(position,1);
}

// Calculate the forces that act on this specific particle 
// based on another particle
void Particle::compute_Force(const Particle& acting_particle){
    glm::vec2 direction = acting_particle.position - this->position;
    // distance2 = square distance for less computation time
    float distance2 = glm::distance2(this->position,acting_particle.position);
    float force_strength=0;     // if out of range dont apply any force

    // Avoid division by zero
    if (distance2 > 0 && distance2 < FORCE_RANGE*FORCE_RANGE) 
        force_strength = force_matrix[this->type][acting_particle.type];
    // direction /= distance;  // Normalize the direction giati mas noiazei mono to direction tou vector oxi to magnitude tou
    // isws na xreiazetai na dieresw me distance^2 gia normalization alla den nomizw
    
}

// Creates a specifc number of every particle type and adds them to the vector of particles
// Every particle is initialized with random positions
void ofApp::Create_particles(){
    for (int i = 0; i < number_of_particles; i++)
    {
        for (int j=0; j<NUM_TYPES; j++)
        {
            Particle newParticle(ofRandom(MAP_WIDTH),ofRandom(MAP_HEIGHT),j);
            particles.push_back(newParticle);
        }
    }       
}

void ofApp::initialize_forces(float min, float max){
    for (int i = 0; i < NUM_TYPES; i++)
    {
        for (int j = 0; j < NUM_TYPES; j++)
        {
            force_matrix[i][j] = ofRandom(min,max);
        }
    }    
}

