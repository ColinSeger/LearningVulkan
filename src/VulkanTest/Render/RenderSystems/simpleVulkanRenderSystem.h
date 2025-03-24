#pragma once

#include <memory>
#include <vector>

#include "../../Camera&Movement/vulkanCamera.h"
#include "../Pipeline/vulkanPipeline.h"
#include "../vulkanDevice.h"
#include "../../../gameObject.h"
#include "../vulkanFrameData.h"

namespace lve {
	class SimpleVulkanRenderSystem {

		VulkanDevice& engineDevice;

		std::unique_ptr<VulkanPipeline> vulkanPipeline;

		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createpipeline(VkRenderPass renderPass);

	public:

		SimpleVulkanRenderSystem(VulkanDevice& device, VkRenderPass renderPas, VkDescriptorSetLayout globalSetLayout);
		~SimpleVulkanRenderSystem();

		SimpleVulkanRenderSystem(const SimpleVulkanRenderSystem&) = delete;
		SimpleVulkanRenderSystem& operator=(const SimpleVulkanRenderSystem&) = delete;
		
		void RenderGameObjects(FrameData& frameData);

	};


}