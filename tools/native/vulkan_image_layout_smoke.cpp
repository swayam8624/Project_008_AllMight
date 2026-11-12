// PROJECTALLMIGHT Vulkan image-layout + transfer visibility witness.
// Creates an image, transitions UNDEFINED->TRANSFER_DST, clears it, transitions to TRANSFER_SRC,
// copies to a host-visible buffer, then verifies the pixel bytes after transfer->host synchronization.
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
std::uint32_t memory_type(
    VkPhysicalDevice physical,std::uint32_t bits,VkMemoryPropertyFlags required){
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(physical,&mp);
    for(std::uint32_t i=0;i<mp.memoryTypeCount;++i){
        if((bits&(1u<<i))!=0 && (mp.memoryTypes[i].propertyFlags&required)==required) return i;
    }
    throw std::runtime_error("no compatible memory type");
}
} // namespace

int main(){
    VkInstance instance=VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger=VK_NULL_HANDLE;
    VkDevice device=VK_NULL_HANDLE;
    VkImage image=VK_NULL_HANDLE;
    VkDeviceMemory image_memory=VK_NULL_HANDLE;
    VkBuffer readback=VK_NULL_HANDLE;
    VkDeviceMemory readback_memory=VK_NULL_HANDLE;
    VkCommandPool pool=VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug=nullptr;

    try{
        const bool validation=has_layer("VK_LAYER_KHRONOS_validation");
        const bool debug=has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkApplicationInfo app{};
        app.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName="PROJECTALLMIGHT image layout witness";
        app.apiVersion=VK_API_VERSION_1_3;

        const char* validation_name="VK_LAYER_KHRONOS_validation";
        const char* debug_name=VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        VkInstanceCreateInfo ici{};
        ici.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ici.pApplicationInfo=&app;
        ici.enabledLayerCount=validation?1u:0u;
        ici.ppEnabledLayerNames=validation?&validation_name:nullptr;
        ici.enabledExtensionCount=debug?1u:0u;
        ici.ppEnabledExtensionNames=debug?&debug_name:nullptr;
        check(vkCreateInstance(&ici,nullptr,&instance),"vkCreateInstance");

        auto create_debug=reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT"));
        destroy_debug=reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
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
        if(pc==0) throw std::runtime_error("no Vulkan device");
        std::vector<VkPhysicalDevice> physicals(pc);
        check(vkEnumeratePhysicalDevices(instance,&pc,physicals.data()),"enumerate physical devices");

        VkPhysicalDevice physical=VK_NULL_HANDLE;
        std::uint32_t qfamily=0;
        for(VkPhysicalDevice candidate:physicals){
            VkPhysicalDeviceSynchronization2Features sync2{};
            sync2.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
            VkPhysicalDeviceFeatures2 f2{};
            f2.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext=&sync2;
            vkGetPhysicalDeviceFeatures2(candidate,&f2);
            if(!sync2.synchronization2) continue;
            std::uint32_t qc=0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate,&qc,nullptr);
            std::vector<VkQueueFamilyProperties> qps(qc);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate,&qc,qps.data());
            for(std::uint32_t q=0;q<qc;++q){
                if(qps[q].queueCount>0 && (qps[q].queueFlags&VK_QUEUE_TRANSFER_BIT)!=0){
                    physical=candidate; qfamily=q; break;
                }
            }
            if(physical!=VK_NULL_HANDLE) break;
        }
        if(physical==VK_NULL_HANDLE) throw std::runtime_error("no synchronization2 transfer-capable device");

        const float priority=1.0F;
        VkDeviceQueueCreateInfo qci{};
        qci.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex=qfamily;
        qci.queueCount=1;
        qci.pQueuePriorities=&priority;

        VkPhysicalDeviceSynchronization2Features sync2_enable{};
        sync2_enable.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2_enable.synchronization2=VK_TRUE;

        VkDeviceCreateInfo dci{};
        dci.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.pNext=&sync2_enable;
        dci.queueCreateInfoCount=1;
        dci.pQueueCreateInfos=&qci;
        check(vkCreateDevice(physical,&dci,nullptr,&device),"vkCreateDevice");

        VkQueue queue=VK_NULL_HANDLE;
        vkGetDeviceQueue(device,qfamily,0,&queue);

        constexpr std::uint32_t width=4,height=4;
        VkImageCreateInfo image_ci{};
        image_ci.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_ci.imageType=VK_IMAGE_TYPE_2D;
        image_ci.format=VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.extent={width,height,1};
        image_ci.mipLevels=1;
        image_ci.arrayLayers=1;
        image_ci.samples=VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling=VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        image_ci.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        check(vkCreateImage(device,&image_ci,nullptr,&image),"vkCreateImage");

        VkMemoryRequirements image_req{};
        vkGetImageMemoryRequirements(device,image,&image_req);
        VkMemoryAllocateInfo image_alloc{};
        image_alloc.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc.allocationSize=image_req.size;
        // Any compatible memory works; prefer device-local when available, fall back to no required flags.
        try{
            image_alloc.memoryTypeIndex=memory_type(
                physical,image_req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }catch(const std::runtime_error&){
            image_alloc.memoryTypeIndex=memory_type(physical,image_req.memoryTypeBits,0);
        }
        check(vkAllocateMemory(device,&image_alloc,nullptr,&image_memory),"allocate image memory");
        check(vkBindImageMemory(device,image,image_memory,0),"bind image memory");

        constexpr VkDeviceSize readback_bytes=width*height*4;
        VkBufferCreateInfo bci{};
        bci.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bci.size=readback_bytes;
        bci.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bci.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
        check(vkCreateBuffer(device,&bci,nullptr,&readback),"vkCreateBuffer");

        VkMemoryRequirements br{};
        vkGetBufferMemoryRequirements(device,readback,&br);
        VkMemoryAllocateInfo ba{};
        ba.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ba.allocationSize=br.size;
        ba.memoryTypeIndex=memory_type(
            physical,br.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        check(vkAllocateMemory(device,&ba,nullptr,&readback_memory),"allocate readback memory");
        check(vkBindBufferMemory(device,readback,readback_memory,0),"bind readback memory");

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

        VkCommandBufferBeginInfo begin{};
        begin.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(cmd,&begin),"vkBeginCommandBuffer");

        VkImageMemoryBarrier2 to_dst{};
        to_dst.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        to_dst.srcStageMask=VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        to_dst.srcAccessMask=0;
        to_dst.dstStageMask=VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        to_dst.dstAccessMask=VK_ACCESS_2_TRANSFER_WRITE_BIT;
        to_dst.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        to_dst.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        to_dst.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_dst.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_dst.image=image;
        to_dst.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
        VkDependencyInfo dep1{};
        dep1.sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep1.imageMemoryBarrierCount=1;
        dep1.pImageMemoryBarriers=&to_dst;
        vkCmdPipelineBarrier2(cmd,&dep1);

        const VkClearColorValue clear{{0.25F,0.5F,0.75F,1.0F}};
        vkCmdClearColorImage(
            cmd,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&clear,1,&to_dst.subresourceRange);

        VkImageMemoryBarrier2 to_src{};
        to_src.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        to_src.srcStageMask=VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        to_src.srcAccessMask=VK_ACCESS_2_TRANSFER_WRITE_BIT;
        to_src.dstStageMask=VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        to_src.dstAccessMask=VK_ACCESS_2_TRANSFER_READ_BIT;
        to_src.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        to_src.newLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        to_src.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_src.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_src.image=image;
        to_src.subresourceRange=to_dst.subresourceRange;
        VkDependencyInfo dep2{};
        dep2.sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep2.imageMemoryBarrierCount=1;
        dep2.pImageMemoryBarriers=&to_src;
        vkCmdPipelineBarrier2(cmd,&dep2);

        VkBufferImageCopy copy{};
        copy.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
        copy.imageExtent={width,height,1};
        vkCmdCopyImageToBuffer(
            cmd,image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,readback,1,&copy);

        VkBufferMemoryBarrier2 to_host{};
        to_host.sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        to_host.srcStageMask=VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        to_host.srcAccessMask=VK_ACCESS_2_TRANSFER_WRITE_BIT;
        to_host.dstStageMask=VK_PIPELINE_STAGE_2_HOST_BIT;
        to_host.dstAccessMask=VK_ACCESS_2_HOST_READ_BIT;
        to_host.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_host.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        to_host.buffer=readback;
        to_host.offset=0;
        to_host.size=VK_WHOLE_SIZE;
        VkDependencyInfo dep3{};
        dep3.sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep3.bufferMemoryBarrierCount=1;
        dep3.pBufferMemoryBarriers=&to_host;
        vkCmdPipelineBarrier2(cmd,&dep3);

        check(vkEndCommandBuffer(cmd),"vkEndCommandBuffer");

        VkCommandBufferSubmitInfo cbsi{};
        cbsi.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cbsi.commandBuffer=cmd;
        VkSubmitInfo2 submit{};
        submit.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount=1;
        submit.pCommandBufferInfos=&cbsi;

        VkFenceCreateInfo fci{};
        fci.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence=VK_NULL_HANDLE;
        check(vkCreateFence(device,&fci,nullptr,&fence),"vkCreateFence");
        check(vkQueueSubmit2(queue,1,&submit,fence),"vkQueueSubmit2");
        check(vkWaitForFences(device,1,&fence,VK_TRUE,10'000'000'000ULL),"vkWaitForFences");
        vkDestroyFence(device,fence,nullptr);

        void* mapped=nullptr;
        check(vkMapMemory(device,readback_memory,0,readback_bytes,0,&mapped),"map readback");
        const auto* bytes=static_cast<const std::uint8_t*>(mapped);
        // UNORM clear rounds to the nearest representable channel value. Accept one code of tolerance.
        const int expected[4]={64,128,191,255};
        for(std::size_t pixel=0;pixel<width*height;++pixel){
            for(int c=0;c<4;++c){
                const int actual=bytes[pixel*4+static_cast<std::size_t>(c)];
                if(std::abs(actual-expected[c])>1)
                    throw std::runtime_error("image clear/readback channel mismatch");
            }
        }
        vkUnmapMemory(device,readback_memory);

        vkDeviceWaitIdle(device);
        vkDestroyCommandPool(device,pool,nullptr); pool=VK_NULL_HANDLE;
        vkDestroyBuffer(device,readback,nullptr); readback=VK_NULL_HANDLE;
        vkFreeMemory(device,readback_memory,nullptr); readback_memory=VK_NULL_HANDLE;
        vkDestroyImage(device,image,nullptr); image=VK_NULL_HANDLE;
        vkFreeMemory(device,image_memory,nullptr); image_memory=VK_NULL_HANDLE;
        vkDestroyDevice(device,nullptr); device=VK_NULL_HANDLE;
        if(messenger!=VK_NULL_HANDLE&&destroy_debug) destroy_debug(instance,messenger,nullptr);
        messenger=VK_NULL_HANDLE;
        vkDestroyInstance(instance,nullptr); instance=VK_NULL_HANDLE;

        if(validation_errors.load()!=0) throw std::runtime_error("validation layer reported errors");
        std::cout
            <<"vulkan_image_layout_smoke: PASS\n"
            <<"proved: optimal-tiled image allocation/binding, UNDEFINED->TRANSFER_DST->TRANSFER_SRC layout transitions, clear, image-to-buffer copy, transfer-to-host visibility, byte readback, validation enabled="
            <<(validation?"yes":"no")
            <<"\nnot proved: attachment layouts, shader sampling/storage hazards, per-mip/layer ownership, queue-family transfers, swapchain present layouts, or production renderer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"vulkan_image_layout_smoke: FAIL: "<<e.what()<<'\n';
        if(device!=VK_NULL_HANDLE) vkDeviceWaitIdle(device);
        if(pool!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroyCommandPool(device,pool,nullptr);
        if(readback!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroyBuffer(device,readback,nullptr);
        if(readback_memory!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkFreeMemory(device,readback_memory,nullptr);
        if(image!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkDestroyImage(device,image,nullptr);
        if(image_memory!=VK_NULL_HANDLE&&device!=VK_NULL_HANDLE) vkFreeMemory(device,image_memory,nullptr);
        if(messenger!=VK_NULL_HANDLE&&instance!=VK_NULL_HANDLE&&destroy_debug)
            destroy_debug(instance,messenger,nullptr);
        if(device!=VK_NULL_HANDLE) vkDestroyDevice(device,nullptr);
        if(instance!=VK_NULL_HANDLE) vkDestroyInstance(instance,nullptr);
        return 1;
    }
}
