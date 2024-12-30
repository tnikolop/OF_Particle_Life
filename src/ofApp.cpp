#include "ofApp.h"

short FORCE_RANGE = 200;
short number_of_particles = 650;    // per type (color)
short total_particles = number_of_particles*NUM_TYPES;
float force_matrix[NUM_TYPES][NUM_TYPES]{{0}};
float viscosity = 0;

//--------------------------------------------------------------
void ofApp::setup(){

//  #pragma omp parallel
//     {
//         int thread_id = omp_get_thread_num();
//         int num_threads = omp_get_num_threads();
//         std::cout << "Hello from thread " << thread_id << " of " << num_threads << std::endl;
//     }

    ofSetBackgroundColor(0,0,0);    // Black Background Color
    Create_particles();
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    

    //========================= CREATE GUI =========================================
    gui.setup("Settings");
    gui.setPosition(MAP_WIDTH+70,20);
    gui.add(button_restart.setup("RESTART (R)"));
    button_restart.addListener(this,&ofApp::restart);
    gui.add(button_shuffle.setup("SHUFFLE (S)"));
    button_shuffle.addListener(this,&ofApp::shuffle);

    gui.add(slider_force_range.setup("FORCE RANGE",FORCE_RANGE,0,FORCE_RANGE));
    gui.add(field_n_particles.setup("PARTICLES PER COLOR",number_of_particles,1,1000));
    gui.add(slider_viscosity.setup("VISCOSITY",0.002F,0.0F,0.5F));

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

    // isws na mhn einai kai o kalyteros tropos
    FORCE_RANGE = slider_force_range;
    number_of_particles = field_n_particles;
    viscosity = slider_viscosity;

    force_matrix[RED][RED] = sliderRR;
    force_matrix[RED][GREEN] = sliderRG;
    force_matrix[RED][YELLOW] = sliderRY;
    force_matrix[GREEN][RED] = sliderGR;
    force_matrix[GREEN][GREEN] = sliderGG;
    force_matrix[GREEN][YELLOW] = sliderGY;
    force_matrix[YELLOW][RED] = sliderYR;
    force_matrix[YELLOW][GREEN] = sliderYG;
    force_matrix[YELLOW][YELLOW] = sliderYY;

    // #pragma omp parallel for
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
    if (key == 'r' || key == 'R')
        restart();
    if (key == 's' || key == 'S')
        shuffle();
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
    if (position.x < WALL_REPEL_BOUND)
        velocity.x += (WALL_REPEL_BOUND - position.x) * WALL_REPEL_FORCE;
    else if (position.x > MAP_WIDTH - WALL_REPEL_BOUND)
        velocity.x += (MAP_WIDTH - WALL_REPEL_BOUND - position.x) * WALL_REPEL_FORCE;

    if (position.y < WALL_REPEL_BOUND)
        velocity.y += (WALL_REPEL_BOUND - position.y) * WALL_REPEL_FORCE;
    else if (position.y > MAP_HEIGHT - WALL_REPEL_BOUND)
        velocity.y += (MAP_HEIGHT - WALL_REPEL_BOUND - position.y) * WALL_REPEL_FORCE;
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
    
    // const float dt = 0.999;  //ousiastika einai kati san tribi
    this->velocity = (this->velocity+force_strength * direction) *(1-viscosity);
    // to dt na koitaksw
}

// Creates a specifc number of every particle type and adds them to the vector of particles
// Every particle is initialized with random positions
void ofApp::Create_particles(){
    // Initialize the particleGroups vector with empty vectors for each type
    particleGroups.resize(NUM_TYPES);
    for (int j = 0; j < NUM_TYPES; j++) {
        particleGroups[j].reserve(number_of_particles); // reserve space for the particles
    }
    particles.reserve(number_of_particles * NUM_TYPES);

    for (int j = 0; j < NUM_TYPES; j++)
    {
        for (int i = 0; i < number_of_particles; i++)
        {
            Particle newParticle(ofRandom(MAP_WIDTH), ofRandom(MAP_HEIGHT), j);
            particles.push_back(newParticle);
            particleGroups[j].push_back(newParticle);
        }
    }
    total_particles = number_of_particles * NUM_TYPES;
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

void ofApp::restart(){
    particles.clear();
    Create_particles();
}

void ofApp::shuffle(){
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    sliderRR = force_matrix[RED][RED];
    sliderRG = force_matrix[RED][GREEN];
    sliderRY = force_matrix[RED][YELLOW];
    sliderGR = force_matrix[GREEN][RED];
    sliderGG = force_matrix[GREEN][GREEN];
    sliderGY = force_matrix[GREEN][YELLOW];
    sliderYR = force_matrix[YELLOW][RED];
    sliderYG = force_matrix[YELLOW][GREEN];
    sliderYY = force_matrix[YELLOW][YELLOW];
}