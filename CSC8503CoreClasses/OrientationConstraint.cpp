#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b)
{
	objectA = a;
	objectB = b;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {
	//Vector3 relevantRot = objectA->GetTransform().GetOrientation().ToEuler() - objectB->GetTransform().GetOrientation().ToEuler();

	//float currentRot = relevantRot.Length();
	//float offset = distance - currentRot;

	//if (abs(offset) > 0.0f) {
	//	Vector3 offsetDir = relevantRot.Normalised();

	//	PhysicsObject* physA = objectA->GetPhysicsObject();
	//	PhysicsObject* physB = objectB->GetPhysicsObject();

	//	Vector3 relativeVelocity = physA->GetAngularVelocity() - physB->GetAngularVelocity();

	//	float constrainMass = physA->GetInverseMass() + physB->GetInverseMass();

	//	if (constrainMass > 0.0f) {
	//		// how much of their relative force is affecting the constraint
	//		float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);

	//		float biasFactor = 0.1f;
	//		float bias = -(biasFactor / dt) * offset;

	//		float lambda = -(velocityDot + bias) / constrainMass;

	//		Vector3 aImpulse = offsetDir * lambda;
	//		Vector3 bImpulse = -offsetDir * lambda;

	//		physA->ApplyAngularImpulse(aImpulse); // Multiply by mass
	//		physB->ApplyAngularImpulse(bImpulse); // Multiply by mass
	//	}
	//}
}