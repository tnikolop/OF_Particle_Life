#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundColor(0,0,0);
    Create_particles();
    
    
    
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

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

Particle::Particle(float x, float y, ofColor colour) {
    position = glm::vec2(x,y);          // Initialize position with input coordinates
    velocity = glm::vec2(0.0f,0.0f);    // Start with zero velocity
    color = colour;                      // Assign the particle type
}

void Particle::draw() {
    ofSetColor(color);
    ofDrawCircle(position,1);
}

// Creates a specifc number of every particle type and adds them to the vector of particles
// Every particle is initialized with random positions
void ofApp::Create_particles(){
    for (int i = 0; i < number_of_particles; i++)
    {
        for (int i=0; i<color_types.size(); i++)
        {
            Particle newParticle(ofRandom(MAP_WIDTH),ofRandom(MAP_HEIGHT),color_types[i]);
            particles.push_back(newParticle);
        }
    }       
}