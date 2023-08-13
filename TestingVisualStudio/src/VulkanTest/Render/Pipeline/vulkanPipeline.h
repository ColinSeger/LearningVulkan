#pragma once
#include <string>
#include <vector>

#include "../vulkanDevice.h"

namespace lve {

	struct PipelineConfigData {
		PipelineConfigData() = default;
		PipelineConfigData(const PipelineConfigData&) = delete;
		PipelineConfigData& operator=(const PipelineConfigData&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyData;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline {

		static std::vector<char> readFile(const std::string& filepath);
		VulkanDevice& vulkanDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		
		void createGraphicsPipeline(
			const std::string& vertFilePath, 
			const std::string& fragFilePath,
			const PipelineConfigData& configData
		);

		void createShaderModule(
			const std::vector<char>& code,
			VkShaderModule* shaderModule
		);

	public:
		VulkanPipeline(
			VulkanDevice &device, 
			const std::string &vertFilePath, 
			const std::string &fragFilePath, 
			const PipelineConfigData &configData
		);
		~VulkanPipeline();

		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void DefaultPipelineConfigData(PipelineConfigData& configData);
	};
}