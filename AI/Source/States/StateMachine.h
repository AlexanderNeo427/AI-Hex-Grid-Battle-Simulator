#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <string>
#include <map>
#include <unordered_map>

#include "State.h"

class StateMachine
{
	std::unordered_map<std::string, State*> m_stateMap;
	State *m_currState;
	State *m_nextState;
public:
	StateMachine();
	~StateMachine();

	void AddState(State *newState);
	void SetNextState(const std::string &nextStateID);
	const std::string& GetCurrentState();
	void Update(double dt);
	void UpdateTurn();
};

#endif