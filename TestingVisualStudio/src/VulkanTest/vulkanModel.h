#pragma once

#include "vulkanDevice.h"
#include "vulkanBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace lve {

	struct VulkanModel {
		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator == (const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indicies{};

			void LoadModel(const std::string &filePath);
		};

		VulkanModel(VulkanDevice& vulkanDevice, const VulkanModel::Builder& builder);
		~VulkanModel();

		VulkanModel(const VulkanModel&) = delete;
		VulkanModel& operator=(const VulkanModel&) = delete;

		static std::unique_ptr<VulkanModel> CreateModelFromDevice(VulkanDevice& device, const std::string &filePath);

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:		
		
		VulkanDevice& vulkanDevice;

		//Vertex
		std::unique_ptr<VulkanBuffer> vertexBuffer;
		uint32_t vertexCount;

		//Index
		std::unique_ptr<VulkanBuffer> indexBuffer;
		uint32_t indexCount;

		bool hasIndexBuffer{false};

		void CreateVertexBuffers(const std::vector<Vertex>& vertices);

		void CreateIndexBuffers(const std::vector<uint32_t>& indicies);
	};
}