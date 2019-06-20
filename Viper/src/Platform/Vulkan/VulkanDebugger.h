#pragma once

#include <GLFW/glfw3.h>

#include <vector>

namespace Viper
{

	class VulkanDebugger
	{
	private:
		VulkanDebugger(bool enableValidationLayers);

		/******************** Validation layers && Message callback ********************/
		bool checkValidationLayerSupport();
		std::vector<const char *> getRequiredExtensions();

		void setupDebugMessenger(VkInstance &instance);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			VkDebugUtilsMessengerEXT *pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);

	private:
		friend class VulkanContext;

		const std::vector<const char *> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		const std::vector<const char *> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor",
			//"VK_LAYER_LUNARG_api_dump"
		};

		VkDebugUtilsMessengerEXT debugMessenger;
		const bool enableValidationLayers;
	};

}