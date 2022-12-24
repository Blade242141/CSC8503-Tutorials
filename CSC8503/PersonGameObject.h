#pragma once
#include "GameObject.h"
#include <BehaviourSelector.h>
#include "PlayerGameObject.h"
#include <BehaviourAction.h>
#include <BehaviourSequence.h>
#include <Maths.h>

namespace NCL {
    namespace CSC8503 {
        class PersonGameObject : public GameObject  {
        public:
            PersonGameObject();
            ~PersonGameObject();

            virtual void Update(float dt);
            virtual void OnCollisionBegin(GameObject* otherObject);

            void SetPlayer(PlayerGameObject* p) { player = p; }

            void SetCanAttackPlayer(bool b) {
                canAttackPlayer = b;
            }

            void SetWorldStart(Vector3 v) { worldStart = v; }
            void SetWorldEnd(Vector3 v) { worldEnd = v; }

        protected:
            bool canAttackPlayer;

            BehaviourSequence* CreateRoot();
            BehaviourSequence* root;
            BehaviourSequence* chasePlayerSeq;
            BehaviourState state;

            bool IsPlayerClose();
            bool IsWithInAttackRange();
            bool IsCloseToWaitPoint();
            float DistanceFromVector3(Vector3 v);

            Vector3 RunFromPlayerTarget();
            void StopMoveing();

            float viewRange;
            float attackRange;
            float waitPointRange;
            float timer;
            float attackCoolDownTimer;
            float attackCoolDown;
            void ResetTimer();

            Vector3 targetPos;
            void SetTargetPosition(Vector3 t);
            void MoveTowardsTargetPos();

            float counter;
            PlayerGameObject* player;

            Vector3 worldStart;
            Vector3 worldEnd;
        };
    }
}
