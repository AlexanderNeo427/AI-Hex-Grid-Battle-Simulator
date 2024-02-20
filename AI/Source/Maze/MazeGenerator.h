#ifndef _MAZE_GENERATOR_H_
#define _MAZE_GENERATOR_H_

#include "Maze.h"

class MazeGenerator
{
public:
	MazeGenerator();
	~MazeGenerator();

	static void SimpleGenerate(unsigned key, Maze* pMaze);
	static void MrTangsAlgorithm(unsigned key, MazePt start, float wallLoad, Maze* pMaze);
	static void PerlinGenerate(int numOctaves, double persistance, Maze* pMaze);
};

#endif

