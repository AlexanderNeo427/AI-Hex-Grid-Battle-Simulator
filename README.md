# NYP-AI-Assignment-2

### Features
* Turn-based game/simulation, played by 2 AI teams (RED vs BLUE)
* Made with C++ and OpenGL
* Each tile type has a different colour and movement cost
* Each AI unit type has a different movement range/range of vision. Their range of vision will slowly reveal the map (the glowing tiles, visualisaing each units' field of view) 
* Fun random events! Such as:
  * Meteor explosion every once in awhile
  * Occasionally an AI unit will spontaneously drop dead

### Technical Details
* Randomly generated tile types, using a sort of pseudo-perlin noise. Referenced algorithm from [this page](https://rtouti.github.io/graphics/perlin-noise-algorithm)
* A* pathfinding for units to optimize the cost of traversing nodes
* Each AI has an instance of a State Machine that controls its behaviour
* Utilizing "Messaging/Event-Drven/Event Queue" architecture for cross-gameObject communication within the game scene
* A basic Object Pool
* Hex Grid movement, courtesy of the article by [Red Blob Games](https://www.redblobgames.com/grids/hexagons/)
