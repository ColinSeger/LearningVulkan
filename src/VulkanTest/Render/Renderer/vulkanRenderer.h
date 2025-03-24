#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "../Window/vulkanWindow.h"
#include "../SwapChain/vulkanSwapChain.h"
#include "../vulkanDevice.h"

namespace lve {
	class VulkanRender {
		bool isFrameStarted{ false };
		uint32_t currentImageIndex;
		int currentFrameIndex;
		LveWindow& lveWindow;
		VulkanDevice& engineDevice;

		std::unique_ptr<vulkanSwapChain> vulkanSwap;

		std::vector<VkCommandBuffer> commandBuffers;

		void createCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

	public:
		VulkanRender(LveWindow& window, VulkanDevice& device);
		~VulkanRender();
		VulkanRender(const VulkanRender&) = delete;
		VulkanRender& operator=(const VulkanRender&) = delete;

		VkCommandBuffer BeginFrame();
		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkRenderPass GetSwapChainRenderPass() const {
			return vulkanSwap->getRenderPass();
		}

		float GetAspectRatio() const {
			return vulkanSwap->extentAspectRatio();
		}

		bool IsFrameInProgress() const { return isFrameStarted; };

		VkCommandBuffer GetCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int GetFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index buffer when frame not in progress");
			return currentFrameIndex;
		}
	};


}