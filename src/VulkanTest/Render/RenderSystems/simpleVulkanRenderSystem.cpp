#include <stdexcept>
#include <array>
//remove later
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "simpleVulkanRenderSystem.h"
#include "../../../timeCheck.h"

namespace lve {

	struct SimplePushConstantData {
		//glm::mat4 transform{1.f};
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f}; //Useful if I want non uniform scaling
	};

	SimpleVulkanRenderSystem::SimpleVulkanRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : engineDevice{device} {
		createPipelineLayout(globalSetLayout);
		createpipeline(renderPass);
	}

	SimpleVulkanRenderSystem::~SimpleVulkanRenderSystem() {
		vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);
	}
	void SimpleVulkanRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutData;
		pipelineLayoutData.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutData.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutData.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutData.pushConstantRangeCount = 1;
		pipelineLayoutData.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutData, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void SimpleVulkanRenderSystem::createpipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigData pipelineConfig{};
		VulkanPipeline::DefaultPipelineConfigData(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vulkanPipeline = std::make_unique<VulkanPipeline>(
			engineDevice,
			"src/VulkanTest/ShaderFolder/simpleShader.vert.spv",
			"src/VulkanTest/ShaderFolder/simpleShader.frag.spv",
			pipelineConfig
		);
	}
	void SimpleVulkanRenderSystem::RenderGameObjects(FrameData& frameData) {
		//Timer time;
		vulkanPipeline->bind(frameData.commandBuffer);

		vkCmdBindDescriptorSets(
			frameData.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 
			1,
			&frameData.globalDescriptorSet,
			0,
			nullptr
		);

		//auto projectionView = frameData.camera.GetProjectionMatrix() * frameData.camera.GetViewMatrix();

		for (auto& kv : frameData.gameObjects	) {
			auto& object = kv.second;

			SimplePushConstantData push{};
			push.modelMatrix = object.transform.mat4();	
			//Useful if I want non uniform scaling
			push.normalMatrix = object.transform.NormalMatrix();


			//push.transform = projectionView * modelMatrix;
			//push.modelMatrix = modelMatrix;

			

			vkCmdPushConstants
			(
				frameData.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			object.model->Bind(frameData.commandBuffer);
			object.model->Draw(frameData.commandBuffer);
		}
	}
}