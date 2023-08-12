#include <stdexcept>
#include <array>
//remove later
#include <iostream>

#include "vulkanRenderer.h"
#include "../timeCheck.h"

namespace lve {

	VulkanRender::VulkanRender(LveWindow& window, VulkanDevice& device) : lveWindow{ window }, engineDevice{ device } {
		RecreateSwapChain();
		createCommandBuffers();
	}

	VulkanRender::~VulkanRender() {
		FreeCommandBuffers();
	}

	void VulkanRender::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(engineDevice.device(), engineDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void VulkanRender::RecreateSwapChain() {
		auto extent = lveWindow.getExtent();
		while (extent.width == 0 && extent.height == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(engineDevice.device());

		if (vulkanSwap == nullptr) {
			vulkanSwap = std::make_unique<vulkanSwapChain>(engineDevice, extent);
		}
		else {
			std::shared_ptr<vulkanSwapChain> oldSwapChain = std::move(vulkanSwap);
			vulkanSwap = std::make_unique<vulkanSwapChain>(engineDevice, extent, oldSwapChain);

			if (!oldSwapChain->CompareSwapFormats(*vulkanSwap.get())) {
				throw std::runtime_error("Swap chain (image or depth) format has changed");
			}
		}
	}

	void VulkanRender::createCommandBuffers() {

		commandBuffers.resize(vulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = engineDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(engineDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

	VkCommandBuffer VulkanRender::BeginFrame() 
	{
		assert(isFrameStarted && "Cannot call BeginFrame() when frame in progress");

		//Timer time;
		auto result = vulkanSwap->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire swap chain image");
		}

		isFrameStarted = true;

		auto commandBuffer = GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return commandBuffer;
	}
	void VulkanRender::EndFrame() {
		assert(isFrameStarted && "Cannot call EndFrame() when frame is not in progress");

		auto commandBuffer = GetCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = vulkanSwap->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.WasWindowResized()) {
			lveWindow.ResetWindowResizedFlag();
			RecreateSwapChain();
		}else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex++) % vulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void VulkanRender::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Cannot call BeginSwapChainRenderPass() when frame is not in progress");
		assert(commandBuffer && "Cannot begin render pass on commandBuffer when on the wrong frame");


		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vulkanSwap->getRenderPass();
		renderPassInfo.framebuffer = vulkanSwap->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = vulkanSwap->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vulkanSwap->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vulkanSwap->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vulkanSwap->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}
	void VulkanRender::EndSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Cannot call EndSwapChainRenderPass() when frame is not in progress");
		assert(commandBuffer && "Cannot end render pass on commandBuffer when on the wrong frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}