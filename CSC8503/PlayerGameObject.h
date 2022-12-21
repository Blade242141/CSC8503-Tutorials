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

        protected:
            bool isAttacking;
            int score;
            int dmg;
        };
    }
}
