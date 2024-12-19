#include "ofApp.h"

short FORCE_RANGE = 200;
short number_of_particles = 450;    // per type (color)
short total_particles = number_of_particles*NUM_TYPES;
float force_matrix[NUM_TYPES][NUM_TYPES]{{0}};

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0,0,0);    // Black Background Color
    Create_particles();
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    
    gui.setup("Settings");
    gui.setPosition(MAP_WIDTH+70,20);
    gui.add(button_restart.setup("RESTART (R)"));
    gui.add(button_shuffle.setup("SHUFFLE"));

    gui.add(slider_force_range.setup("FORCE RANGE",FORCE_RANGE,0,FORCE_RANGE));

    gui.add(sliderRR.setup("RED TO RED",force_matrix[RED][RED],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderRG.setup("RED TO GREEN",force_matrix[RED][GREEN],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderRY.setup("RED TO YELLOW",force_matrix[RED][YELLOW],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderGR.setup("GREEN TO RED",force_matrix[GREEN][RED],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderGG.setup("GREEN TO GREEN",force_matrix[GREEN][GREEN],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderGY.setup("GREEN TO YELLOW",force_matrix[GREEN][YELLOW],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderYR.setup("YELLOW TO RED",force_matrix[YELLOW][RED],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderYG.setup("YELLOW TO GREEN",force_matrix[YELLOW][GREEN],-MAX_FORCE,MAX_FORCE));
    gui.add(sliderYY.setup("YELLOW TO YELLOW",force_matrix[YELLOW][YELLOW],-MAX_FORCE,MAX_FORCE));
}

//--------------------------------------------------------------
void ofApp::update(){
    for (int i = 0; i < total_particles; i++)
    {
        for (int j = 0; j < total_particles; j++)
        {
            if (i!= j) {
                particles[i].compute_Force(particles[j]);
            }
        }
        particles[i].apply_WallRepel();
    }
    for (auto& particle : particles) {
            particle.update();
        }
}

//--------------------------------------------------------------
void ofApp::draw(){
    gui.draw();

    for (int i = 0; i < particles.size(); i++)
        {
            particles[i].draw();
        }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //restart
    particles.clear();
    Create_particles();
}

// Particle constructor
Particle::Particle(float x, float y, int color) {
    position = glm::vec2(x,y);          // Initialize position with input coordinates
    velocity = glm::vec2(0.0f,0.0f);    // Start with zero velocity
    type = color;                     // Assign the particle type
}

// Update particles position based on its updated velocity
void Particle::update(){
    position += velocity;  // Add velocity to position to move the particle
    
    // the particles must always be on screen
    if (position.x > MAP_WIDTH)
        position.x = MAP_WIDTH - 1;
    else if (position.x < MAP_BORDER)
        position.x = MAP_BORDER;
    
    if (position.y > MAP_HEIGHT)
        position.y = MAP_HEIGHT - 1;
    else if (position.y < MAP_BORDER)
        position.y = MAP_BORDER;
}

// Force that repells the particles from the edge of the map
// so they do not stay there
void Particle::apply_WallRepel(){
    velocity.x += position.x < WALL_REPEL_BOUND ? (WALL_REPEL_BOUND - position.x) * WALL_REPEL_FORCE : 0.0F;
    velocity.y += position.y < WALL_REPEL_BOUND ? (WALL_REPEL_BOUND - position.y) * WALL_REPEL_FORCE : 0.0F;
    velocity.x += position.x > MAP_WIDTH - WALL_REPEL_BOUND ? (MAP_WIDTH - WALL_REPEL_BOUND - position.x) * WALL_REPEL_FORCE : 0.0F;
    velocity.y += position.y > MAP_HEIGHT - WALL_REPEL_BOUND ? (MAP_HEIGHT - WALL_REPEL_BOUND - position.y) * WALL_REPEL_FORCE : 0.0F;
}

// Draw particle on screen
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
        force_strength = force_matrix[this->type][acting_particle.type] / distance2;
    // direction /= distance;  // Normalize the direction giati mas noiazei mono to direction tou vector oxi to magnitude tou
    // isws na xreiazetai na dieresw me distance^2 gia normalization alla den nomizw
    
    const float dt = 0.99;  //ousiastika einai kati san tribi
    this->velocity = (this->velocity+force_strength * direction) *dt;
    // to dt na koitaksw
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

// Initialize with random values the forces of interaction between each particle type
void ofApp::initialize_forces(float min, float max){
    for (int i = 0; i < NUM_TYPES; i++)
    {
        for (int j = 0; j < NUM_TYPES; j++)
        {
            force_matrix[i][j] = ofRandom(min,max);
        }
    }    
}

