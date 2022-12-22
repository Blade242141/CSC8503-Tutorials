#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(std::string name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		//Added, instead of creating new game obj just to add health
		Vector2 TakeDamage(int dmg, bool playerAttacking) {
			if (canTakeDmg && playerAttacking) {
				std::cout << "Taking dmg - " << health << std::endl;
				health -= dmg;
				if (health <= 0 && !isDead) {
					isActive = false;
					health = -points;
					isDead = true;
				}
				else if (isDead) {
					health = 0;
				}
			}
			return Vector2(health, isDead);
		}

		void SetHealth(int i) { health = i; }
		void SetCanTakeDmg(bool b) { canTakeDmg = b; }
		void SetPoints(int amt) { points = amt; }

	protected:
		//Added, instead of creating new game obj just to add health
		int health;
		bool canTakeDmg;
		int points;
		bool isDead;

		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

