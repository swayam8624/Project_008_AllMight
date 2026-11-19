// Project AllMight / Pass13 optional NATIVE Vulkan capability + buffer probe.
// This is the only compilation unit in the lesson that may include Vulkan.
// CMake only defines ALLMIGHT_COURSE_HAS_VULKAN and links Vulkan::Vulkan when
// a compatible Vulkan SDK/loader is discoverable. No SDK => clean C++23
// core-only build; explicitly requested probe returns code 77 (unavailable).
//
// --probe-native tests real vkCreateInstance, physical-device/queue/memory
// enumeration, headless vkCreateDevice, VkBuffer requirements, memory choice,
// vkAllocateMemory and vkBindBufferMemory; then destroys bound resources in
// reverse order. No window/surface, presentation, command submission,
// shader execution, Metal device, or GPU performance measurement is claimed.
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string_view>

#ifdef ALLMIGHT_COURSE_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace allmight::gpu_course {
int run_native_probe() {
#ifndef ALLMIGHT_COURSE_HAS_VULKAN
    std::cout <<
        "UNAVAILABLE: Vulkan headers/loader were absent at CMake configure time. "
        "No native device was queried; portable CPU contract tests still apply.\n";
    return 77;
#else
    std::uint32_t extCount=0;
    VkResult result=vkEnumerateInstanceExtensionProperties(
        nullptr,&extCount,nullptr);
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: vkEnumerateInstanceExtensionProperties: "<<result<<"\n";
        return 1;
    }
    std::vector<VkExtensionProperties> available(extCount);
    if(extCount!=0) {
        result=vkEnumerateInstanceExtensionProperties(
            nullptr,&extCount,available.data());
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: instance extension discovery: "<<result<<"\n";
            return 1;
        }
    }
    std::vector<const char*> requestedExtensions;
    VkInstanceCreateFlags instanceFlags=0;
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    for(const auto& ext:available) {
        if(std::strcmp(ext.extensionName,
                       VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)==0) {
            requestedExtensions.push_back(
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#ifdef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
            instanceFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
            break;
        }
    }
#endif
    VkApplicationInfo application{};
    application.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application.pApplicationName="AllMight cross-project capability probe";
    application.applicationVersion=VK_MAKE_VERSION(1,0,0);
    application.pEngineName="AllMight teaching course";
    application.engineVersion=VK_MAKE_VERSION(1,0,0);
    // Vulkan 1.0 is enough for the methods used in this bounded probe.
    application.apiVersion=VK_API_VERSION_1_0;
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo=&application;
    instanceInfo.flags=instanceFlags;
    instanceInfo.enabledExtensionCount=
        static_cast<std::uint32_t>(requestedExtensions.size());
    instanceInfo.ppEnabledExtensionNames=requestedExtensions.data();
    VkInstance instance=VK_NULL_HANDLE;
    result=vkCreateInstance(&instanceInfo,nullptr,&instance);
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: vkCreateInstance failed: "<<result<<"\n";
        return 1;
    }
    struct InstanceGuard {
        VkInstance value;
        ~InstanceGuard(){if(value) vkDestroyInstance(value,nullptr);}
    } instanceGuard{instance};

    std::uint32_t physicalCount=0;
    result=vkEnumeratePhysicalDevices(instance,&physicalCount,nullptr);
    if(result!=VK_SUCCESS || physicalCount==0) {
        std::cout<<"UNAVAILABLE: native Vulkan instance has no enumerable "
                    "physical device (result="<<result<<").\n";
        return result==VK_SUCCESS?77:1;
    }
    std::vector<VkPhysicalDevice> candidates(physicalCount);
    result=vkEnumeratePhysicalDevices(instance,&physicalCount,candidates.data());
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: vkEnumeratePhysicalDevices list: "<<result<<"\n";
        return 1;
    }
    for(const VkPhysicalDevice physical:candidates) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physical,&properties);
        std::uint32_t queueCount=0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical,&queueCount,nullptr);
        std::vector<VkQueueFamilyProperties> families(queueCount);
        if(queueCount!=0)
            vkGetPhysicalDeviceQueueFamilyProperties(
                physical,&queueCount,families.data());

        std::uint32_t chosen=queueCount;
        for(std::uint32_t i=0;i<queueCount;++i) {
            if(families[i].queueCount!=0 &&
               (families[i].queueFlags &
                    (VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT))!=0) {
                chosen=i;
                break;
            }
        }
        std::cout<<"Physical device: "<<properties.deviceName
                 <<", Vulkan "<<VK_VERSION_MAJOR(properties.apiVersion)
                 <<"."<<VK_VERSION_MINOR(properties.apiVersion)
                 <<", queue families "<<queueCount
                 <<". Surface presentation: NOT QUERIED.\n";
        if(chosen==queueCount) continue;

        // Portability subset is a *device* extension and may be mandatory
        // on portability implementations. Never require VK_KHR_swapchain:
        // without a VkSurfaceKHR there is no presentation requirement.
        std::vector<const char*> deviceExtensions;
        std::uint32_t deviceExtCount=0;
        result=vkEnumerateDeviceExtensionProperties(
            physical,nullptr,&deviceExtCount,nullptr);
        if(result!=VK_SUCCESS) continue;
        std::vector<VkExtensionProperties> deviceExts(deviceExtCount);
        if(deviceExtCount!=0) {
            result=vkEnumerateDeviceExtensionProperties(
                physical,nullptr,&deviceExtCount,deviceExts.data());
            if(result!=VK_SUCCESS) continue;
        }
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        for(const auto& ext:deviceExts) {
            if(std::strcmp(ext.extensionName,
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)==0) {
                deviceExtensions.push_back(
                    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
                break;
            }
        }
#endif
        const float priority=1.0F;
        VkDeviceQueueCreateInfo qi{};
        qi.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex=chosen;
        qi.queueCount=1;
        qi.pQueuePriorities=&priority;
        VkDeviceCreateInfo di{};
        di.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        di.queueCreateInfoCount=1;
        di.pQueueCreateInfos=&qi;
        di.enabledExtensionCount=
            static_cast<std::uint32_t>(deviceExtensions.size());
        di.ppEnabledExtensionNames=deviceExtensions.data();
        VkDevice device=VK_NULL_HANDLE;
        result=vkCreateDevice(physical,&di,nullptr,&device);
        if(result!=VK_SUCCESS) {
            std::cout<<"Device creation refused on this adapter: "<<result<<"\n";
            continue;
        }
        struct DeviceGuard {
            VkDevice value;
            ~DeviceGuard(){if(value) vkDestroyDevice(value,nullptr);}
        } deviceGuard{device};
        VkQueue queue=VK_NULL_HANDLE;
        vkGetDeviceQueue(device,chosen,0,&queue);
        if(!queue) {
            std::cerr<<"ERROR: created queue could not be fetched.\n";
            return 1;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size=256;
        bufferInfo.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        // One joint owner prevents destructor-order mistakes: the resource
        // is destroyed BEFORE its still-bound backing memory is freed.
        struct BoundBufferGuard {
            VkDevice device;
            VkBuffer buffer{VK_NULL_HANDLE};
            VkDeviceMemory storage{VK_NULL_HANDLE};
            ~BoundBufferGuard() {
                if(buffer) vkDestroyBuffer(device,buffer,nullptr);
                if(storage) vkFreeMemory(device,storage,nullptr);
            }
        } owned{device};
        result=vkCreateBuffer(device,&bufferInfo,nullptr,&owned.buffer);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: native vkCreateBuffer: "<<result<<"\n";
            return 1;
        }

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device,owned.buffer,&requirements);
        VkPhysicalDeviceMemoryProperties memory{};
        vkGetPhysicalDeviceMemoryProperties(physical,&memory);

        std::uint32_t selected=memory.memoryTypeCount;
        // Prefer a host-accessible coherent type for this small upload,
        // otherwise a compatible device-local type (no host map attempt).
        for(std::uint32_t i=0;i<memory.memoryTypeCount;++i) {
            if((requirements.memoryTypeBits&(1U<<i))!=0 &&
               (memory.memoryTypes[i].propertyFlags &
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                selected=i;
                break;
            }
        }
        if(selected==memory.memoryTypeCount) {
            for(std::uint32_t i=0;i<memory.memoryTypeCount;++i) {
                if((requirements.memoryTypeBits&(1U<<i))!=0) {
                    selected=i;
                    break;
                }
            }
        }
        if(selected==memory.memoryTypeCount) {
            std::cerr<<"ERROR: no memory type for actual buffer requirements.\n";
            return 1;
        }
        VkMemoryAllocateInfo alloc{};
        alloc.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize=requirements.size; // not the 256 source bytes
        alloc.memoryTypeIndex=selected;
        result=vkAllocateMemory(device,&alloc,nullptr,&owned.storage);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: native vkAllocateMemory: "<<result<<"\n";
            return 1;
        }
        result=vkBindBufferMemory(device,owned.buffer,owned.storage,0);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: native vkBindBufferMemory: "<<result<<"\n";
            return 1;
        }
        std::cout<<"NATIVE SUCCESS: headless device+queue created; "
                 <<"256-byte buffer created and memory bound. "
                 <<"Actual allocation bytes "<<requirements.size
                 <<", alignment "<<requirements.alignment
                 <<", selected type "<<selected
                 <<". No commands submitted, no presentation inferred.\n";
        // Local destructors release VkBuffer BEFORE bound memory, then
        // logical device, and finally Vulkan instance.
        return 0;
    }
    std::cout<<"UNAVAILABLE: no enumerated adapter accepted a headless "
                "graphics/compute logical-device request.\n";
    return 77;
#endif
}
} // namespace allmight::gpu_course
