#include "PersonGameObject.h"
#include "PhysicsObject.h"


using namespace NCL;
using namespace CSC8503;

PersonGameObject::PersonGameObject() {
	counter = 0.0f;
	viewRange = 20.0f;
	attackRange = 4.0f;
	waitPointRange = 5.0f;

	attackCoolDown = 2.0f;

	//stateMachine = new StateMachine();

	//initPos = GetTransform().GetPosition();
	root = CreateRoot();
	root->Reset();
	state = Ongoing;

	ResetTimer();
}

PersonGameObject::~PersonGameObject() {
}

void PersonGameObject::Update(float dt) {
	//stateMachine->Update(dt);
		state = root->Execute(dt);
		if (state == Failure)
			root->Reset();

		if (transform.GetPosition().y <= -50) {
			root->Reset();
			transform.SetPosition({ 0, 0, 0 });
		}
}

BehaviourSequence* PersonGameObject::CreateRoot() {
	BehaviourAction* wounder = new BehaviourAction("wounder", [&](float dt, BehaviourState state)->BehaviourState {
		if (IsPlayerClose())
			return Success;

		if (state == Initialise) {
			//Move towards random point
			SetTargetPosition(Vector3(
				worldStart.x + (rand() % (int)round(worldEnd.x - worldStart.x + 1)),
				-15,
				worldStart.z + (rand() % (int)round(worldEnd.z - worldStart.z + 1))
			));
			StopMoveing();

			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (IsCloseToWaitPoint()) {
				return Success;
			}
			else {
				MoveTowardsTargetPos();
				}
			}
		return state; // will return ongoing until person gets to wait point
		});

	BehaviourAction* wait = new BehaviourAction("wait", [&](float dt, BehaviourState state)->BehaviourState {
		if (IsPlayerClose())
			return Success;

		if (state == Initialise) {
			//Start waiting, reset variabels if needed
			StopMoveing();
			ResetTimer();
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (timer <= 0) {
				//return success, start woundering again
				return Failure;
			}
			else {
				timer -= dt;
			}
		}
		return state; // will return ongoing until timer runs out
		});

	chasePlayerSeq = new BehaviourSequence("Chase Player");

	BehaviourAction* runFromPlayer = new BehaviourAction("run from player", [&](float dt, BehaviourState state)->BehaviourState {
		if (canAttackPlayer)
			chasePlayerSeq->Execute(dt);

		if (state == Initialise) {
			SetTargetPosition(RunFromPlayerTarget());

			state = Ongoing;
		}
		else if (state == Ongoing) {
			MoveTowardsTargetPos();
			if (IsPlayerClose()) {
					SetTargetPosition(RunFromPlayerTarget());
			}
			else {
				return Failure;
			}
		}
		return state; // will return ongoing until player is no longer within range
		});

	BehaviourAction* chasePlayer = new BehaviourAction("chase player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			SetTargetPosition(player->GetTransform().GetPosition());
			//chase player
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (IsWithInAttackRange())
			return Success;
		}
		else if (!IsPlayerClose()) {

			return Failure;
		}
		else {
			SetTargetPosition(player->GetTransform().GetPosition());
			MoveTowardsTargetPos();
		}
		return state; // will return ongoing until player is no longer within range
		});

	BehaviourAction* attackPlayer = new BehaviourAction("attack player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			SetTargetPosition(player->GetTransform().GetPosition());
			//chase player
			attackCoolDownTimer = attackCoolDown;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			SetTargetPosition(player->GetTransform().GetPosition());
			MoveTowardsTargetPos();

			if (IsWithInAttackRange() && attackCoolDownTimer <= 0 && canAttackPlayer) {
				player->TakeScoreDamage();
				attackCoolDownTimer = attackCoolDown;
			}
			else if (IsWithInAttackRange() && canAttackPlayer) {
				physicsObject->ClearForces();
				attackCoolDownTimer -= dt;
			}
			else {
				return Failure;
			}
		}
		return state; // will return ongoing until player is no longer within range
		});

	BehaviourSequence* seq = new BehaviourSequence("Idle Seq");
	seq->AddChild(wounder);
	seq->AddChild(wait);

	chasePlayerSeq->AddChild(chasePlayer);
	chasePlayerSeq->AddChild(attackPlayer);

	BehaviourSelector* sel = new BehaviourSelector("Player Interaction");
	sel->AddChild(runFromPlayer);
	sel->AddChild(chasePlayerSeq);

	BehaviourSequence* rootSeq = new BehaviourSequence("Root Sel");
	rootSeq->AddChild(seq);
	rootSeq->AddChild(sel);
	
	return rootSeq;
}

bool PersonGameObject::IsPlayerClose() {
	if (DistanceFromVector3(player->GetTransform().GetPosition()) <= viewRange)
		return true;
	else
		return false;
}

bool PersonGameObject::IsWithInAttackRange() {
	if (DistanceFromVector3(player->GetTransform().GetPosition()) <= viewRange)
		return true;
	else
		return false;
}

bool PersonGameObject::IsCloseToWaitPoint() {
	if (DistanceFromVector3(player->GetTransform().GetPosition()) <= waitPointRange)
		return true;
	else
		return false;
}

float PersonGameObject::DistanceFromVector3(Vector3 v) {
	Vector3 diff = Vector3(
		v.x - this->GetTransform().GetPosition().x, 
		v.y - this->GetTransform().GetPosition().y,
		v.z - this->GetTransform().GetPosition().z
	);
	return (_CMATH_::sqrt(_CMATH_::pow(diff.x, 2.0f) + _CMATH_::pow(diff.y, 2.0f) + _CMATH_::pow(diff.z, 2.0f)));
}

void PersonGameObject::ResetTimer() {
	timer = (rand() % 5) + 5;
}

void PersonGameObject::SetTargetPosition(Vector3 t) {
	targetPos = t;
}

void PersonGameObject::MoveTowardsTargetPos() {
	Vector3 v;
	if (targetPos.x > transform.GetPosition().x)
		v.x = 1;
	if (targetPos.x < transform.GetPosition().x)
		v.x = -1;

	v.y = 0;

	if (targetPos.z > transform.GetPosition().z)
		v.z = 1;
	if (targetPos.z < transform.GetPosition().z)
		v.z = -1;

	physicsObject->AddForce(v*10);
}

Vector3 PersonGameObject::RunFromPlayerTarget() {
	Vector3 t;
	if (player->GetTransform().GetPosition().x < transform.GetPosition().x) {
		//give higher value
		t.x = transform.GetPosition().x - (rand() % (int)round(worldEnd.x - transform.GetPosition().x + 1));
	}
	else {
		//give lower value
		t.x = t.x = worldStart.x - (rand() % (int)round(transform.GetPosition().x - worldStart.x + 1));
	}

	t.y = -15;

	if (player->GetTransform().GetPosition().z < transform.GetPosition().z) {
		//give higher value
		t.z = transform.GetPosition().z - (rand() % (int)round(worldEnd.z - transform.GetPosition().z + 1));
	}
	else {
		//give lower value
		t.z = t.z = worldStart.z - (rand() % ((int)round(transform.GetPosition().z - worldStart.z) + 1));
	}

	return t;
}

void PersonGameObject::OnCollisionBegin(GameObject* gm) {
	if (gm->GetWorldID() != player->GetWorldID()) {

	}
}



void PersonGameObject::StopMoveing() {
	physicsObject->ClearForces();
}