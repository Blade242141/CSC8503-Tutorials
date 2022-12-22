#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class PlayerGameObject : public GameObject {
        public:
            PlayerGameObject();
            ~PlayerGameObject();

            virtual void Update(float dt);

            void SetIsAttacking(bool b) { isAttacking = b; }
            void IncreaseScore() { score++; }

            int GetPlayerDmg() { return dmg; }

            bool IsPlayerAttacking() { return isAttacking; }

            void IncreaseScore(int amt) { score += amt; }

            int GetScore() { return score; }


            void SetTargetInt(int* i) { targets = i; }
            int* GetTargetInt() { return targets; }
            void DecreaseTargetInt() { *targets -= 1; }
        protected:
            bool isAttacking;
            int score;
            int dmg;
            int* targets;
        };
    }
}
