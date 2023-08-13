#pragma once

#include "../Camera&Movement/vulkanCamera.h"
#include "../../gameObject.h"

#include <vulkan/vulkan.h>

namespace lve {
	struct FrameData {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& gameObjects;
	};
}