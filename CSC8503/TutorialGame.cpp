#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "PlayerGameObject.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = true;
	physics->UseGravity(useGravity);

	inSelectionMode = false;

	isDebug = false;

	gm = none;

	InitialiseAssets();
}

void TutorialGame::MainMenu() {
	Debug::Print("Goat Game", Vector2(40, 10));
	Debug::Print("Select GameMode:", Vector2(40, 20));
	Debug::Print("(1) : Standard ", Vector2(40, 30));
	Debug::Print("(2) : Speed Run", Vector2(40, 40));

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		timer = 120.0f;
		gm = standard;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
		timer = 0.0f;
		gm = speedrun;
	}
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	if (isDebug)
		InitWorld();
	else
		InitGame();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::M)) {
		isDebug = !isDebug;
		std::cout << "Debug: " << isDebug << std::endl;
	}
	switch (gm)
	{
	case NCL::CSC8503::TutorialGame::none:
		MainMenu();
		break;
	case NCL::CSC8503::TutorialGame::standard:
		UpdateGMStandard(dt);
		break;
	case NCL::CSC8503::TutorialGame::speedrun:
		UpdateGMSpeedrun(dt);
		break;
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);
	if (!gameOver)
		physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);

	if (isDebug) {
		world->GetMainCamera()->UpdateCamera(dt);

		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset;

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera()->SetPosition(camPos);
			world->GetMainCamera()->SetPitch(angles.x);
			world->GetMainCamera()->SetYaw(angles.y);

		}

		UpdateKeys();

		if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
		}
		else {
			Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
		}

		RayCollision closestCollision;
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
			Vector3 rayPos;
			Vector3 rayDir;

			rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

			rayPos = selectionObject->GetTransform().GetPosition();

			Ray r = Ray(rayPos, rayDir);

			if (world->Raycast(r, closestCollision, true, selectionObject)) {
				if (objClosest) {
					objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				}
				objClosest = (GameObject*)closestCollision.node;

				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
		}

		Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		SelectObject();
		MoveSelectedObject();
	}
}

void TutorialGame::UpdateGMStandard(float dt) {
	if (!gameOver)
		UpdateGeneral(dt);

	int mins = std::floor((int)round(timer) / 60);
	int secs = (int)round(timer) - (mins * 60);
	string tmp;
	secs <= 9 ? tmp = "0" + std::to_string(secs) : tmp = std::to_string(secs);

	if (!gameOver) {
		timer -= dt;

		Debug::Print("Time Left:" + std::to_string(mins) + ":" + tmp, Vector2(60, 25));

		if (*player->GetTargetInt() <= 0 || timer <= 0) {
			gameOver = true;
		}
	}
	else {
			Debug::Print("Goat Sim", Vector2(40, 10));
			Debug::Print("Game Over!", Vector2(40, 20));
			if (timer <= 0)
				Debug::Print("Loser! ", Vector2(40, 30));
			else
				player->GetScore() < 0 ? Debug::Print("Loser! ", Vector2(40, 30)) : Debug::Print("Winner! ", Vector2(40, 30));
			Debug::Print("Score:" + std::to_string(player->GetScore()), Vector2(40, 40));
			Debug::Print("Time Left:" + std::to_string(mins) + ":" + tmp, Vector2(40, 50));
			Debug::Print("Press The Space Bar!", Vector2(40, 60));

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
			ResetGame();
		}
	}
}

void TutorialGame::UpdateGMSpeedrun(float dt) {
	if (!gameOver)
		UpdateGeneral(dt);

	int mins = std::floor((int)round(timer) / 60);
	int secs = (int)round(timer) - (mins * 60);
	string tmp;
	secs <= 9 ? tmp = "0" + std::to_string(secs) : tmp = std::to_string(secs);

	if (!gameOver) {
		timer += dt;
		Debug::Print("Time Taken:" + std::to_string(mins) + ":" + tmp, Vector2(60, 25));


		if (*player->GetTargetInt() <= 0 || timer >= 600) { // Limited to 10 mins
			gameOver = true;
		}
	}
	else {
		Debug::Print("Goat Sim", Vector2(40, 10));
		Debug::Print("Game Over!", Vector2(40, 20));
		if (timer >= 600)
			Debug::Print("Some how LOST?! ", Vector2(40, 30));
		else
			Debug::Print("Winner! ", Vector2(40, 30));
		Debug::Print("Time Taken:" + std::to_string(mins) + ":" + tmp, Vector2(40, 40));
		Debug::Print("Press The Space Bar!", Vector2(40, 50));
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
			ResetGame();
		}
	}
}

void TutorialGame::UpdateGeneral(float dt) {
	Vector3 objPos = player->GetTransform().GetPosition();
	Vector3 camPos = objPos + (lockedOffset);

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler();

	float yAngle = (player->GetTransform().GetOrientation().ToEuler().y / 180 * 360) / 2; // Convert Y angle to 0-360 scale

	if (yAngle < 0) { yAngle = yAngle + 180 + 180; } // Convert stop 180 jumping to 360

	PlayerMovement();

	yAngle = (yAngle * 3.14159265 / 180); // Get yAngle ready for pos

	float radius = 20;

	// Set position offset on circle based on yAngle
	Vector3 pos = Vector3(
		(radius * sin(yAngle)) + player->GetTransform().GetPosition().x,
		player->GetTransform().GetPosition().y + 10,
		(radius * cos(yAngle)) + player->GetTransform().GetPosition().z
	);

	world->GetMainCamera()->SetPosition(pos);
	world->GetMainCamera()->SetPitch(angles.x + 25);
	world->GetMainCamera()->SetYaw(player->GetTransform().GetOrientation().ToEuler().y); // Now Rotates with goat

	//world->UpdateWorld(dt);
	//renderer->Update(dt);
	//physics->Update(dt);

	//renderer->Render();
	//Debug::UpdateRenderables(dt);

	if (player->GetTransform().GetPosition().y <= -50)
		RespawnPlayer();

	if (liftStateObj)
		liftStateObj->Update(dt);

	for (int i = 0; i < people.size(); ++i) {
		people[i]->Update(dt);
	}

		Debug::Print("Score:" + std::to_string(player->GetScore()), Vector2(10, 20));
		Debug::Print("Targets:" + std::to_string(*player->GetTargetInt()), Vector2(60, 20));
}

void TutorialGame::ResetGame() {
	if (gm != none) {
		targets.clear();
		people.clear();
		gameOver = false;
		InitGame();
		gm = none;
	}
}

void TutorialGame::RespawnPlayer() {
	player->GetTransform().SetPosition(Vector3(0, 2.5, 0));
}

void TutorialGame::PlayerMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::W))
		player->GetPhysicsObject()->AddForce(fwdAxis * forceMagnitude);

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::A))
		player->GetPhysicsObject()->AddForce(-rightAxis * forceMagnitude);

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::D))
		player->GetPhysicsObject()->AddForce(rightAxis * forceMagnitude);

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::S))
		player->GetPhysicsObject()->AddForce(-fwdAxis * forceMagnitude);

	//Add Dash Here
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::SHIFT))
		player->GetPhysicsObject()->ApplyLinearImpulse(fwdAxis * 30.0f);
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::SHIFT))
		player->SetIsAttacking(true);
	else 
		player->SetIsAttacking(false);

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE) && player->GetPhysicsObject()->GetLinearVelocity().y <= 1.0f)
		player->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 20, 0));


	//Rotation
	player->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::Q))
		player->GetPhysicsObject()->AddTorque(Vector3(0, 5, 0));
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::E))
		player->GetPhysicsObject()->AddTorque(Vector3(0, -5, 0));
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::LEFT))
		player->GetPhysicsObject()->ApplyAngularImpulse(Vector3(0, 2, 0));
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::RIGHT))
		player->GetPhysicsObject()->ApplyAngularImpulse(Vector3(0, -2, 0));
	
	//if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::SPACE)) {
	//	Ray ray = Ray(player->GetTransform().GetPosition(), player->GetTransform().GetOrientation().ToEuler().Normalised());

	//	Debug::DrawLine(ray.GetPosition(), Vector3(ray.GetDirection().x + 10, 0, 0), Vector4(1, 1, 1, 1), 10.0f);
	//}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//testStateObj = AddStateObjToWorld(Vector3(-10, 10, -10), Vector3(2, 2, 2), 10.0f);

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	BridgeConstraintTest();

	//InitMaze();

	//InitGameExamples();
	InitPlayer();

	InitDefaultFloor();
}

void TutorialGame::InitGame() {
	world->ClearAndErase();
	physics->Clear();


	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	
	//BridgeConstraintTest();
	InitPlayer();
	InitGameWorld();
	liftStateObj = AddStateObjToWorld(Vector3(10, 0, -75), Vector3(5, 0.5, 5), 0.1f, 0.0f);
	InitTargets();
	InitDefaultFloor();
	InitMixedGridWorld(7, 7, 3.5f, 3.5f);
	GameObject* obj = AddCubeToWorld(Vector3(60, 22, -75), Vector3(1, 1, 1), 0.5, 0.4, true);
	obj->SetPoints(250);
	obj->SetIsBonus(true);
	obj->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	AddPersonToWorld({ 10, 0, 10 }, { 1, 3, 1 }, true);
	AddPersonToWorld({ -10, 0, -10 }, { 1, 3, 1 }, true);
	AddPersonToWorld({ 35, 0, -10 }, { 1, 3, 1 }, true);
	AddPersonToWorld({ -20, 0, 50 }, { 1, 3, 1 }, false);
	AddPersonToWorld({ -50, 0, 100 }, { 1, 3, 1 }, false);

}

void TutorialGame::InitPlayer() {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	player = new PlayerGameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(Vector3(10, 0, 0));

	player->SetRenderObject(new RenderObject(&player->GetTransform(), charMesh, nullptr, basicShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	world->GetMainCamera()->SetPosition(Vector3(player->GetTransform().GetPosition().x, player->GetTransform().GetPosition().y + 5, player->GetTransform().GetPosition().z + 10));

	//lockedObject = player;
	player->GetPhysicsObject()->SetElasticity(0);

	world->SetPlayerObj(player);
}

void TutorialGame::InitGameWorld() {
	AddCubeToWorld(Vector3(-40, -15, -15), Vector3(5, 5, 50), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Connection wall
	//AddCubeToWorld(Vector3(-80, -12, 100), Vector3(7.5, 2, 25), 0, 1)->GetTransform().SetOrientation(Quaternion().EulerAnglesToQuaternion(-14, 125, -10)); // Connection Ramp	
	//AddCubeToWorld(Vector3(-15, 5, -50), Vector3(8.5, 2, 40), 0, 1)->GetTransform().SetOrientation(Quaternion().EulerAnglesToQuaternion(-32, 90, 0)); // Upper Ramp
	AddCubeToWorld(Vector3(-180, -7.5, 12.1), Vector3(2.5, 5, 27.5), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Cross 1
	GameObject* obj = AddCubeToWorld(Vector3(-160, -7.5, 17.5), Vector3(2.5, 5, 27.5), 0, 1);
	//AddCubeToWorld(Vector3(100, 0, 100), Vector3(40, 5, 2.5), 10.0f, 1)->GetRenderObject()->SetColour(Vector4(0.2, 0.2, 1, 1));

	AddCubeToWorld(Vector3(60, 20.06, -75), Vector3(40, 5, 40), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Upper level A
	AddCubeToWorld(Vector3(60, 0, -75), Vector3(10, 20, 10), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Pillar A

	SpawnConnectionBridge(Vector3(60, 20.06, -35));

	AddCubeToWorld(Vector3(60, 20.06, 200), Vector3(40, 5, 40), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Upper level B
	AddCubeToWorld(Vector3(60, 0, 200), Vector3(10, 20, 10), 0, 1)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1)); // Pillar B

	//AddOBBCubeToWorld(Vector3(0, 0, -10), Vector3(5, 5, 5), 10.0, 0.4, false)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
}

void TutorialGame::InitTargets() {
	Vector3 cubeSize = Vector3(2, 2, 2);
	float inverseMass = 0.5;
	float elastisity = 0.4;
	bool canTakeDmg = true;
	bool isTarget = true;

	targets.push_back(AddCubeToWorld(Vector3(5, 5, 5), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));
	targets.push_back(AddCubeToWorld(Vector3(70, 5, -60), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));
	targets.push_back(AddCubeToWorld(Vector3(130, 5, 55), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));
	targets.push_back(AddCubeToWorld(Vector3(40, 5, -70), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));;
	targets.push_back(AddCubeToWorld(Vector3(-80, 5, 140), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));
	targets.push_back(AddCubeToWorld(Vector3(-20, 5, -90), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));
	targets.push_back(AddCubeToWorld(Vector3(-180, 5, 90), cubeSize, inverseMass, elastisity, canTakeDmg, isTarget));

	for (int i = 0; i < targets.size(); ++i) {
		targets[i]->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	}
	targetsLeft = new int (targets.size());
	player->SetTargetInt(targetsLeft);
}

void TutorialGame::SpawnConnectionBridge(Vector3 startPos) {
	Vector3 cubeSize = Vector3(8, 8, 8);
	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 4;
	float maxDistance = 50; // constraint distance
	float cubeDistance = 2.5; // distance between links

	//Vector3 startPos = Vector3(0, 0, -75);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3(0, 0, (numLinks + 35*2.5) * cubeDistance), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3(0, 0, (i + 1) * cubeDistance), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(250, 2, 250);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

//StateGameObject* TutorialGame::AddPersonToWorld(const Vector3& position) {
//	float meshSize = 3.0f;
//	float inverseMass = 0.5f;
//
//	GameObject* character = new GameObject();
//
//	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
//	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(character);
//
//	return character;
//}

PersonGameObject* TutorialGame::AddPersonToWorld(const Vector3& position, Vector3 dims, bool passive) {
	float inverseMass = 0.5f;

	PersonGameObject* character = new PersonGameObject();

	AABBVolume* volume = new AABBVolume(dims);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetPosition(position)
		.SetScale(dims * 2);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), cubeMesh, basicTex, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();


	character->GetPhysicsObject()->SetElasticity(0.01);
	character->SetCanTakeDmg(true);
	character->SetPoints(100);

	character->SetPlayer(player);

	if (passive) {
		character->GetRenderObject()->SetColour({ 0, 0, 1, 1 });
		character->SetCanAttackPlayer(false);
	}
	else {
		character->GetRenderObject()->SetColour({ 1, 0, 0, 1 });
		character->SetCanAttackPlayer(true);
	}


	people.push_back(character);
	character->SetWorldStart({ -120, 0, -120 });
	character->SetWorldEnd({ 120, 0, 120 });
	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjToWorld(const Vector3& pos, Vector3 dimensions, float inverseMass, float elasticity) {
	StateGameObject* cube = new StateGameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(pos)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);


	world->AddGameObject(cube);

	return cube;
}

#pragma region Stuff

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, float elasticity, bool canTakedmg) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	// Different material elasticities
	//float random = ((float)rand()) / (float)RAND_MAX;
	//float diff = 1.5 - 0.5;
	//sphere->GetPhysicsObject()->SetElasticity(0.01 + (random * diff));
	sphere->GetPhysicsObject()->SetElasticity(elasticity);
	sphere->SetCanTakeDmg(canTakedmg);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity, bool canTakedmg, bool isTarget) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	// Different material elasticities
	cube->GetPhysicsObject()->SetElasticity(elasticity);
	cube->SetCanTakeDmg(canTakedmg);
	cube->SetIsTarget(true);
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity, bool canTakedmg) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	// Different material elasticities
	cube->GetPhysicsObject()->SetElasticity(elasticity);
	cube->SetCanTakeDmg(canTakedmg);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddPersonToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.5f;
	Vector3 cubeDims = Vector3(1.5f, 1.5f, 1.5f);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, 10.f, ((rand() % 100)/100), false, false);
			}
			else {
				AddSphereToWorld(position, sphereRadius, 10.f, ((rand() % 100) / 100), false);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::W))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(1, 0, 0) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::A))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(0, 0, -1) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::S))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(-1, 0, 0) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::D))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(0, 0, 1) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::SPACE))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(0, 1, 0) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::SHIFT))
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(0, -1, 0) * forceMagnitude, selectionObject->GetTransform().GetPosition());

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::L))
		physics->ToggleUseBroadPhase();
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(4, 4, 4);
	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(0, 0, -75);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		OrientationConstraint* ori = new OrientationConstraint(previous, block);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	OrientationConstraint* ori = new OrientationConstraint(previous, end);

	world->AddConstraint(constraint);
	world->AddConstraint(ori);
}
#pragma endregion
