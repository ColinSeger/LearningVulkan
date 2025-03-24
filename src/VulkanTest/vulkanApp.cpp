#include <stdexcept>
#include <array>
#include <cassert>
//remove later
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Render/Buffer/vulkanBuffer.h"
#include "Camera&Movement/vulkanCamera.h"
#include "Render/RenderSystems/simpleVulkanRenderSystem.h"
#include "vulkanApp.h"
#include "../timeCheck.h"
#include "Camera&Movement/KeyboardMovementCTRL.h"


namespace lve {

	struct GlobalUbo {
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};

		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		glm::vec3 lightPosition {-1.f};
		alignas(16) glm::vec4 lightColor {1.f};//w is light intensity
	};

	vulkanApp::vulkanApp() {
		globalPool = 
			LveDescriptorPool::Builder(engineDevice)
			.setMaxSets(vulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		LoadGameObjects();
	}	

	vulkanApp::~vulkanApp() {}

	void vulkanApp::Run() {

		std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(vulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<VulkanBuffer>(
				engineDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->Map();
		}

		auto globalSetLayout = VulkanDescriptorSetLayout::Builder(engineDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDesrciptorSets(vulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < globalDesrciptorSets.size(); i++) {
			auto bufferData = uboBuffers[i]->DescriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferData)
				.build(globalDesrciptorSets[i]
				);
		}

		SimpleVulkanRenderSystem simpleRendererSystem
		{
			engineDevice, 
			vulkanRenderer.GetSwapChainRenderPass(), 
			globalSetLayout->getDescriptorSetLayout()
		};

        VulkanCamera camera{};

        //camera.SetViewDirection(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
        //camera.SetViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

        auto viewerObject = GameObject::CreateGameObject();
		viewerObject.transform.translation.z = -2.5f;

        KeyboardMovementCTRL cameraController{};

		std::cout << "maxPushConstantsSize" << engineDevice.properties.limits.maxPushConstantsSize << "\n";

        auto currentTime = std::chrono::high_resolution_clock::now();
		
		while (!lveWindow.ShouldClose()) {
            //Timer timePerFrame;

			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

            currentTime = newTime;

            cameraController.MoveInPlaneXZ(lveWindow.GetGLFWWindow(), frameTime, viewerObject);
            camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            
            float aspect = vulkanRenderer.GetAspectRatio();
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (auto commandBuffer = vulkanRenderer.BeginFrame()) {

				int frameIndex = vulkanRenderer.GetFrameIndex();
				FrameData frameData
				{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDesrciptorSets[frameIndex],
					gameObjects
				};


				//Update
				GlobalUbo ubo{};
				ubo.projection = camera.GetProjectionMatrix();
				ubo.view = camera.GetViewMatrix();
				uboBuffers[frameIndex]->WriteToBuffer(&ubo);
				uboBuffers[frameIndex]->Flush();

				//Render
				vulkanRenderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRendererSystem.RenderGameObjects(frameData);
				vulkanRenderer.EndSwapChainRenderPass(commandBuffer);
				vulkanRenderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(engineDevice.device());
	}

	void vulkanApp::LoadGameObjects() {
		std::shared_ptr<VulkanModel> lveModel =
			VulkanModel::CreateModelFromDevice(engineDevice, "src/Models/flat_vase.obj");
		auto flatVase = GameObject::CreateGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(flatVase.GetId(), std::move(flatVase));

		lveModel = VulkanModel::CreateModelFromDevice(engineDevice, "src/Models/smooth_vase.obj");
		auto smoothVase = GameObject::CreateGameObject();
		smoothVase.model = lveModel;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(smoothVase.GetId(), std::move(smoothVase));


		lveModel = VulkanModel::CreateModelFromDevice(engineDevice, "src/Models/quad.obj");
		auto floor = GameObject::CreateGameObject();
		floor.model = lveModel;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(floor.GetId(), std::move(floor));

		/*std::shared_ptr<VulkanModel> model = VulkanModel::CreateModelFromDevice(engineDevice, "src/Models/flat_vase.obj");

        auto gameObj = GameObject::CreateGameObject();
        gameObj.model = model;
        gameObj.transform.translation = { 0.f,0.5f,2.5f };
        gameObj.transform.scale = { 3.5f,3.5f,3.5f };
        gameObjects.push_back(std::move(gameObj));*/
	}
}