#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void {
		this->MoveLeft(dt);
		});
	State* stateB = new State([&](float dt)->void {
		this->MoveRight(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool {return this->counter > 3.0f;}));
	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool { return this->counter < 0.0f; }));

	//State* stateA = new State([&](float dt)->void {
	//	this->MoveUp(dt);
	//	});
	//State* stateB = new State([&](float dt)->void {
	//	this->MoveDown(dt);
	//	});

	//stateMachine->AddState(stateA);
	//stateMachine->AddState(stateB);

	//stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool {return this->GetTransform().GetPosition().y > 26.0f; }));
	//stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool { return this->GetTransform().GetPosition().y < 0.0f; }));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -1, 0, 0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 1, 0, 0 });
	counter += dt;
}

//Added
void StateGameObject::MoveUp(float dt) {
	std::cout << "UP" << std::endl;
	GetPhysicsObject()->AddForce({ 0, 20000, 0 });
}

void StateGameObject::MoveDown(float dt) {
	std::cout << "Down" << std::endl;
	GetPhysicsObject()->AddForce({ 0, -20, 0 });
}