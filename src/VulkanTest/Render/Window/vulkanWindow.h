#pragma once
#include<string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lve {

	class LveWindow {
		std::string windowName;
		int width;
		int height;
		bool frameBufferRezied = false;

		GLFWwindow* window;

		static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height);

		void InitAWindow();

	public:
		LveWindow(int w, int h, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window); }

		VkExtent2D getExtent() { return{ static_cast<uint32_t>(width),static_cast<uint32_t>(height) }; }

		bool WasWindowResized() { return frameBufferRezied; };

		void ResetWindowResizedFlag() { frameBufferRezied = false; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

		GLFWwindow* GetGLFWWindow() const { return window; }
	};
}