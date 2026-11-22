// Project AllMight GPU-14F: OPTIONAL actual Vulkan queue/fence/buffer witness.
//
// Built with real Vulkan only when CMake finds an SDK. Invoked explicitly via
// --probe-native, never by the portable CTest. No surface, swapchain, shader,
// Sync2, timeline, GPU timestamp, MoltenVK graphics window or display output.
//
// Real operations on the opt-in path: instance/device creation, headless
// graphics/compute/transfer-capable queue, HOST_VISIBLE|HOST_COHERENT buffer,
// command pool + primary command buffer, vkCmdFillBuffer, memory barrier to
// HOST, vkQueueSubmit (Vulkan 1.0 baseline), vkWaitForFences, map and verify
// every 32-bit word, then reverse lifetime cleanup. A real VkFence success
// is the ONLY completion proof. No call to vkResetFences before recording.
//
// Return 0 = this real native fill/submit/wait/readback succeeded.
// Return 77 = environment unavailable (SDK/device compatible memory).
// Return 1 = actual API/result check failed. Never count 77 as a GPU pass.
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>
#ifdef ALLMIGHT_COURSE_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace allmight::gpu_course {
int run_native_submission_probe() {
#ifndef ALLMIGHT_COURSE_HAS_VULKAN
    std::cout<<"UNAVAILABLE: Vulkan SDK/loader not found at configure time; "
                "portable GPU-14F CPU state tests are distinct.\n";
    return 77;
#else
    constexpr std::uint32_t fillWord=0xA17F3C29U;
    constexpr VkDeviceSize bufferBytes=256;
    struct Owned {
        VkInstance instance{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        VkCommandPool pool{VK_NULL_HANDLE};
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        VkFence fence{VK_NULL_HANDLE};
        bool work_accepted{};
        bool gpu_done{};
        ~Owned() {
            if(device) {
                // On any error *after* queue acceptance, do not free command
                // storage while the GPU may still access it. A verified
                // device idle is needed before the ordinary destroy path.
                // Under device loss, native cleanup rules apply separately.
                if(work_accepted && !gpu_done) {
                    const auto idle=vkDeviceWaitIdle(device);
                    if(idle!=VK_SUCCESS)
                        std::cerr<<"Native Vulkan device idle did not complete "
                                 <<"normally during failure cleanup: "<<idle<<"\n";
                }
                if(fence) vkDestroyFence(device,fence,nullptr);
                if(pool) vkDestroyCommandPool(device,pool,nullptr);
                if(buffer) vkDestroyBuffer(device,buffer,nullptr);
                if(memory) vkFreeMemory(device,memory,nullptr);
                vkDestroyDevice(device,nullptr);
            }
            if(instance) vkDestroyInstance(instance,nullptr);
        }
    } owned;

    std::uint32_t extensionsCount=0;
    auto result=vkEnumerateInstanceExtensionProperties(
        nullptr,&extensionsCount,nullptr);
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: enumerate Vulkan instance extensions "<<result<<"\n";
        return 1;
    }
    std::vector<VkExtensionProperties> instanceExtensions(extensionsCount);
    if(extensionsCount) {
        result=vkEnumerateInstanceExtensionProperties(
            nullptr,&extensionsCount,instanceExtensions.data());
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: enumerate extension list "<<result<<"\n";
            return 1;
        }
    }
    std::vector<const char*> instanceRequired;
    VkInstanceCreateFlags flags{};
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    for(const auto& ext:instanceExtensions) {
        if(std::strcmp(ext.extensionName,
              VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)==0) {
            instanceRequired.push_back(
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags|=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            break;
        }
    }
#endif
    VkApplicationInfo app{};
    app.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.apiVersion=VK_API_VERSION_1_0;
    app.pApplicationName="AllMight real Vulkan queue/fence test";
    VkInstanceCreateInfo info{};
    info.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo=&app;
    info.flags=flags;
    info.enabledExtensionCount=
        static_cast<std::uint32_t>(instanceRequired.size());
    info.ppEnabledExtensionNames=instanceRequired.data();
    result=vkCreateInstance(&info,nullptr,&owned.instance);
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: vkCreateInstance "<<result<<"\n";
        return 1;
    }
    std::uint32_t count=0;
    result=vkEnumeratePhysicalDevices(owned.instance,&count,nullptr);
    if(result!=VK_SUCCESS || count==0) {
        std::cout<<"UNAVAILABLE: no Vulkan physical device ("<<result<<").\n";
        return result==VK_SUCCESS?77:1;
    }
    std::vector<VkPhysicalDevice> physicals(count);
    result=vkEnumeratePhysicalDevices(owned.instance,&count,physicals.data());
    if(result!=VK_SUCCESS) {
        std::cerr<<"ERROR: physical-device list "<<result<<"\n";
        return 1;
    }
    physicals.resize(count);
    for(const auto physical:physicals) {
        std::uint32_t queueCount=0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            physical,&queueCount,nullptr);
        std::vector<VkQueueFamilyProperties> families(queueCount);
        if(queueCount)
            vkGetPhysicalDeviceQueueFamilyProperties(
                physical,&queueCount,families.data());
        std::uint32_t chosen=queueCount;
        for(std::uint32_t i=0;i<queueCount;++i)
            if(families[i].queueCount &&
               (families[i].queueFlags &
                 (VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT|
                  VK_QUEUE_TRANSFER_BIT))) {
                chosen=i;break;
            }
        if(chosen==queueCount) continue;

        // Instance can expose portability devices; the device may require
        // enabling VK_KHR_portability_subset if it advertises that extension.
        std::uint32_t extCount=0;
        result=vkEnumerateDeviceExtensionProperties(
            physical,nullptr,&extCount,nullptr);
        if(result!=VK_SUCCESS) continue;
        std::vector<VkExtensionProperties> available(extCount);
        if(extCount) {
            result=vkEnumerateDeviceExtensionProperties(
                physical,nullptr,&extCount,available.data());
            if(result!=VK_SUCCESS) continue;
        }
        std::vector<const char*> enabled;
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        for(const auto& ext:available)
            if(std::strcmp(ext.extensionName,
                    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)==0) {
                enabled.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
                break;
            }
#endif
        const float priority=1.0f;
        VkDeviceQueueCreateInfo qi{};
        qi.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex=chosen;
        qi.queueCount=1;
        qi.pQueuePriorities=&priority;
        VkDeviceCreateInfo di{};
        di.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        di.queueCreateInfoCount=1;
        di.pQueueCreateInfos=&qi;
        di.enabledExtensionCount=static_cast<std::uint32_t>(enabled.size());
        di.ppEnabledExtensionNames=enabled.data();
        result=vkCreateDevice(physical,&di,nullptr,&owned.device);
        if(result!=VK_SUCCESS) continue;
        VkQueue queue=VK_NULL_HANDLE;
        vkGetDeviceQueue(owned.device,chosen,0,&queue);
        if(!queue) {
            std::cerr<<"ERROR: selected native queue missing.\n";
            return 1;
        }

        VkBufferCreateInfo bc{};
        bc.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bc.size=bufferBytes;
        bc.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bc.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        result=vkCreateBuffer(owned.device,&bc,nullptr,&owned.buffer);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkCreateBuffer "<<result<<"\n";
            return 1;
        }
        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(owned.device,owned.buffer,&req);
        VkPhysicalDeviceMemoryProperties types{};
        vkGetPhysicalDeviceMemoryProperties(physical,&types);
        std::uint32_t type=types.memoryTypeCount;
        const auto required=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for(std::uint32_t i=0;i<types.memoryTypeCount;++i) {
            if((req.memoryTypeBits&(1U<<i)) &&
               (types.memoryTypes[i].propertyFlags&required)==required) {
                type=i;break;
            }
        }
        if(type==types.memoryTypeCount) {
            std::cout<<"UNAVAILABLE: device has no compatible HOST_VISIBLE "
                        "and HOST_COHERENT memory type for transfer target.\n";
            return 77;
        }
        VkMemoryAllocateInfo ma{};
        ma.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ma.allocationSize=req.size;
        ma.memoryTypeIndex=type;
        result=vkAllocateMemory(owned.device,&ma,nullptr,&owned.memory);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkAllocateMemory "<<result<<"\n";
            return 1;
        }
        result=vkBindBufferMemory(owned.device,owned.buffer,owned.memory,0);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkBindBufferMemory "<<result<<"\n";
            return 1;
        }
        VkCommandPoolCreateInfo pc{};
        pc.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pc.queueFamilyIndex=chosen;
        result=vkCreateCommandPool(owned.device,&pc,nullptr,&owned.pool);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkCreateCommandPool "<<result<<"\n";
            return 1;
        }
        VkCommandBufferAllocateInfo ca{};
        ca.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ca.commandPool=owned.pool;
        ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ca.commandBufferCount=1;
        VkCommandBuffer cb=VK_NULL_HANDLE;
        result=vkAllocateCommandBuffers(owned.device,&ca,&cb);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkAllocateCommandBuffers "<<result<<"\n";
            return 1;
        }
        VkCommandBufferBeginInfo bi{};
        bi.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        result=vkBeginCommandBuffer(cb,&bi);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkBeginCommandBuffer "<<result<<"\n";
            return 1;
        }
        vkCmdFillBuffer(cb,owned.buffer,0,bufferBytes,fillWord);
        VkMemoryBarrier barrier{};
        barrier.sType=VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask=VK_ACCESS_HOST_READ_BIT;
        vkCmdPipelineBarrier(cb,VK_PIPELINE_STAGE_TRANSFER_BIT,
             VK_PIPELINE_STAGE_HOST_BIT,0,
             1,&barrier,0,nullptr,0,nullptr);
        result=vkEndCommandBuffer(cb);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkEndCommandBuffer "<<result<<"\n";
            return 1;
        }
        VkFenceCreateInfo fc{};
        fc.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // Unsignaled fence: it will be signaled only when native submission
        // finishes, not by an invented CPU state transition.
        result=vkCreateFence(owned.device,&fc,nullptr,&owned.fence);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkCreateFence "<<result<<"\n";
            return 1;
        }
        VkSubmitInfo si{};
        si.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount=1;
        si.pCommandBuffers=&cb;
        result=vkQueueSubmit(queue,1,&si,owned.fence);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: vkQueueSubmit "<<result<<"\n";
            return 1;
        }
        owned.work_accepted=true;
        result=vkWaitForFences(owned.device,1,&owned.fence,
                               VK_TRUE,UINT64_MAX);
        if(result!=VK_SUCCESS) {
            std::cerr<<"ERROR: native vkWaitForFences "<<result<<"\n";
            return 1;
        }
        owned.gpu_done=true;
        void* mapped=nullptr;
        result=vkMapMemory(owned.device,owned.memory,0,bufferBytes,
                           0,&mapped);
        if(result!=VK_SUCCESS || !mapped) {
            std::cerr<<"ERROR: vkMapMemory after native fence "<<result<<"\n";
            return 1;
        }
        // CPU memcpy avoids requiring C++ object lifetime to be established
        // in GPU-written mapped storage before interpreting 32-bit words.
        bool correct=true;
        for(std::size_t i=0;i<bufferBytes/sizeof(std::uint32_t);++i) {
            std::uint32_t word{};
            std::memcpy(&word,static_cast<const unsigned char*>(mapped)+
                             i*sizeof(word),sizeof(word));
            if(word!=fillWord) { correct=false;break; }
        }
        vkUnmapMemory(owned.device,owned.memory);
        if(!correct) {
            std::cerr<<"ERROR: GPU-written fill pattern differs from "
                        "independent CPU expected words.\n";
            return 1;
        }
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physical,&props);
        std::cout<<"NATIVE PASS: "<<props.deviceName
                 <<" actual vkQueueSubmit + fence completion + checked "
                   "vkCmdFillBuffer readback ("<<bufferBytes
                 <<" bytes), type "<<type<<", allocation "<<req.size
                 <<". No surface, presentation, shader or timing claims.\n";
        return 0;
    }
    std::cout<<"UNAVAILABLE: no suitable native queue/device.\n";
    return 77;
#endif
}
} // namespace allmight::gpu_course
