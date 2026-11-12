// Native Vulkan 1.3 synchronization2 smoke test.
// This file is built only by the dedicated native-GPU workflow, not the generic standalone-lab matrix.
#include <vulkan/vulkan.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::atomic<int> validation_errors{0};

VKAPI_ATTR VkBool32 VKAPI_CALL callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        ++validation_errors;
        std::cerr << "Vulkan validation error: "
                  << (data && data->pMessage ? data->pMessage : "(no message)") << '\n';
    }
    return VK_FALSE;
}

void check(VkResult result, const char* what) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(std::string(what) + " failed with VkResult " + std::to_string(result));
    }
}

bool has_layer(const char* name) {
    std::uint32_t count = 0;
    check(vkEnumerateInstanceLayerProperties(&count, nullptr), "vkEnumerateInstanceLayerProperties");
    std::vector<VkLayerProperties> props(count);
    check(vkEnumerateInstanceLayerProperties(&count, props.data()), "vkEnumerateInstanceLayerProperties");
    for (const auto& p : props) if (std::strcmp(p.layerName, name) == 0) return true;
    return false;
}

bool has_instance_extension(const char* name) {
    std::uint32_t count = 0;
    check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr), "vkEnumerateInstanceExtensionProperties");
    std::vector<VkExtensionProperties> props(count);
    check(vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data()), "vkEnumerateInstanceExtensionProperties");
    for (const auto& p : props) if (std::strcmp(p.extensionName, name) == 0) return true;
    return false;
}
}

int main() {
    try {
        std::uint32_t loader_version = VK_API_VERSION_1_0;
        if (vkEnumerateInstanceVersion) check(vkEnumerateInstanceVersion(&loader_version), "vkEnumerateInstanceVersion");
        if (VK_API_VERSION_MAJOR(loader_version) < 1 ||
            (VK_API_VERSION_MAJOR(loader_version) == 1 && VK_API_VERSION_MINOR(loader_version) < 3)) {
            throw std::runtime_error("Vulkan 1.3 loader required for this core synchronization2 witness");
        }

        const bool validation = has_layer("VK_LAYER_KHRONOS_validation");
        const bool debug_utils = has_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        std::vector<const char*> layers;
        if (validation) layers.push_back("VK_LAYER_KHRONOS_validation");
        std::vector<const char*> extensions;
        if (debug_utils) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = "PROJECTALLMIGHT synchronization2 witness";
        app.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo ici{};
        ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ici.pApplicationInfo = &app;
        ici.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
        ici.ppEnabledLayerNames = layers.data();
        ici.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        ici.ppEnabledExtensionNames = extensions.data();

        VkInstance instance = VK_NULL_HANDLE;
        check(vkCreateInstance(&ici, nullptr, &instance), "vkCreateInstance");

        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
        auto create_debug = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        auto destroy_debug = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (debug_utils && create_debug) {
            VkDebugUtilsMessengerCreateInfoEXT ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            ci.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            ci.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            ci.pfnUserCallback = callback;
            check(create_debug(instance, &ci, nullptr, &messenger), "vkCreateDebugUtilsMessengerEXT");
        }

        std::uint32_t physical_count = 0;
        check(vkEnumeratePhysicalDevices(instance, &physical_count, nullptr), "vkEnumeratePhysicalDevices");
        if (physical_count == 0) throw std::runtime_error("no Vulkan physical device/ICD available");
        std::vector<VkPhysicalDevice> physicals(physical_count);
        check(vkEnumeratePhysicalDevices(instance, &physical_count, physicals.data()), "vkEnumeratePhysicalDevices");

        VkPhysicalDevice physical = VK_NULL_HANDLE;
        std::uint32_t queue_family = 0;
        bool found = false;
        for (VkPhysicalDevice candidate : physicals) {
            VkPhysicalDeviceSynchronization2Features sync2{};
            sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
            VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features.pNext = &sync2;
            vkGetPhysicalDeviceFeatures2(candidate, &features);
            if (!sync2.synchronization2) continue;

            std::uint32_t qcount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &qcount, nullptr);
            std::vector<VkQueueFamilyProperties> qprops(qcount);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &qcount, qprops.data());
            for (std::uint32_t q = 0; q < qcount; ++q) {
                if (qprops[q].queueCount > 0 &&
                    (qprops[q].queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))) {
                    physical = candidate;
                    queue_family = q;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) throw std::runtime_error("no device with synchronization2 and usable queue found");

        const float priority = 1.0F;
        VkDeviceQueueCreateInfo qci{};
        qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex = queue_family;
        qci.queueCount = 1;
        qci.pQueuePriorities = &priority;

        VkPhysicalDeviceSynchronization2Features sync2_enable{};
        sync2_enable.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2_enable.synchronization2 = VK_TRUE;

        VkDeviceCreateInfo dci{};
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.pNext = &sync2_enable;
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &qci;

        VkDevice device = VK_NULL_HANDLE;
        check(vkCreateDevice(physical, &dci, nullptr, &device), "vkCreateDevice");

        VkQueue queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, queue_family, 0, &queue);

        VkCommandPoolCreateInfo pci{};
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.queueFamilyIndex = queue_family;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkCommandPool pool = VK_NULL_HANDLE;
        check(vkCreateCommandPool(device, &pci, nullptr, &pool), "vkCreateCommandPool");

        VkCommandBufferAllocateInfo cai{};
        cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cai.commandPool = pool;
        cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cai.commandBufferCount = 1;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        check(vkAllocateCommandBuffers(device, &cai, &cmd), "vkAllocateCommandBuffers");

        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(cmd, &begin), "vkBeginCommandBuffer");

        VkMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

        VkDependencyInfo dependency{};
        dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency.memoryBarrierCount = 1;
        dependency.pMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(cmd, &dependency);
        check(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");

        VkCommandBufferSubmitInfo cbsi{};
        cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cbsi.commandBuffer = cmd;

        VkSubmitInfo2 submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cbsi;

        VkFenceCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        check(vkCreateFence(device, &fci, nullptr, &fence), "vkCreateFence");
        check(vkQueueSubmit2(queue, 1, &submit, fence), "vkQueueSubmit2");
        check(vkWaitForFences(device, 1, &fence, VK_TRUE, 10'000'000'000ULL), "vkWaitForFences");

        vkDestroyFence(device, fence, nullptr);
        vkDestroyCommandPool(device, pool, nullptr);
        vkDestroyDevice(device, nullptr);
        if (messenger != VK_NULL_HANDLE && destroy_debug) destroy_debug(instance, messenger, nullptr);
        vkDestroyInstance(instance, nullptr);

        if (validation_errors.load() != 0) throw std::runtime_error("validation layer reported errors");

        std::cout
            << "vulkan_sync_smoke: PASS\n"
            << "proved: Vulkan 1.3 instance/device creation, synchronization2 feature enable, "
               "vkCmdPipelineBarrier2 recording, vkQueueSubmit2 execution, fence completion"
            << (validation ? ", validation layer enabled" : ", validation layer unavailable")
            << "\nnot proved: resource-specific visibility, image-layout correctness, queue-family transfer, "
               "timeline semaphore protocols, transient aliasing, hardware performance, or Kairo parity\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "vulkan_sync_smoke: FAIL: " << e.what() << '\n';
        return 1;
    }
}
