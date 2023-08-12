#pragma once

#include "vulkanCamera.h"

#include <vulkan/vulkan.h>

namespace lve {
	struct FrameData {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera &camera;
		VkDescriptorSet globalDescriptorSet;
	};
}