#ifndef CHARACTER
#define CHARACTER

#include "Camera.H"

class Character {
public:
	Character(bool);
	Camera camera;
	glm::vec3 position;
	bool isCharacter = false;
	bool FocusPosition = false;
	bool FocusCamera = false;
	int  scene;
	void update();

};
#endif // !CHARACTER
