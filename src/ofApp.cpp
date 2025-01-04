#include "ofApp.h"

short FORCE_RANGE = 200;
short number_of_particles = 1000;                       // per type (color)
int FORCE_RANGE_SQUARED = FORCE_RANGE * FORCE_RANGE;    // for less computational time
float viscosity = 0;
short total_particles = -1;
float force_matrix[NUM_TYPES][NUM_TYPES]{{0}};

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0,0,0);    // Black Background Color
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    restart();      // create particles and initialize vectors
    
    //========================= CREATE GUI =========================================
    gui.setup("Settings");
    gui.setPosition(MAP_WIDTH+70,20);
    gui.add(button_restart.setup("RESTART (R)"));
    button_restart.addListener(this,&ofApp::restart);
    gui.add(button_shuffle.setup("SHUFFLE (S)"));
    button_shuffle.addListener(this,&ofApp::shuffle);

    gui.add(slider_force_range.setup("FORCE RANGE",FORCE_RANGE,0,FORCE_RANGE));
    gui.add(field_n_particles.setup("PARTICLES PER COLOR",number_of_particles,1,3000));
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
    FORCE_RANGE = slider_force_range;
    FORCE_RANGE_SQUARED = FORCE_RANGE * FORCE_RANGE;
    number_of_particles = field_n_particles;
    viscosity = slider_viscosity;

    // this needs to be automatic for dynamic number of types
    force_matrix[RED][RED] = sliderRR;
    force_matrix[RED][GREEN] = sliderRG;
    force_matrix[RED][YELLOW] = sliderRY;
    force_matrix[GREEN][RED] = sliderGR;
    force_matrix[GREEN][GREEN] = sliderGG;
    force_matrix[GREEN][YELLOW] = sliderGY;
    force_matrix[YELLOW][RED] = sliderYR;
    force_matrix[YELLOW][GREEN] = sliderYG;
    force_matrix[YELLOW][YELLOW] = sliderYY;

    int numThreads = std::thread::hardware_concurrency(); // Get the number of available hardware threads
    if (numThreads == 0) {
        numThreads = 1; // Fallback to 1 if hardware_concurrency() is not well-defined
        cerr << "Only 1 thread is being utilized" << endl;
    }
    int particlesPerThread = total_particles / numThreads;
    std::vector<std::unique_ptr<ParticleThread>> threads;

    for (int i = 0; i < numThreads; i++) {
        int startIdx = i * particlesPerThread;
        int endIdx = (i == numThreads - 1) ? total_particles : startIdx + particlesPerThread;
        threads.emplace_back(std::make_unique<ParticleThread>(&all_particles, startIdx, endIdx,total_particles));
        threads.back()->startThread();
    }
    for (auto& thread : threads) {
        thread->waitForThread();
    }

    for (size_t i = 0; i < all_particles.size(); i++) {
        all_particles[i].update();
        all_positions[i] = all_particles[i].position;  // Update positions in all_positions
    }
    // Update the VBO with the new positions
    vbo.updateVertexData(all_positions.data(), all_positions.size());
}

//--------------------------------------------------------------
void ofApp::draw(){
    gui.draw();
    vbo.draw(GL_POINTS,0,total_particles);
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
    type = color;                       // Assign the particle type
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

// -------- NOT IN USE (OUTDATED) --------
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
    
    float distance2 = glm::distance2(this->position,acting_particle.position);  // distance2 = distance^2 for less computation time
    float force_strength=0;     // if out of range dont apply any force

    // Avoid division by zero
    if (distance2 > 0 && distance2 < FORCE_RANGE_SQUARED)
        force_strength = force_matrix[this->type][acting_particle.type] / distance2;

    this->velocity = (this->velocity+force_strength * direction) *(1-viscosity);
}

// Creates a specifc number of every particle type and adds them to the vector of particles
// Every particle is initialized with random positions
void ofApp::Create_particles(){
    for (int j = 0; j < NUM_TYPES; j++)
    {
        for (int i = 0; i < number_of_particles; i++)
        {
            // -------------------- pointers maybe here --------------------
            Particle newParticle(ofRandom(MAP_WIDTH), ofRandom(MAP_HEIGHT), j);
            all_particles.push_back(newParticle);
            all_positions.push_back(newParticle.position);     // Extract only the position
            all_colors.push_back(newParticle.getColor());      // Extract color
        }
    }
    vbo.setVertexData(all_positions.data(),all_positions.size(),GL_STREAM_DRAW);
    vbo.setColorData(all_colors.data(), all_colors.size(), GL_STREAM_DRAW);
}

// Returns Color of the particle
ofFloatColor Particle::getColor() const{
    if (this->type == 0) {
        return ofFloatColor(1, 0, 0);  // Red
    } else if (type == 1) {
        return ofFloatColor(0, 1, 0);  // Green
    } else {
        return ofFloatColor(1, 1, 0);  // Yellow
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

// Clears all vectors and creates particles from scratch
void ofApp::restart(){
    all_particles.clear();
    all_positions.clear();
    all_colors.clear();
    total_particles = number_of_particles * NUM_TYPES;
    all_particles.reserve(total_particles);
    all_colors.reserve(total_particles);
    all_positions.reserve(total_particles);
    Create_particles();
}

// populates the force matrix with random values and updates the gui sliders
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