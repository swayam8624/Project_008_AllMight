// Native Vulkan 1.3 resource-visibility + timeline-semaphore witness.
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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        ++validation_errors;
        std::cerr << "validation error: "
                  << (data && data->pMessage ? data->pMessage : "(none)") << '\n';
    }
    return VK_FALSE;
}

void check(VkResult r,const char* what){
    if(r!=VK_SUCCESS) throw std::runtime_error(std::string(what)+" failed: "+std::to_string(r));
}

bool has_layer(const char* name){
    std::uint32_t n=0; check(vkEnumerateInstanceLayerProperties(&n,nullptr),"enumerate layers");
    std::vector<VkLayerProperties> p(n); check(vkEnumerateInstanceLayerProperties(&n,p.data()),"enumerate layers");
    for(const auto& x:p) if(std::strcmp(x.layerName,name)==0) return true;
    return false;
}

bool has_extension(const char* name){
    std::uint32_t n=0; check(vkEnumerateInstanceExtensionProperties(nullptr,&n,nullptr),"enumerate extensions");
    std::vector<VkExtensionProperties> p(n); check(vkEnumerateInstanceExtensionProperties(nullptr,&n,p.data()),"enumerate extensions");
    for(const auto& x:p) if(std::strcmp(x.extensionName,name)==0) return true;
    return false;
}

std::uint32_t choose_memory_type(
    VkPhysicalDevice physical,
    std::uint32_t bits,
    VkMemoryPropertyFlags required) {
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(physical,&mp);
    for(std::uint32_t i=0;i<mp.memoryTypeCount;++i){
        if((bits&(1u<<i))!=0 &&
           (mp.memoryTypes[i].propertyFlags&required)==required) return i;
    }
    throw std::runtime_error("no compatible host-visible coherent memory type");
}
} // namespace

int main(){
    VkInstance instance=VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger=VK_NULL_HANDLE;
    VkDevice device=VK_NULL_HANDLE;
    VkBuffer buffer=VK_NULL_HANDLE;
    VkDeviceMemory memory=VK_NULL_HANDLE;
    VkCommandPool pool=VK_NULL_HANDLE;
    VkSemaphore timeline=VK_NULL_HANDLE;

    try{
        const bool validation=has_layer("VK_LAYER_KHRONOS_validation");
        const bool debug=has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkApplicationInfo app{};
        app.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName="PROJECTALLMIGHT buffer timeline witness";
        app.apiVersion=VK_API_VERSION_1_3;

        std::vector<const char*> layers;
        if(validation) layers.push_back("VK_LAYER_KHRONOS_validation");
        std::vector<const char*> exts;
        if(debug) exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkInstanceCreateInfo ici{};
        ici.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ici.pApplicationInfo=&app;
        ici.enabledLayerCount=static_cast<std::uint32_t>(layers.size());
        ici.ppEnabledLayerNames=layers.empty()?nullptr:layers.data();
        ici.enabledExtensionCount=static_cast<std::uint32_t>(exts.size());
        ici.ppEnabledExtensionNames=exts.empty()?nullptr:exts.data();
        check(vkCreateInstance(&ici,nullptr,&instance),"vkCreateInstance");

        auto create_debug=reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT"));
        auto destroy_debug=reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT"));
        if(debug&&create_debug){
            VkDebugUtilsMessengerCreateInfoEXT dci{};
            dci.sType=VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            dci.messageSeverity=VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            dci.messageType=VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            dci.pfnUserCallback=debug_callback;
            check(create_debug(instance,&dci,nullptr,&messenger),"create debug messenger");
        }

        std::uint32_t pc=0; check(vkEnumeratePhysicalDevices(instance,&pc,nullptr),"enumerate physical devices");
        require:
        if(pc==0) throw std::runtime_error("no Vulkan physical device");
        std::vector<VkPhysicalDevice> physicals(pc);
        check(vkEnumeratePhysicalDevices(instance,&pc,physicals.data()),"enumerate physical devices");

        VkPhysicalDevice physical=VK_NULL_HANDLE;
        std::uint32_t qfamily=0;
        for(VkPhysicalDevice candidate:physicals){
            VkPhysicalDeviceTimelineSemaphoreFeatures timeline_feature{};
            timeline_feature.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;

            VkPhysicalDeviceSynchronization2Features sync2{};
            sync2.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
            sync2.pNext=&timeline_feature;

            VkPhysicalDeviceFeatures2 f2{};
            f2.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext=&sync2;
            vkGetPhysicalDeviceFeatures2(candidate,&f2);
            if(!sync2.synchronization2||!timeline_feature.timelineSemaphore) continue;

            std::uint32_t qc=0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate,&qc,nullptr);
            std::vector<VkQueueFamilyProperties> qp(qc);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate,&qc,qp.data());
            for(std::uint32_t q=0;q<qc;++q){
                if(qp[q].queueCount>0 && (qp[q].queueFlags&VK_QUEUE_TRANSFER_BIT)!=0){
                    physical=candidate; qfamily=q; break;
                }
            }
            if(physical!=VK_NULL_HANDLE) break;
        }
        if(physical==VK_NULL_HANDLE) throw std::runtime_error("no sync2+timeline device with transfer queue");

        const float priority=1.0F;
        VkDeviceQueueCreateInfo qci{};
        qci.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex=qfamily;
        qci.queueCount=1;
        qci.pQueuePriorities=&priority;

        VkPhysicalDeviceTimelineSemaphoreFeatures timeline_enable{};
        timeline_enable.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timeline_enable.timelineSemaphore=VK_TRUE;

        VkPhysicalDeviceSynchronization2Features sync2_enable{};
        sync2_enable.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2_enable.pNext=&timeline_enable;
        sync2_enable.synchronization2=VK_TRUE;

        VkDeviceCreateInfo devci{};
        devci.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        devci.pNext=&sync2_enable;
        devci.queueCreateInfoCount=1;
        devci.pQueueCreateInfos=&qci;
        check(vkCreateDevice(physical,&devci,nullptr,&device),"vkCreateDevice");

        VkQueue queue=VK_NULL_HANDLE;
        vkGetDeviceQueue(device,qfamily,0,&queue);

        constexpr VkDeviceSize bytes=64;
        VkBufferCreateInfo bci{};
        bci.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bci.size=bytes;
        bci.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bci.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        check(vkCreateBuffer(device,&bci,nullptr,&buffer),"vkCreateBuffer");

        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(device,buffer,&req);
        VkMemoryAllocateInfo mai{};
        mai.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mai.allocationSize=req.size;
        mai.memoryTypeIndex=choose_memory_type(
            physical,req.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        check(vkAllocateMemory(device,&mai,nullptr,&memory),"vkAllocateMemory");
        check(vkBindBufferMemory(device,buffer,memory,0),"vkBindBufferMemory");

        VkCommandPoolCreateInfo pci{};
        pci.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.queueFamilyIndex=qfamily;
        check(vkCreateCommandPool(device,&pci,nullptr,&pool),"vkCreateCommandPool");

        VkCommandBufferAllocateInfo cai{};
        cai.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cai.commandPool=pool;
        cai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cai.commandBufferCount=1;
        VkCommandBuffer cmd=VK_NULL_HANDLE;
        check(vkAllocateCommandBuffers(device,&cai,&cmd),"vkAllocateCommandBuffers");

        VkCommandBufferBeginInfo cbi{};
        cbi.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(cmd,&cbi),"vkBeginCommandBuffer");

        constexpr std::uint32_t pattern=0x01020304u;
        vkCmdFillBuffer(cmd,buffer,0,bytes,pattern);

        VkBufferMemoryBarrier2 barrier{};
        barrier.sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.srcStageMask=VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask=VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask=VK_PIPELINE_STAGE_2_HOST_BIT;
        barrier.dstAccessMask=VK_ACCESS_2_HOST_READ_BIT;
        barrier.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer=buffer;
        barrier.offset=0;
        barrier.size=VK_WHOLE_SIZE;

        VkDependencyInfo dep{};
        dep.sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.bufferMemoryBarrierCount=1;
        dep.pBufferMemoryBarriers=&barrier;
        vkCmdPipelineBarrier2(cmd,&dep);
        check(vkEndCommandBuffer(cmd),"vkEndCommandBuffer");

        VkSemaphoreTypeCreateInfo stci{};
        stci.sType=VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        stci.semaphoreType=VK_SEMAPHORE_TYPE_TIMELINE;
        stci.initialValue=0;
        VkSemaphoreCreateInfo sci{};
        sci.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        sci.pNext=&stci;
        check(vkCreateSemaphore(device,&sci,nullptr,&timeline),"vkCreateSemaphore");

        VkCommandBufferSubmitInfo cbsi{};
        cbsi.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cbsi.commandBuffer=cmd;

        VkSemaphoreSubmitInfo signal{};
        signal.sType=VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signal.semaphore=timeline;
        signal.value=7;
        signal.stageMask=VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submit{};
        submit.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount=1;
        submit.pCommandBufferInfos=&cbsi;
        submit.signalSemaphoreInfoCount=1;
        submit.pSignalSemaphoreInfos=&signal;
        check(vkQueueSubmit2(queue,1,&submit,VK_NULL_HANDLE),"vkQueueSubmit2");

        const std::uint64_t wait_value=7;
        VkSemaphoreWaitInfo wi{};
        wi.sType=VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wi.semaphoreCount=1;
        wi.pSemaphores=&timeline;
        wi.pValues=&wait_value;
        check(vkWaitSemaphores(device,&wi,10'000'000'000ULL),"vkWaitSemaphores");

        std::uint64_t observed=0;
        check(vkGetSemaphoreCounterValue(device,timeline,&observed),"vkGetSemaphoreCounterValue");
        if(observed<7) throw std::runtime_error("timeline did not reach signaled value");

        void* mapped=nullptr;
        check(vkMapMemory(device,memory,0,bytes,0,&mapped),"vkMapMemory");
        const auto* words=static_cast<const std::uint32_t*>(mapped);
        for(std::size_t i=0;i<bytes/sizeof(std::uint32_t);++i){
            if(words[i]!=pattern) throw std::runtime_error("host did not observe transfer fill pattern");
        }
        vkUnmapMemory(device,memory);

        vkDeviceWaitIdle(device);
        vkDestroySemaphore(device,timeline,nullptr); timeline=VK_NULL_HANDLE;
        vkDestroyCommandPool(device,pool,nullptr); pool=VK_NULL_HANDLE;
        vkDestroyBuffer(device,buffer,nullptr); buffer=VK_NULL_HANDLE;
        vkFreeMemory(device,memory,nullptr); memory=VK_NULL_HANDLE;
        vkDestroyDevice(device,nullptr); device=VK_NULL_HANDLE;
        if(messenger!=VK_NULL_HANDLE&&destroy_debug) destroy_debug(instance,messenger,nullptr);
        messenger=VK_NULL_HANDLE;
        vkDestroyInstance(instance,nullptr); instance=VK_NULL_HANDLE;

        if(validation_errors.load()!=0) throw std::runtime_error("validation layer reported errors");

        std::cout
            <<"vulkan_buffer_timeline_smoke: PASS\n"
            <<"proved: real buffer allocation/binding, transfer write, resource-specific sync2 buffer barrier to host read, timeline semaphore signal/wait/counter, mapped-result visibility"
            <<(validation?", validation enabled":", validation unavailable")
            <<"\nnot proved: image layouts, queue-family ownership transfer, device-local staging copies, descriptor/shader visibility, transient aliasing, or production renderer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"vulkan_buffer_timeline_smoke: FAIL: "<<e.what()<<'\n';
        if(device!=VK_NULL_HANDLE) vkDeviceWaitIdle(device);
        if(timeline!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroySemaphore(device,timeline,nullptr);
        if(pool!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroyCommandPool(device,pool,nullptr);
        if(buffer!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroyBuffer(device,buffer,nullptr);
        if(memory!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkFreeMemory(device,memory,nullptr);
        if(device!=VK_NULL_HANDLE) vkDestroyDevice(device,nullptr);
        if(instance!=VK_NULL_HANDLE) vkDestroyInstance(instance,nullptr);
        return 1;
    }
}
