#pragma once

#include "../gameObject.h"
#include "../VulkanTest/vulkanWindow.h"

namespace lve {
	class KeyboardMovementCTRL {


	public:
		struct KeyMappings
		{
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackwards = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};

		void MoveInPlaneXZ(GLFWwindow* window, float dt, GameObject &gameObject);

		KeyMappings keys{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
	};
}