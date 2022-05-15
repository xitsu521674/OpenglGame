#include "Character.h"

Character::Character(bool isCharacter) {
    this->isCharacter = isCharacter;
    position = glm::vec3(0, 0, 0);
}
void Character::update() {
    if (!FocusPosition) {
        if (isCharacter) {
            glm::vec3 originPos = position;
            glm::vec3 tmpFront = glm::vec3(camera.Front.x, 0, camera.Front.z);
            glm::vec3 tmpRight = glm::vec3(camera.Right.x, 0, camera.Right.z);
            if (camera.wHold)
                position += tmpFront * camera.MovementSpeed;
            if (camera.sHold)
                position -= tmpFront * camera.MovementSpeed;
            if (camera.aHold)
                position -= tmpRight * camera.MovementSpeed;
            if (camera.dHold)
                position += tmpRight * camera.MovementSpeed;
            if (scene == 1) {
                if ((position.x > 181) || (position.x < -181) || (position.z > 405) || (position.z) < 200)
                    position = originPos;
            }
            else if (scene == 2) {
                
            }
            
            
        }
        else {
            if (camera.wHold)
                position += camera.Front * camera.MovementSpeed;
            if (camera.sHold)
                position -= camera.Front * camera.MovementSpeed;
            if (camera.aHold)
                position -= camera.Right * camera.MovementSpeed;
            if (camera.dHold)
                position += camera.Right * camera.MovementSpeed;
        }
    }
    camera.Position = position;
    if(isCharacter)
        camera.Position.y += 40.0f;
    camera.update();

    
}