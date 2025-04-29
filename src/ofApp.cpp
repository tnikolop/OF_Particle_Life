#include "ofApp.h"

// Constant Variables
short MAP_WIDTH;                
short MAP_HEIGHT;
short FORCE_RANGE = 200;
short number_of_particles[NUM_TYPES] = {1000,1000,1000};                       // per type (color)
float viscosity;
short total_particles = -1;
float force_matrix[NUM_TYPES][NUM_TYPES]{{0}};               // the forces of attraction of each individual color against every other color
int color_force_range_matrix_squared[NUM_TYPES][NUM_TYPES]{{0}};      // the force range of each individual color againts every other color
                                                                        // squared so we save computational time on compute force and 
                                                                        // dont calculate the square distance thouasands of times needlesly
short numThreads;
int particlesPerThread;
//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0,0,0);    // Black Background Color
    // The map is offset MAP_BORDER in both axis for better visibility
    MAP_WIDTH = 0.75 * ofGetScreenWidth() + MAP_BORDER;      
    MAP_HEIGHT = 0.95 * ofGetScreenHeight() + MAP_BORDER;
    
    numThreads = std::thread::hardware_concurrency(); // Get the number of available hardware threads
    if (numThreads == 0) {
        numThreads = 1; // Fallback to 1 if hardware_concurrency() is not well-defined
        ofLogError() << "Only 1 thread is being utilized"; 
    }
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    restart();      // create particles and initialize vectors
    create_settings_dir();
    
    //========================= CREATE GUI =========================================
    gui.setup("Settings");
    gui.setPosition(MAP_WIDTH+70,20);
    gui.setWidthElements(260);
    gui.add(button_restart.setup("RESTART (R)"));
    button_restart.addListener(this,&ofApp::restart);
    gui.add(button_shuffle.setup("SHUFFLE (S)"));
    button_shuffle.addListener(this,&ofApp::shuffle);

    SimSettings.setup("Simulation Settings");
    gui.add(&SimSettings);
    SimSettings.add(toggle_reverse_velocity.setup("REVERSE VELOCITY ON MAP EDGE",false));
    SimSettings.add(toggle_shuffle_numbers.setup("Shuffle Number of Particles",false));
    SimSettings.add(field_n_particles.setup("PARTICLES PER COLOR",1000,1,MAX_PARTICLES));
    SimSettings.add(slider_viscosity.setup("VISCOSITY",0.001F,0.0F,0.1F));  //Max Viscosity 0.1
    SimSettings.add(slider_wall_repel_force.setup("WALL REPEL FORCE",0.1F,0,WALL_REPEL_FORCE_MAX));

    RedSettings.setup("RED SETTINGS");
    SimSettings.add(&RedSettings);
    RedSettings.add(field_number_R.setup("Number of Particles",number_of_particles[RED],1,MAX_PARTICLES));
    RedSettings.add(sliderRR.setup("RED X RED",force_matrix[RED][RED],-MAX_FORCE,MAX_FORCE));
    RedSettings.add(sliderRG.setup("RED X GREEN",force_matrix[RED][GREEN],-MAX_FORCE,MAX_FORCE));
    RedSettings.add(sliderRY.setup("RED X YELLOW",force_matrix[RED][YELLOW],-MAX_FORCE,MAX_FORCE));
    sliderRR.setFillColor(ofColor::darkRed);
    sliderRG.setFillColor(ofColor::darkRed);
    sliderRY.setFillColor(ofColor::darkRed);
    RedSettings.add(slider_rangeRR.setup("Radius of RED X RED",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    RedSettings.add(slider_rangeRG.setup("Radius of RED X GREEN",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    RedSettings.add(slider_rangeRY.setup("Radius of RED X YELLOW",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));

    GreenSettings.setup("GREEN SETTINGS");
    SimSettings.add(&GreenSettings);
    GreenSettings.add(field_number_G.setup("Number of Particles",number_of_particles[GREEN],1,MAX_PARTICLES));
    GreenSettings.add(sliderGR.setup("GREEN X RED",force_matrix[GREEN][RED],-MAX_FORCE,MAX_FORCE));
    GreenSettings.add(sliderGG.setup("GREEN X GREEN",force_matrix[GREEN][GREEN],-MAX_FORCE,MAX_FORCE));
    GreenSettings.add(sliderGY.setup("GREEN X YELLOW",force_matrix[GREEN][YELLOW],-MAX_FORCE,MAX_FORCE));
    sliderGR.setFillColor(ofColor::darkGreen);
    sliderGG.setFillColor(ofColor::darkGreen);
    sliderGY.setFillColor(ofColor::darkGreen);
    GreenSettings.add(slider_rangeGR.setup("Radius of GREEN X RED",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    GreenSettings.add(slider_rangeGG.setup("Radius of GREEN X GREEN",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    GreenSettings.add(slider_rangeGY.setup("Radius of GREEN X YELLOW",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));

    YellowSettings.setup("YELLOW SETTINGS");
    SimSettings.add(&YellowSettings);
    YellowSettings.add(field_number_Y.setup("Number of Particles",number_of_particles[YELLOW],1,MAX_PARTICLES));
    YellowSettings.add(sliderYR.setup("YELLOW X RED",force_matrix[YELLOW][RED],-MAX_FORCE,MAX_FORCE));
    YellowSettings.add(sliderYG.setup("YELLOW X GREEN",force_matrix[YELLOW][GREEN],-MAX_FORCE,MAX_FORCE));
    YellowSettings.add(sliderYY.setup("YELLOW X YELLOW",force_matrix[YELLOW][YELLOW],-MAX_FORCE,MAX_FORCE));
    sliderYR.setFillColor(ofColor::darkGoldenRod);
    sliderYG.setFillColor(ofColor::darkGoldenRod);
    sliderYY.setFillColor(ofColor::darkGoldenRod);
    YellowSettings.add(slider_rangeYR.setup("Radius of YELLOW X RED",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    YellowSettings.add(slider_rangeYG.setup("Radius of YELLOW X GREEN",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));
    YellowSettings.add(slider_rangeYY.setup("Radius of YELLOW X YELLOW",ofRandom(0,FORCE_RANGE),0,FORCE_RANGE));

    gui.add(button_save_settings.setup("Save Simulation"));
    button_save_settings.addListener(this,&ofApp::save_settings);
    gui.add(dropdown.setup("Load Simulation"));
    dropdown.addListener(this,&ofApp::load_settings);
    if (!dropdown.populateFromDirectory(settings_folder_path, {"xml"}))
    ofLogError() << "Could not populate dropdown from path: "+ofToDataPath(settings_folder_path);

    gui.add(field_get_name.setup("Simulation Name:",""));
    feedback.setup("","");
    gui.add(&feedback);
    }

//--------------------------------------------------------------
void ofApp::update(){
    // number_of_particles = field_n_particles;
    number_of_particles[RED] = field_number_R;
    number_of_particles[GREEN] = field_number_G;
    number_of_particles[YELLOW] = field_number_Y;
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

    color_force_range_matrix_squared[RED][RED] = slider_rangeRR * slider_rangeRR;
    color_force_range_matrix_squared[RED][GREEN] = slider_rangeRG * slider_rangeRG;
    color_force_range_matrix_squared[RED][YELLOW] = slider_rangeRY * slider_rangeRY;
    color_force_range_matrix_squared[GREEN][RED] = slider_rangeGR * slider_rangeGR;
    color_force_range_matrix_squared[GREEN][GREEN] = slider_rangeGG * slider_rangeGG;
    color_force_range_matrix_squared[GREEN][YELLOW] = slider_rangeGY * slider_rangeGY;
    color_force_range_matrix_squared[YELLOW][RED] = slider_rangeYR * slider_rangeYR;
    color_force_range_matrix_squared[YELLOW][GREEN] = slider_rangeYG * slider_rangeYG;
    color_force_range_matrix_squared[YELLOW][YELLOW] = slider_rangeYY * slider_rangeYY;    

    if (particlesPerThread > 25)
    {
        // Compute forces using Threads
        try
        {
	    vector<std::unique_ptr<ParticleThread>> threads;
            for (int i = 0; i < numThreads; i++) {
                int startIdx = i * particlesPerThread;
                int endIdx = (i == numThreads - 1) ? total_particles : startIdx + particlesPerThread;
                threads.emplace_back(std::make_unique<ParticleThread>(&all_particles, startIdx, endIdx,total_particles,slider_wall_repel_force));
                threads.back()->startThread();
            }
            for (auto& thread : threads) {
                thread->waitForThread();
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        // cerr << "FLAG 1" << endl;
    }
    else    // No threads
    {
        for (int i = 0; i < total_particles; i++)
        {
            for (int j = 0; j < total_particles; j++)
            {
                if (i!= j) {
                    all_particles[i].compute_Force(all_particles[j]);
                }
            }
            all_particles[i].apply_WallRepel(slider_wall_repel_force);
        }
    }
    // cerr << "FLAG 2" << endl;
    for (size_t i = 0; i < all_particles.size(); i++) {
        all_particles[i].update(toggle_reverse_velocity);
        all_positions[i] = all_particles[i].position;  // Update positions in all_positions
    }
    // cerr << "FLAG 3" << endl;
    // Update the VBO with the new positions
    vbo.updateVertexData(all_positions.data(), all_positions.size());
        // cerr << "FLAG 4" << endl;
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
void Particle::update(bool toggle){
    position += velocity;  // Add velocity to position to move the particle
    short t = toggle ? -1 : 1;  //if toggle is on reverse velocity on map edge

    // the particles must always be on screen
    if (position.x > MAP_WIDTH) {
        position.x = MAP_WIDTH - 1;
        velocity.x *= t;
    }
    else if (position.x < MAP_BORDER) {
        position.x = MAP_BORDER;
        velocity.x *= t;
    }
    if (position.y > MAP_HEIGHT) {
        position.y = MAP_HEIGHT - 1;
        velocity.y *= t;
    }
    else if (position.y < MAP_BORDER) {
        position.y = MAP_BORDER;
        velocity.y *= t;
    }
}

// Force that repells the particles from the edge of the map
// so they do not stay there
void Particle::apply_WallRepel(float force){
    if (force == 0)
        return;
    if (position.x < WALL_REPEL_BOUND)
        velocity.x += (WALL_REPEL_BOUND - position.x) * force;
    else if (position.x > MAP_WIDTH - WALL_REPEL_BOUND)
        velocity.x += (MAP_WIDTH - WALL_REPEL_BOUND - position.x) * force;

    if (position.y < WALL_REPEL_BOUND)
        velocity.y += (WALL_REPEL_BOUND - position.y) * force;
    else if (position.y > MAP_HEIGHT - WALL_REPEL_BOUND)
        velocity.y += (MAP_HEIGHT - WALL_REPEL_BOUND - position.y) * force;
}

// Calculate the forces that act on this specific particle 
// based on another particle
void Particle::compute_Force(const Particle& acting_particle){
    glm::vec2 direction = acting_particle.position - this->position;
    
    float distance2 = glm::distance2(this->position,acting_particle.position);  // distance2 = distance^2 for less computation time
    float force_strength=0;     // if out of range dont apply any force
    float force_range = color_force_range_matrix_squared[this->type][acting_particle.type];

    // Avoid division by zero
    if (distance2 > 0 && distance2 < force_range)
        force_strength = force_matrix[this->type][acting_particle.type] / distance2;

    this->velocity = (this->velocity+force_strength * direction) *(1-viscosity);
}

// Creates a specifc number of every particle type and adds them to the vector of particles
// Every particle is initialized with random positions
void ofApp::Create_particles(){
    for (int j = 0; j < NUM_TYPES; j++)
    {
        for (int i = 0; i < number_of_particles[j]; i++)
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
    // threads.clear();
    total_particles = 0;
    for (int i = 0; i < NUM_TYPES; i++)
    {
        total_particles += number_of_particles[i];
    }
    
    // total_particles = number_of_particles * NUM_TYPES;
    particlesPerThread = total_particles / numThreads;
    // cerr << particlesPerThread << endl;
    // threads.reserve(numThreads);
    
    all_particles.reserve(total_particles);
    all_colors.reserve(total_particles);
    all_positions.reserve(total_particles);
    Create_particles();
    feedback = ""; // clean feedback text. kserw oti yparxei kalyterow tropos alla aytos einai o pio aplos

}

// populates the force and force_range matrixes with random values and updates the gui sliders
void ofApp::shuffle(){
    initialize_forces(-MAX_FORCE,MAX_FORCE);
    // initialize_color_force_range(0,FORCE_RANGE);
    sliderRR = force_matrix[RED][RED];
    sliderRG = force_matrix[RED][GREEN];
    sliderRY = force_matrix[RED][YELLOW];
    sliderGR = force_matrix[GREEN][RED];
    sliderGG = force_matrix[GREEN][GREEN];
    sliderGY = force_matrix[GREEN][YELLOW];
    sliderYR = force_matrix[YELLOW][RED];
    sliderYG = force_matrix[YELLOW][GREEN];
    sliderYY = force_matrix[YELLOW][YELLOW];
    slider_rangeRR = ofRandom(0,FORCE_RANGE);
    slider_rangeRG = ofRandom(0,FORCE_RANGE);
    slider_rangeRY = ofRandom(0,FORCE_RANGE);
    slider_rangeGR = ofRandom(0,FORCE_RANGE);
    slider_rangeGG = ofRandom(0,FORCE_RANGE);
    slider_rangeGY = ofRandom(0,FORCE_RANGE);
    slider_rangeYR = ofRandom(0,FORCE_RANGE);
    slider_rangeYG = ofRandom(0,FORCE_RANGE);
    slider_rangeYY = ofRandom(0,FORCE_RANGE);  
    feedback = ""; // clean feedback text. kserw oti yparxei kalyterow tropos alla aytos einai o pio aplos

    if (toggle_shuffle_numbers) {
        field_number_R = ofRandom(1,MAX_PARTICLES);
        field_number_G = ofRandom(1,MAX_PARTICLES);
        field_number_Y = ofRandom(1,MAX_PARTICLES);
    }
}

// Save all current Simulation parameters
void ofApp::save_settings(){
    string name = field_get_name;

    // Check if the name is valid
    if (name.empty()) {
        ofLogWarning() << "Simulation name is empty. Can not save current simulation!";
        feedback = "Name field cannot be empty!";
        feedback.setDefaultTextColor(ofColor::red);
        return;
    }

    // Check if the file already exists
    string filePath = "Settings/"+name + ".xml";
    ofFile file(filePath);
    if (file.exists()) {
        ofLogWarning() << "A file with that name ["+name+"] already exists. Simulation not saved!";
        feedback = "Name already exists!";
        feedback.setDefaultTextColor(ofColor::red);
        return;
    }
    SimSettings.saveToFile(filePath);
    feedback = "Saved Succesfullty!";  
    feedback.setDefaultTextColor(ofColor::green);

    // // repopulate dropdown list with the new entry
    // //dropdown.clear() is broken and causes bugs
    // dropdown.clear();
    // dropdown.setup("Load Saved Simulation Settings");
    // if (!dropdown.populateFromDirectory(ofToDataPath("Settings"), {"xml"}))
    //     ofLogError("Could not populate dropdown from path: "+ofToDataPath(""));
    //
    // thats why I removed this code. There is no refreshing. Just restart the program for loading new settings 
}

// Load saved settings from a list
void ofApp::load_settings(ofFile &file){
    string file_name = file.getFileName();
    // Load settings
    string file_path = "Settings/"+file_name;
    SimSettings.loadFromFile(file_path);
    dropdown.deselect();
    feedback = ""; // clean feedback text. kserw oti yparxei kalyterow tropos alla aytos einai o pio aplos
    restart();  // to kaei thn prwth fora

}

// Creates a directory for storing all the saved simulation settings
void ofApp::create_settings_dir()
{
    if (!ofDirectory(settings_folder_path).exists()) {
        bool created = ofDirectory::createDirectory(settings_folder_path, true, true);
        if (created == false)
        {
            ofLogError("Setup") << "Failed to create settings directory at: " << settings_folder_path;
        }
    }
}
