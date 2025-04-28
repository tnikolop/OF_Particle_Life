# Particle Life Simulator
 A simple program to simulate primitive Artificial Life using simple rules of attraction or repulsion among atom-like particles, producing complex self-organzing life-like patterns.

This is the new version of Particle Life using Open Frameworks

### TO DO:
- Better Thread Management
- Better GUI
  
### BUG FIXES
- if all 3 of the same color radius is 0 then fps drops dramatically. Alla ama kaneis restart gia kapoion logo ftiaxnei
- ta threads krasaroun se xamhlo arithmo se tyxaies stigmes

### TO ADD:
- Change individual number of particles per color
- 3D version

### INSATLLATION:
- https://openframeworks.cc/download/ Downlaod
- https://openframeworks.cc/setup/linux-install/ Follow steps
-If you see the following error:
- - usr/bin/ld.gold: error: cannot find -lXi
- - /usr/bin/ld.gold: error: cannot find -lXcursor
-- /usr/bin/ld.gold: error: cannot find -lXinerama
- Do 
  - - sudo apt-get install libxi-dev libxcursor-dev libxinerama-dev
 
 - After installing Project Generator use it to import the downloaded github, in order to create the make files

https://github.com/roymacdonald/ofxDropdown dropdown 
also use project generator to add addons