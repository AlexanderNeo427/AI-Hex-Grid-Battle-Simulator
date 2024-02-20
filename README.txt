CONTROLS
==============================================
-/+ 	 			 : Increase/decrease simulation speed (Highly suggest increasing)
R		 			 : Reset Game
Left/Right arrow keys: Cycle through 3 pre-generated mazes



MISC
==============================================
- DFS
	- Each unit will have a unique order 
	  in which they check directions
	 
- AStar
	- Shortest path(s) for all entities are rendered
	  (Assuming they are in the chase state)

- Visibility range
	- Each unit type will have a unique 
	  visibility range (FOV, dist from unit)
	- Vision will be blocked by walls

- Random events
	- Meteor strikes
	- Sudden death
	- Random spawns

- Game end conditions
	- One team reaches 12 kills
	- Game runs out of turns

- Maze(s) randomly generated via Perlin noise algorithm from 
https://web.archive.org/web/20160530124230/http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
( with some modifications )




