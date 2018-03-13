#include "FuzzyLogicProject.h"
#include <iostream>
#include "FuzzyLogicEngine.h"
#include "SimulationObjects.h"

Fuzzy fuzzyEngine;  
using namespace std;

int gScreenWidth;
int gScreenHeight;

FuzzyProject::FuzzyProject() {}

FuzzyProject::~FuzzyProject() {}

bool FuzzyProject::onCreate(int argc, char* argv[])
{
	// initialise the Gizmos helper class
	Gizmos::create(0xffff, 0xffff, 0xffff, 0xffff);

	// set up a camera
	//m_camera = new Camera(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.0f);
	//m_camera->setLookAtFrom(glm::vec3(50, 10, 0), glm::vec3(0, 0, 0));
    
	// set the colour the window is cleared to
	glClearColor(0.25f, 0.25f, 0.25f, 1);

	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	GLFWMouseButton1Down = false;

	glfwGetWindowSize(glfwGetCurrentContext(), &gScreenWidth, &gScreenHeight);
	FuzzyLogicExample();
	return true;
}

void FuzzyProject::FuzzyLogicExample()
{
	//set up simulation world.
	worldController = new WorldController();
	// Water
	Water* water1 = new Water(glm::vec2(400, 400)); // Bot 2
	worldController->addObject(water1);
	AIWater* water2 = new AIWater(glm::vec2(1000, 400)); // Bot 1
	worldController->addObject(water2);
	// Cave
	Cave* cave = new Cave(glm::vec2(400, 200)); // Bot 2
	worldController->addObject(cave);
	AICave* AIcave = new AICave(glm::vec2(1000, 200)); // Bot 1
	worldController->addObject(AIcave);
	// Food
	Food* food = new Food(glm::vec2(400, 600)); // Bot 2
	worldController->addObject(food);
	AIFood* AIfood = new AIFood(glm::vec2(1000, 600)); // Bot 1
	worldController->addObject(AIfood);
	// Safe Zone
	SafeZone* safeZone = new SafeZone(glm::vec2(700, 400));
	worldController->addObject(safeZone);
	// Primary AI
	Agent* simpleAI = new Agent(0, glm::vec2(500, 400), glm::vec4(1, 0, 1, 1)); // Bot 2
	worldController->addObject(simpleAI);
	simpleAI->ActivationEnemyAI = false;
	// Enemy AI
	Agent* enemyAI = new Agent(1, glm::vec2(350, 200), glm::vec4(0, 0, 0, 1)); // Bot 1
	worldController->addObject(enemyAI);
	enemyAI->ActivationEnemyAI = true;
	
	// Membership functions for the tiredess set
	fuzzyEngine.tired = new leftShoulderMembershipFunction(0.2f, 0.4f, "tired");
	fuzzyEngine.awake = new TrapezoidFunction(0.2f, 0.4f, 0.6f, 0.8f, "awake");
	fuzzyEngine.superActive = new rightShoulderMembershipFunction(0.6f, 0.8f, "SuperActive");

	// Membership functions for the hunger set
	fuzzyEngine.veryHungry = new leftShoulderMembershipFunction(0.2f, 0.4f, "very hungry");
	fuzzyEngine.hungry = new TrapezoidFunction(0.2f, 0.4f, 0.8f, 0.9f, "hungry");
	fuzzyEngine.full = new rightShoulderMembershipFunction(0.8f, 0.9f, "full");

	// Membership functions for the thirst set
	fuzzyEngine.WeakFromThirsty = new leftShoulderMembershipFunction(0.1f, 0.3f, "weak from thirst");
	fuzzyEngine.veryThirsty = new TriangleFunction(0.1f, 0.3f, 0.5f, "very thristy");
	fuzzyEngine.thirsty = new TriangleFunction(0.3f, 0.5f, 0.8f, "thristy");
	fuzzyEngine.notThirsty = new rightShoulderMembershipFunction(0.5f, 0.9f, "not thirsty");

	//membership functions for the health set
	fuzzyEngine.Injured = new TrapezoidFunction(0, 0.5f, 0.7f, 0.9f, "feeling weak");
	fuzzyEngine.Healthy = new rightShoulderMembershipFunction(0.7f, 0.9f, "feeling healthy");

	//membership functions for run function
	fuzzyEngine.StartRunning = new TrapezoidFunction(0, 0.5f, 0.7f, 0.9f, "injured must run");
	fuzzyEngine.Stay = new rightShoulderMembershipFunction(0.7f, 0.9f, "feeling healthy");

	// Membership functions for the distance set
	fuzzyEngine.veryNear = new leftShoulderMembershipFunction(2, 4, "very close");
	fuzzyEngine.mediumRange = new TrapezoidFunction(2, 4, 50, 70, "medium range");
	fuzzyEngine.farAway = new rightShoulderMembershipFunction(50, 70, "far away");

	// Membership functions for the desirability set (used for defuzification)
	fuzzyEngine.undesirable = new leftShoulderMembershipFunction(0.3f, 0.5f, "undesirable");
	fuzzyEngine.desirable = new TriangleFunction(0.3f, 0.5f, 0.7f, "desirable");
	fuzzyEngine.veryDesirable = new rightShoulderMembershipFunction(0.5f, 0.7f, "very desirable");
}

void FuzzyProject::onDestroy()
{
	Gizmos::destroy();
}

void FuzzyProject::onUpdate(float deltaTime)
{
	Gizmos::clear();
	worldController->update(deltaTime);
	worldController->draw();
	
	// update the camera
	//m_camera->update(deltaTime);
	// clear all gizmos from last frame
}

void FuzzyProject::onDraw()
{
	// clear the back-buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw the gizmos for this frame
	//Gizmos::draw(m_camera->getProjectionView());

	// get window dimensions for 2D orthographic projection
	int width = 0, height = 0;
	glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, (float)width, 0, (float)height, -1.0f, 1.0f));
}