#include <cassert>
#include <iostream>
#include <unordered_map>


#include "vulkanModel.h"
#include "utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"


namespace std {
	template<>
	struct hash<lve::VulkanModel::Vertex> 
	{
		size_t operator()(lve::VulkanModel::Vertex const& vertex) const 
		{
			size_t seed = 0;
			lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace lve {

	//NOTE TO SELF CHECK VULKAN DEVICE IF ERROR
	VulkanModel::VulkanModel(VulkanDevice& device, const VulkanModel::Builder & builder) : vulkanDevice{device} {
		CreateVertexBuffers(builder.vertices);
		CreateIndexBuffers(builder.indicies);
	}
	VulkanModel::~VulkanModel() {}

	std::unique_ptr<VulkanModel> VulkanModel::CreateModelFromDevice(VulkanDevice& device, const std::string& filePath) {
		Builder builder{};

		builder.LoadModel(filePath);

		std::cout << "Vertex count: " << builder.vertices.size() << "\n";

		return std::make_unique<VulkanModel>(device, builder);
	}

	void VulkanModel::CreateVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());

		assert(vertexCount >= 3 && "vertex count must be atleast 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		uint32_t vertexSize = sizeof(vertices[0]);

		VulkanBuffer stagingBuffer{
			vulkanDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*) vertices.data());

		vertexBuffer = std::make_unique<VulkanBuffer>
			(
				vulkanDevice, 
				vertexSize, 
				vertexCount,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			); 

		vulkanDevice.copyBuffer(stagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);
	}

	void VulkanModel::CreateIndexBuffers(const std::vector<uint32_t>& indecies) {
		indexCount = static_cast<uint32_t>(indecies.size());

		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indecies[0]) * indexCount;
		uint32_t indexSize = sizeof(indecies[0]);

		VulkanBuffer stagingBuffer{
			vulkanDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)indecies.data());

		indexBuffer = std::make_unique<VulkanBuffer>
			(
				vulkanDevice,
				indexCount,
				indexCount,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

		vulkanDevice.copyBuffer(stagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), bufferSize);
	}

	void VulkanModel::Draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	void VulkanModel::Bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {vertexBuffer->GetBuffer()};

		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription> VulkanModel::Vertex::GetBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}
	/*
	* PERHAPS I TRY THIS LATER
	 std::vector<VkVertexInputBindingDescription> VulkanModel::Vertex::GetBindingDescriptions() {
		return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
	}
	*/
	
	std::vector<VkVertexInputAttributeDescription> VulkanModel::Vertex::GetAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });


		return attributeDescriptions;
	}

	void VulkanModel::Builder::LoadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);	
		}

		vertices.clear();
		indicies.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertecies{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = { 
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}
				if (index.normal_index >= 0) {
					vertex.normal = { 
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = { 
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertecies.count(vertex) == 0) {
					uniqueVertecies[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indicies.push_back(uniqueVertecies[vertex]);
			}
		}
	}
}