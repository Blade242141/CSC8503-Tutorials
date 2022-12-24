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

	initPos = GetTransform().GetPosition();

	//State* stateA = new State([&](float dt)->void {
	//	this->MoveLeft(dt);
	//	});
	//State* stateB = new State([&](float dt)->void {
	//	this->MoveRight(dt);
	//	});

	//stateMachine->AddState(stateA);
	//stateMachine->AddState(stateB);

	//stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool {return this->counter > 3.0f;}));
	//stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool { return this->counter < 0.0f; }));

	State* stateA = new State([&](float dt)->void {
		this->MoveUp(dt);
		});
	State* stateB = new State([&](float dt)->void {
		this->MoveDown(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool {return this->GetTransform().GetPosition().y > 20.0f; }));
	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool { return this->GetTransform().GetPosition().y < 0.0f && counter > 5.0f; }));
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
	GetPhysicsObject()->AddForce({ 0, 250, 0 });
	counter = 0.0f;
}

void StateGameObject::MoveDown(float dt) {
	GetPhysicsObject()->AddForce({ 0, -10, 0 });
	if (this->GetTransform().GetPosition().y < 0.0f) { counter += dt; }
}