#include "PlayerGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

PlayerGameObject::PlayerGameObject() {
	isAttacking = false;
	score = 0;
	dmg = 100;
}

PlayerGameObject::~PlayerGameObject() {
}

void PlayerGameObject::Update(float dt) {

}

void PlayerGameObject::OnCollisionBegin(GameObject* gm) {
	score += gm->TakeDamage(dmg, isAttacking, targets);
}