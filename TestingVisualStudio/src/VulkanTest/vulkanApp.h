#pragma once

#include <memory>
#include <vector>

#include "Render/Window/vulkanWindow.h"
#include "Render/vulkanDevice.h"
#include "../gameObject.h"
#include "Render/Renderer/vulkanRenderer.h"
#include "Render/Descriptors/vulkanDescriptor.h"

namespace lve {
	class vulkanApp{
		LveWindow lveWindow{ WIDTH, HEIGHT, "VulkanTest" };

		VulkanDevice engineDevice{lveWindow};

		VulkanRender vulkanRenderer{ lveWindow, engineDevice };

		//std::vector<GameObject>(gameObjects);

		void LoadGameObjects();

		// note: order of declarations matters
		std::unique_ptr<LveDescriptorPool> globalPool{};
		GameObject::Map gameObjects;

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