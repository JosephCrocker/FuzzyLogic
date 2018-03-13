#include "SimulationObjects.h"
#include "FuzzyLogicEngine.h"
#include <iostream>

extern Fuzzy fuzzyEngine;

void WorldObject::draw()
{
	int segments = 20;
	Gizmos::add2DCircle(position, radius,segments, colour);
}

BaseAgent::BaseAgent(int id, glm::vec2 position, glm::vec4 colour, float radius)
{
	this->agentId = id;
	this->position = position;
	this->colour = colour;
	this->radius = radius;
}

BaseResource::BaseResource(glm::vec2 position,glm::vec4 colour,float radius)
{
	this->position = position;
	this->colour = colour;
	this->radius = radius;
}
 
//===----------------------------------------------------------------------------===//
// Food, Water, Rest & AI Items
Water::Water(glm::vec2 position):BaseResource(position, glm::vec4(0, 0, 1, 1), 40)
{
	type = WATER;
}

Food::Food(glm::vec2 position):BaseResource(position,glm::vec4(0, 1, 0, 1), 40)
{
	type = FOOD;
}

Cave::Cave(glm::vec2 position):BaseResource(position,glm::vec4(0, 1, 1, 1), 40)
{
	type = CAVE;
}

AICave::AICave(glm::vec2 position) : BaseResource(position, glm::vec4(0, 1, 1, 1), 40)
{
	type = AICAVE;
}

AIWater::AIWater(glm::vec2 position) : BaseResource(position, glm::vec4(0, 0, 1, 1), 40)
{
	type = AIWATER;
}

AIFood::AIFood(glm::vec2 position) : BaseResource(position, glm::vec4(0, 1, 0, 1), 40)
{
	type = AIFOOD;
}

SafeZone::SafeZone(glm::vec2 position) : BaseResource(position, glm::vec4(1, 1, 0, 1), 40)
{
	type = SAFEZONE;
}
//===----------------------------------------------------------------------------===//

Agent::Agent(int id, glm::vec2 position, glm::vec4 agentColour) : BaseAgent(id, position, glm::vec4(agentColour), 20)
{
	// Bar Colours
	TealBlue = glm::vec4(0, 1, 1, 1);
	Green = glm::vec4(0, 1, 0, 1);
	Blue = glm::vec4(0, 0, 1, 1);
	Red = glm::vec4(1, 0, 0, 1);

	// Innocent AI Stats
	tiredness = 1;
	food = 1;
	water = 1;
	InnocentSafe = false;

	// Enemy AI Stats
	AItiredness = 1;
	AIfood = 1;
	AIwater = 1;

	InnocentHealth = 1;
	EnemyHealth = 1;
	ActivationEnemyAI = false;
	// Setting Innocent AI
	if (ActivationEnemyAI == false)
	{
		type = SIMPLE_AI;
	}
	// Setting Enemy AI
	if (ActivationEnemyAI == true)
	{
		type = ENEMY_AI;
	}
	InnocentSpeed = 150;
	EnemySpeed = 150;
};

void Agent::drawBar(float value, int index, glm::vec2 Pos)
{
	glm::vec4 colours[] = {glm::vec4(TealBlue), glm::vec4(Green), glm::vec4(Blue), glm::vec4(Red), glm::vec4(TealBlue),glm::vec4(Green),glm::vec4(Blue), glm::vec4(Red)};
	float barLength = 90;
	float barHeight = 10;
	glm::vec4 back(1, 1, 1, 0.25f);
	glm::vec4 colour(0.3, 0.3, 0.3, 1);
	glm::vec2 extents(barLength,barHeight);
	glm::vec2 centrePos = Pos;

	centrePos.y -= barHeight * 3 * index;
	centrePos.x += barLength;
	Gizmos::add2DAABBFilled(centrePos, extents, back);

	barLength *= value;
	extents = glm::vec2(barLength,barHeight);
	centrePos = Pos;
	centrePos.y -= barHeight * 3 * index;
	centrePos.x += barLength;
	colour = colours[index];
	Gizmos::add2DAABBFilled(centrePos, extents, colour);
}

float BaseAgent::findNearestResource(WorldObjectType type)
{
	float minDistance = 1000000;
	for(auto object:*worldObjects)
	{
		if(object->type == type && this != object)
		{
			float distance = glm::length(object->position - position);
			if(distance < minDistance)
			{
				minDistance = distance;
			}
		}
	}
	return minDistance;
}

glm::vec2 BaseAgent::findResourceVector(WorldObjectType type)
{
	float minDistance = 1000000;
	glm::vec2 target;
	for(auto object:*worldObjects)
	{
		if(object->type == type && object != this)
		{
			float distance = glm::length(object->position - position);
			if(distance < minDistance)
			{
				minDistance = distance;
				target = object->position;
			}
		}
	}
	glm::vec2 vector;
	if(minDistance < 1)
	{
		vector = glm::vec2(0,0);
	}
	else
	{
		vector = (target - position)/minDistance;
	}
	return vector;
}

void Agent::draw()
{
	BaseAgent::draw();
	int startPosX = agentId * 1 + 10;

	// Innocent AI Stats
	if (ActivationEnemyAI == false)
	{
		drawBar(tiredness, 0, glm::vec2(startPosX, 700));
		drawBar(food, 1, glm::vec2(startPosX, 700));
		drawBar(water, 2, glm::vec2(startPosX, 700));
		drawBar(InnocentHealth, 3, glm::vec2(startPosX, 700));
	}

	// Enemy AI Stats
	if (ActivationEnemyAI == true)
	{
		drawBar(AItiredness, 4, glm::vec2(startPosX, 260));
		drawBar(AIfood, 5, glm::vec2(startPosX, 255));
		drawBar(AIwater, 6, glm::vec2(startPosX, 250));
		drawBar(EnemyHealth, 7, glm::vec2(startPosX, 245));
	}
}

//===----------------------------------------------------------------------------===//
// Desirability Checker / Setter
float Agent::checkEatingDesirable(float FoodValue, WorldObjectType type)
{
	float desire = 0; 
	float foodRange = findNearestResource(type);
	float hungry = fuzzyEngine.hungry->getMembership(FoodValue);

	float full = fuzzyEngine.full->getMembership(FoodValue);
	float veryHungry = fuzzyEngine.veryHungry->getMembership(FoodValue);
	float foodRangeClose = fuzzyEngine.veryNear->getMembership(foodRange);
	float foodRangeMedium = fuzzyEngine.mediumRange->getMembership(foodRange);
	float foodRangeFar = fuzzyEngine.farAway->getMembership(foodRange);
	
	float veryDesirableValue = Fuzzy::OR(Fuzzy::AND(foodRangeClose, hungry), veryHungry);
	float desirableValue = Fuzzy::AND(Fuzzy::NOT(foodRangeFar), hungry);
	float undesirableValue = full;

	float maxVeryDesirable = fuzzyEngine.veryDesirable->getMaxMembership();
	float maxDesirable = fuzzyEngine.desirable->getMaxMembership();
	float maxUndesirable = fuzzyEngine.undesirable->getMaxMembership();

	desire = maxVeryDesirable * veryDesirableValue + maxDesirable * desirableValue + maxUndesirable * undesirableValue;
	desire /= (.1f + veryDesirableValue + desirableValue + undesirableValue);
	return desire;
}

float Agent::checkSleepDesirable(float RestValue, WorldObjectType type)
{
	float desire = 0; 
	float caveRange = findNearestResource(type);
	float tired = fuzzyEngine.tired->getMembership(RestValue);
	float active = fuzzyEngine.superActive->getMembership(RestValue);
	float awake = fuzzyEngine.awake->getMembership(RestValue);

	float caveClose = fuzzyEngine.veryNear->getMembership(caveRange);
	float caveMedium = fuzzyEngine.mediumRange->getMembership(caveRange);
	float caveFar = fuzzyEngine.farAway->getMembership(caveRange);
	
	float veryDesirableValue = Fuzzy::OR(Fuzzy::AND(caveClose, awake), tired);
	float desirableValue = Fuzzy::AND(Fuzzy::NOT(caveFar), tired);
	float undesirableValue = active;

	float maxVeryDesirable = fuzzyEngine.veryDesirable->getMaxMembership();
	float maxDesirable = fuzzyEngine.desirable->getMaxMembership();
	float maxUndesirable = fuzzyEngine.undesirable->getMaxMembership();

	desire = maxVeryDesirable * veryDesirableValue + maxDesirable * desirableValue + maxUndesirable * undesirableValue;
	desire /= (.1f + veryDesirableValue + desirableValue + undesirableValue);
	return desire;
}

float Agent::checkDrinkingDesirable(float DrinkValue, WorldObjectType type)
{
	float desire = 0; 
	float waterRange = findNearestResource(type);
	float thirsty = fuzzyEngine.thirsty->getMembership(DrinkValue);
	float notThirsty = fuzzyEngine.notThirsty->getMembership(DrinkValue);
	float veryThirsty = fuzzyEngine.veryThirsty->getMembership(DrinkValue);
	float weekFromThirst = fuzzyEngine.WeakFromThirsty->getMembership(DrinkValue);

	float waterRangeClose = fuzzyEngine.veryNear->getMembership(waterRange);
	float waterRangeMedium = fuzzyEngine.mediumRange->getMembership(waterRange);
	float waterRangeFar = fuzzyEngine.farAway->getMembership(waterRange);

	float veryDesirableValue = Fuzzy::OR(Fuzzy::AND(waterRangeClose, thirsty), veryThirsty);
	veryDesirableValue = Fuzzy::OR(veryDesirableValue, weekFromThirst);
	float desirableValue = Fuzzy::AND(Fuzzy::NOT(waterRangeClose), thirsty);
	float undesirableValue = notThirsty;

	float maxVeryDesirable = fuzzyEngine.veryDesirable->getMaxMembership();
	float maxDesirable = fuzzyEngine.desirable->getMaxMembership();
	float maxUndesirable = fuzzyEngine.undesirable->getMaxMembership();

	desire = maxVeryDesirable * veryDesirableValue + maxDesirable * desirableValue + maxUndesirable * undesirableValue;
	desire /= (0.1f + veryDesirableValue + desirableValue + undesirableValue);
	return desire;
}

float Agent::checkInnocentAIDesirable(float HealthValue, WorldObjectType type)
{
	float desire = 0;
	if (ActivationEnemyAI == true)
	{
		float attackRange = findNearestResource(type);
		float Healthy = fuzzyEngine.Healthy->getMembership(HealthValue);
		float LowHp = fuzzyEngine.Injured->getMembership(HealthValue);

		float rangeFromClose = fuzzyEngine.veryNear->getMembership(attackRange);
		float rangeFromMedium = fuzzyEngine.mediumRange->getMembership(attackRange);
		float rangeFromFar = fuzzyEngine.farAway->getMembership(attackRange);

		float veryDesirableValue = Fuzzy::OR(Fuzzy::AND(rangeFromClose, LowHp), Healthy);
		//veryDesirableValue = Fuzzy::OR(veryDesirableValue, LowHp);
		float desirableValue = Fuzzy::AND(Fuzzy::NOT(rangeFromClose), Healthy);
		float undesirableValue = LowHp;

		float maxVeryDesirable = fuzzyEngine.veryDesirable->getMaxMembership();
		float maxDesirable = fuzzyEngine.desirable->getMaxMembership();
		float maxUndesirable = fuzzyEngine.undesirable->getMaxMembership();
		desire = maxVeryDesirable * veryDesirableValue + maxDesirable * desirableValue + maxUndesirable * undesirableValue;
		desire /= (0.1f + veryDesirableValue + desirableValue + undesirableValue);
	}
	return desire;
}

float Agent::checkRunStatDesirable(float HealthValue, WorldObjectType type) 
{
	float desire = 0;
	if (ActivationEnemyAI == false)
	{
		float RunRange = findNearestResource(type);
		float Stay = fuzzyEngine.Stay->getMembership(HealthValue);
		float StartRunning = fuzzyEngine.StartRunning->getMembership(HealthValue);

		float rangeFromClose = fuzzyEngine.veryNear->getMembership(RunRange);
		float rangeFromMedium = fuzzyEngine.mediumRange->getMembership(RunRange);
		float rangeFromFar = fuzzyEngine.farAway->getMembership(RunRange);

		float veryDesirableValue = Fuzzy::OR(Fuzzy::AND(rangeFromClose, Stay), StartRunning);
		veryDesirableValue = Fuzzy::OR(veryDesirableValue, Stay);
		float desirableValue = Fuzzy::AND(Fuzzy::NOT(rangeFromClose), StartRunning);
		float undesirableValue = Stay;

		float maxVeryDesirable = fuzzyEngine.veryDesirable->getMaxMembership();
		float maxDesirable = fuzzyEngine.desirable->getMaxMembership();
		float maxUndesirable = fuzzyEngine.undesirable->getMaxMembership();
		desire = maxVeryDesirable * veryDesirableValue + maxDesirable * desirableValue + maxUndesirable * undesirableValue;
		desire /= (0.1f + veryDesirableValue + desirableValue + undesirableValue);
	}
	return desire;
}
//===----------------------------------------------------------------------------===//
// Variable Desirability
glm::vec2 Agent::gotoFood(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(FOOD) * delta * (1+desirability) * InnocentSpeed;
	return velocity;
}

glm::vec2 Agent::gotoCave(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(CAVE) * delta * (1+desirability) * InnocentSpeed;
	return velocity;
}

glm::vec2 Agent::gotoWater(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(WATER) * delta * (1 + desirability) * InnocentSpeed;
	return velocity;
}

glm::vec2 Agent::gotoAIFood(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(AIFOOD) * delta * (1 + desirability) * EnemySpeed;
	return velocity;
}

glm::vec2 Agent::gotoAIWater(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(AIWATER) * delta * (1 + desirability) * EnemySpeed;
	return velocity;
}

glm::vec2 Agent::gotoAICave(float desirability, float delta)
{
	glm::vec2 velocity = findResourceVector(AICAVE) * delta * (1 + desirability) * EnemySpeed;
	return velocity;
}

glm::vec2 Agent::gotoInnocentAI(float desirability, float delta)
{
	glm::vec2 velocity;
	if (ActivationEnemyAI == true && InnocentSafe == false)
	{
		velocity = findResourceVector(SIMPLE_AI) * delta * (1 + desirability) * EnemySpeed;
	}
	return velocity;
}

glm::vec2 Agent::gotoSafeZone(float desirability, float delta)
{
	glm::vec2 velocity;
	if (ActivationEnemyAI == false)
	{
		velocity = findResourceVector(SAFEZONE) * delta * (1 + desirability) * InnocentSpeed;
	}
	return velocity;
}
//===----------------------------------------------------------------------------===//

void Agent::update(float delta)
{
	if (ActivationEnemyAI == true)
	{
		eatDesirability = checkEatingDesirable(AIfood, AIFOOD);
		sleepDesirability = checkSleepDesirable(AItiredness, AICAVE);
		drinkDesirability = checkDrinkingDesirable(AIwater, AIWATER);
		attackDesirability = checkInnocentAIDesirable(InnocentHealth, SIMPLE_AI);
	}
	else if (ActivationEnemyAI == false)
	{
		eatDesirability = checkEatingDesirable(food, FOOD);
		sleepDesirability = checkSleepDesirable(tiredness, CAVE);
		drinkDesirability = checkDrinkingDesirable(water, WATER);
		runDesirability = checkRunStatDesirable(InnocentHealth, SIMPLE_AI);
	}
	glm::vec2 velocity;
	
	//===----------------------------------------------------------------------------===//
	// Food, Water & Rest Manager
	// Enemy AI
	if (ActivationEnemyAI == true)
	{
		if ((drinkDesirability >= sleepDesirability && drinkDesirability >= attackDesirability && drinkDesirability >= eatDesirability ))
		{
			velocity = gotoAIWater(drinkDesirability, delta);
		}
		else if (eatDesirability >= sleepDesirability && eatDesirability >= drinkDesirability)
		{
			velocity = gotoAIFood(eatDesirability, delta);
		}
		else if (sleepDesirability >= drinkDesirability)
		{
			velocity = gotoAICave(sleepDesirability, delta);
		}
		else
		{
			velocity = gotoInnocentAI(attackDesirability, delta);
		}
	}
	// Innocent AI
	if (ActivationEnemyAI == false)
	{
		if ((runDesirability >= sleepDesirability && runDesirability >= drinkDesirability && runDesirability >= eatDesirability))
		{
			velocity = gotoSafeZone(runDesirability, delta);
		}
		else if (eatDesirability > sleepDesirability && eatDesirability > drinkDesirability)
		{
			velocity = gotoFood(eatDesirability, delta);
		}
		else if (sleepDesirability > drinkDesirability)
		{
			velocity = gotoCave(sleepDesirability, delta);
		}
		else
		{
			velocity = gotoWater(drinkDesirability, delta);
		}
	}
	position += velocity;

	//---------------------------------------//
	#pragma region Water Section:
	// Water
	if (ActivationEnemyAI == true)
	{
		if (AIwater > 0) {
			std::cout << "BOT 1 Water " << AIwater << std::endl;
		}
		if (findNearestResource(AIWATER) < 2) {
			AIwater = 1; // If Ur Close To The Water
		}
		else {
			AIwater -= delta * 0.06f; // If Ur Not Close To The Water
		}
		if (AIwater <= 0) {
			AIwater = 0;
			EnemySpeed = 0;
			std::cout << "BOT 1 - Out Of Water!" << std::endl;
		}
		if (AIwater > 1) {
			AIwater = 1;
		}
	}
	else if (ActivationEnemyAI == false)
	{
		if (water > 0) {
			std::cout << "BOT 2 Water " << water << std::endl;
		}
		if (findNearestResource(WATER) < 2) {
			water = 1; // If Ur Close To The Water
		}
		else {
			water -= delta * 0.04f; // If Ur Not Close To The Water
		}
		if (water <= 0) {
			water = 0;
			InnocentSpeed = 0;
			std::cout << "BOT 2 - Out Of Water!" << std::endl;
		}
		if (water > 1) {
			water = 1;
		}
	}
	#pragma endregion
	//---------------------------------------//
	#pragma region Food Section:
	// Food
	if (ActivationEnemyAI == true)
	{
		if (AIfood > 0) {
			std::cout << "BOT 1 Food " << AIfood << std::endl;
		}
		if (findNearestResource(AIFOOD) < 2) {
			AIfood = 1; // If Ur Close To The Food
		}
		else {
			AIfood -= delta * 0.05f; // If Ur Not Close To The Food
		}
		if (AIfood <= 0) {
			AIfood = 0;
			EnemySpeed = 0;
			std::cout << "BOT 1 - Out Of Food!" << std::endl;
		}
		if (AIfood > 1) {
			AIfood = 1;
		}
	}
	else if (ActivationEnemyAI == false)
	{
		if (food > 0) {
			std::cout << "BOT 2 Food " << food << std::endl;
		}
		if (findNearestResource(FOOD) < 2) {
			food = 1; // If Ur Close To The Food
		}
		else {
			food -= delta * 0.04f; // If Ur Not Close To The Food
		}
		if (food <= 0) {
			food = 0;
			InnocentSpeed = 0;
			std::cout << "BOT 2 - Out Of Food!" << std::endl;
		}
		if (food > 1) {
			food = 1;
		}
	}
	#pragma endregion
	//---------------------------------------//
	#pragma region Rest Section:
	// Rest
	if (ActivationEnemyAI == true)
	{
		if (AItiredness > 0) {
			std::cout << "BOT 1 Tiredness " << AItiredness << std::endl;
		}
		if (findNearestResource(AICAVE) < 2) {
			AItiredness = 1; // If Ur Close To The Cave
		}
		else {
			AItiredness -= delta * 0.02f; // If Ur Not Close To The Cave
		}
		if (AItiredness <= 0) {
			AItiredness = 0;
			EnemySpeed = 0;
			std::cout << "BOT 1 - Out Of Energy!" << std::endl;
		}
		if (AItiredness > 1) {
			AItiredness = 1;
		}
	}
	else if (ActivationEnemyAI == false)
	{
		if (tiredness > 0) {
			std::cout << "BOT 2 Tiredness " << tiredness << std::endl;
		}
		if (findNearestResource(CAVE) < 2) {
			tiredness += delta * 0.5f; // If Ur Close To The Cave
		}
		else {
			tiredness -= delta * 0.02f; // If Ur Not Close To The Cave
		}
		if (tiredness <= 0) {
			tiredness = 0;
			InnocentSpeed = 0;
			std::cout << "BOT 2 - Out Of Energy!" << std::endl;
		}
		if (tiredness > 1) {
			tiredness = 1;
		}
	}
	#pragma endregion
	//---------------------------------------//
	#pragma region Health Section:
	// Health
	if (ActivationEnemyAI == false) // Needs to be false
	{
		if (InnocentHealth > 0) {
			std::cout << "BOT 2 Health " << InnocentHealth << std::endl;
		}
		if (findNearestResource(SIMPLE_AI) < 1)
		{
			InnocentHealth -= delta * 0.2f;
		}
		if (InnocentHealth <= 0)
		{
			InnocentHealth = 0;
			InnocentSpeed = 0;
			std::cout << "BOT 2 - Dead!" << std::endl;
		}
		if (InnocentHealth > 1)
		{
			InnocentHealth = 1;
		}
	}
	if (ActivationEnemyAI == true)
	{
		if (EnemyHealth > 0) {
			std::cout << "BOT 1 Health " << EnemyHealth << std::endl;
			EnemyHealth -= delta * 0.02f;
		}
		if (findNearestResource(SIMPLE_AI) < 1)
		{
			EnemyHealth += delta * 0.07f;
		}
		if (EnemyHealth <= 0)
		{
			EnemyHealth = 0;
			EnemySpeed = 0;
			std::cout << "BOT 1 - Dead!" << std::endl;
		}
		if (EnemyHealth > 1)
		{
			EnemyHealth = 1;
		}
	}
	#pragma endregion
	//---------------------------------------//
	#pragma region Safe Section:
	if (ActivationEnemyAI == false)
	{
		if (findNearestResource(SAFEZONE) < 2)
		{
			InnocentSafe = true;
			if (InnocentHealth >= 1)
			{
				InnocentHealth = 1;
			}
			else
			{
				InnocentHealth += delta * 0.3f;
			}
			if (InnocentSafe == true)
			{
				std::cout << "Innocent SAFE" << std::endl;
			}
		}
		else
		{
			InnocentSafe = false;
		}
	}
	#pragma endregion
	//===----------------------------------------------------------------------------===//
}

void WorldController::update(float delta)
{
	for(auto worldObject:worldObjects) {
		worldObject->update(delta);
	}
}

void WorldController::draw()
{
	for(auto worldObject:worldObjects) {
		worldObject->draw();
	}
}

void WorldController::addObject(WorldObject* newObject)
{
	newObject->worldObjects = &worldObjects;
	worldObjects.push_back(newObject);
}