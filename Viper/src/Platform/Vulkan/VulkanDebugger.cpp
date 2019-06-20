#include "vpch.h"
#include "VulkanDebugger.h"

namespace Viper
{

	/******************** Validation layers && Message callback ********************/

	VulkanDebugger::VulkanDebugger(bool enableValidationLayers) 
		: enableValidationLayers(enableValidationLayers)
	{
	}

	bool VulkanDebugger::checkValidationLayerSupport()
	{
		/*
			function checks if all requested validation layers are available
		*/

		// get number of layer properties available
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// get all available layers
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// loop through all validationLayers and check if all exist in the availableLayers
		for (const char* layerName : this->validationLayers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (layerFound == false) return false;
		}

		return true;
	}

	std::vector<const char *> VulkanDebugger::getRequiredExtensions()
	{
		/*
			get the required list of extensions based on whether validation layers are enabled or not
		*/

		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		// This function returns an array of names of Vulkan instance extensions required by GLFW for creating Vulkan surfaces for GLFW windows
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (this->enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanDebugger::setupDebugMessenger(VkInstance &instance)
	{
		if (!this->enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = this->debugCallback;
		createInfo.pUserData = nullptr; // Optional

		// create debug messenger
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS)
			V_CORE_ASSERT(false, "failed to set up debug messenger!");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																 VkDebugUtilsMessageTypeFlagsEXT messageType,
																 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
																 void *pUserData)
	{
		/*
			debug callback function
		*/

		V_CORE_TRACE("Validation layer - {0}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkResult VulkanDebugger::CreateDebugUtilsMessengerEXT(VkInstance instance,
														  const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
														  const VkAllocationCallbacks *pAllocator,
														  VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		/*
			create debug messenger
		*/

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebugger::DestroyDebugUtilsMessengerEXT(VkInstance instance,
													   VkDebugUtilsMessengerEXT debugMessenger,
													   const VkAllocationCallbacks * pAllocator)
	{
		/*
			destroy debug messenger
		*/

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

}