#pragma once
#include "glm/glm.hpp"
#include "Gizmos.h"
#include <vector>

enum WorldObjectType
{
	SIMPLE_AI,
	WATER,
	FOOD,
	CAVE,
	ENEMY_AI,
	AIWATER,
	AIFOOD,
	AICAVE,
	SAFEZONE,
};

using namespace std;
class WorldObject
{
public:
	vector<WorldObject* > *worldObjects;
	virtual void update(float delta) = 0;
	virtual void draw();
	float radius;
	glm::vec4 colour;
	glm::vec2 position;
	WorldObjectType type;
};

class BaseResource:public WorldObject
{
	public:
	float amount;

	BaseResource(glm::vec2 position,glm::vec4 colour,float radius);
	void virtual update(float delta){};
};

//---------------------------------------//
// AI Stat Refills
class Water:public BaseResource
{
	public:
	Water(glm::vec2 position);
};

class Food:public BaseResource
{
	public:
	Food(glm::vec2 position);
};

class Cave:public BaseResource
{
	public:
	Cave(glm::vec2 position);
};

class AICave :public BaseResource
{
public:
	AICave(glm::vec2 position);
};

class AIWater :public BaseResource
{
public:
	AIWater(glm::vec2 position);
};

class AIFood :public BaseResource
{
public:
	AIFood(glm::vec2 position);
};

class SafeZone :public BaseResource
{
public:
	SafeZone(glm::vec2 position);
};
//---------------------------------------//

class BaseAgent : public WorldObject
{
public:
	BaseAgent(int id, glm::vec2 position, glm::vec4 colour, float radius);
	float findNearestResource(WorldObjectType type);
	glm::vec2 findResourceVector(WorldObjectType type);
	
	bool InnocentSafe;
	// AI Innocent Bar Items
	float tiredness;
	float food;
	float water;
	float InnocentHealth;
	// AI Enemy Bar Items
	float AItiredness;
	float AIfood;
	float AIwater;
	float EnemyHealth;
	int agentId;
	// Colours
	glm::vec4 TealBlue;
	glm::vec4 Green;
	glm::vec4 Blue;
	glm::vec4 Red;
};

class Agent : public BaseAgent
{
public:
	bool ActivationEnemyAI;
	float InnocentSpeed;
	float EnemySpeed;
	virtual void update(float delta);
	virtual void draw() override;
	void drawBar(float value,int index, glm::vec2 Pos);
	Agent(int id, glm::vec2 position, glm::vec4 agentColour);
	
	// Desirable Check
	float checkEatingDesirable(float FoodValue, WorldObjectType type);
	float checkSleepDesirable(float RestValue, WorldObjectType type);
	float checkDrinkingDesirable(float DrinkValue, WorldObjectType type);
	float checkInnocentAIDesirable(float HealthValue, WorldObjectType type);
	float checkRunStatDesirable(float Run, WorldObjectType type);
	// Desirable Set
	float eatDesirability;
	float sleepDesirability;
	float drinkDesirability;
	float attackDesirability;
	float runDesirability;

	// Food, Water, AI & Rest
	glm::vec2 gotoFood(float desirability,float delta);
	glm::vec2 gotoCave(float desirability,float delta);
	glm::vec2 gotoWater(float desirability,float delta);
	glm::vec2 gotoInnocentAI(float desirability, float delta);
	glm::vec2 gotoAIFood(float desirability, float delta);
	glm::vec2 gotoAICave(float desirability, float delta);
	glm::vec2 gotoAIWater(float desirability, float delta);
	glm::vec2 gotoSafeZone(float desirability, float delta);
};

class WorldController
{
public:
	vector<WorldObject*> worldObjects;
	void update(float delta);
	void draw();
	void addObject(WorldObject* newObject);
};