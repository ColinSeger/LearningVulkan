#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

#include "vulkanPipeline.h"
#include "../Model/vulkanModel.h"

namespace lve {

	VulkanPipeline::VulkanPipeline(VulkanDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigData& configData) : vulkanDevice{device} {
		createGraphicsPipeline(vertFilePath, fragFilePath, configData);
	}
	VulkanPipeline::~VulkanPipeline() {
		vkDestroyShaderModule(vulkanDevice.device(), vertShaderModule, nullptr);
		vkDestroyShaderModule(vulkanDevice.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(vulkanDevice.device(), graphicsPipeline, nullptr);
	}

	std::vector<char> VulkanPipeline::readFile(const std::string& filepath) {

		std::ifstream file{filepath, std::ios::ate | std::ios::binary};

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanPipeline::createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigData& configData) {
		
		assert(configData.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configData");
		assert(configData.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderpass provided in configData");

		auto vertCode = readFile(vertFilePath);
		auto fragCode = readFile(fragFilePath);

		createShaderModule(vertCode, &vertShaderModule);
		createShaderModule(fragCode, &fragShaderModule);

		//Perhaps try std::array later?
		VkPipelineShaderStageCreateInfo shaderStages[2];

		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;		
		
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;


		auto bindingDescriptions = VulkanModel::Vertex::GetBindingDescriptions();
		auto attributeDescriptions = VulkanModel::Vertex::GetAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputData{};
		vertexInputData.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputData.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputData.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputData.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputData.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineData{};
		pipelineData.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineData.stageCount = 2;
		pipelineData.pStages = shaderStages;
		pipelineData.pVertexInputState = &vertexInputData;
		pipelineData.pInputAssemblyState = &configData.inputAssemblyData;
		pipelineData.pViewportState = &configData.viewportInfo;
		pipelineData.pRasterizationState = &configData.rasterizationInfo;
		pipelineData.pMultisampleState = &configData.multisampleInfo;
		pipelineData.pColorBlendState = &configData.colorBlendInfo;
		pipelineData.pDepthStencilState = &configData.depthStencilInfo;
		pipelineData.pDynamicState = &configData.dynamicStateInfo;

		pipelineData.layout = configData.pipelineLayout;
		pipelineData.renderPass = configData.renderPass;
		pipelineData.subpass = configData.subpass;

		//Apparantly can affect perfomance
		pipelineData.basePipelineIndex = -1;
		pipelineData.basePipelineHandle = VK_NULL_HANDLE;

		//Can be optimized
 		if (vkCreateGraphicsPipelines(vulkanDevice.device(), VK_NULL_HANDLE, 1, &pipelineData, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create grapics pipeline using method in vulkanpipelineFile");
		}

		/*
		std::cout << "vertexShader Code size " << vertCode.size() << '\n';
		std::cout << "fragmentShader Code size " << fragCode.size() << '\n';
		*/
	}

	void VulkanPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(vulkanDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a shader module");
		}
	}

	void VulkanPipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	void VulkanPipeline::DefaultPipelineConfigData(PipelineConfigData& configData)
	{
		configData.inputAssemblyData.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configData.inputAssemblyData.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configData.inputAssemblyData.primitiveRestartEnable = VK_FALSE;

		configData.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configData.viewportInfo.viewportCount = 1;
		configData.viewportInfo.pViewports = nullptr;
		configData.viewportInfo.scissorCount = 1;
		configData.viewportInfo.pScissors = nullptr;

		configData.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configData.rasterizationInfo.depthClampEnable = VK_FALSE;
		configData.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configData.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configData.rasterizationInfo.lineWidth = 1.0f;
		configData.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configData.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configData.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configData.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configData.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configData.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configData.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configData.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configData.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configData.multisampleInfo.minSampleShading = 1.0f;           // Optional
		configData.multisampleInfo.pSampleMask = nullptr;             // Optional
		configData.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configData.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		configData.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configData.colorBlendAttachment.blendEnable = VK_FALSE;
		configData.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configData.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configData.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		configData.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configData.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configData.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		configData.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configData.colorBlendInfo.logicOpEnable = VK_FALSE;
		configData.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configData.colorBlendInfo.attachmentCount = 1;
		configData.colorBlendInfo.pAttachments = &configData.colorBlendAttachment;
		configData.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configData.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configData.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configData.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		configData.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configData.depthStencilInfo.depthTestEnable = VK_TRUE;
		configData.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configData.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configData.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configData.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configData.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configData.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configData.depthStencilInfo.front = {};  // Optional
		configData.depthStencilInfo.back = {};   // Optional

		configData.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		configData.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configData.dynamicStateInfo.pDynamicStates = configData.dynamicStateEnables.data();
		configData.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configData.dynamicStateEnables.size());
		configData.dynamicStateInfo.flags = 0;

	}
}