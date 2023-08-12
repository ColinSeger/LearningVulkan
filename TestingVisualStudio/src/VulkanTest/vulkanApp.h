#pragma once

#include <memory>
#include <vector>

#include "vulkanWindow.h"
#include "vulkanDevice.h"
#include "../gameObject.h"
#include "vulkanRenderer.h"
#include "vulkanDescriptor.h"

namespace lve {
	class vulkanApp{
		LveWindow lveWindow{ WIDTH, HEIGHT, "VulkanTest" };

		VulkanDevice engineDevice{lveWindow};

		VulkanRender vulkanRenderer{ lveWindow, engineDevice };

		//std::vector<GameObject>(gameObjects);

		void LoadGameObjects();

		// note: order of declarations matters
		std::unique_ptr<LveDescriptorPool> globalPool{};
		std::vector<GameObject> gameObjects;

	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;

		vulkanApp();
		~vulkanApp();

		vulkanApp(const vulkanApp&) = delete;
		vulkanApp& operator=(const vulkanApp&) = delete;

		void Run();

	};


}