#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

struct ResourceEntry {
    wgpu::BindGroupEntry bindGroupEntry;
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry;
};

class Resource {
public:
    virtual ResourceEntry resourceEntry() const = 0;

    virtual ~Resource() = default;
};

struct ResourceGroupEntry {
    wgpu::BindGroup wgpuBindGroup;
    wgpu::BindGroupLayout wgpuBindGroupLayout;
};

class ResourceGroup {
public:
    ResourceGroupEntry resourceGroupEntry() {
        if (!mDirty) {
            return ResourceGroupEntry{.wgpuBindGroup = mBindGroup, .wgpuBindGroupLayout = mBindGroupLayout};
        }

        auto& context = WGPUContext::getContext();

        auto device = context.getDevice();

        std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
        bindGroupLayoutEntries.reserve(mResources.size());

        std::vector<wgpu::BindGroupEntry> bindGroupEntries;
        bindGroupEntries.reserve(mResources.size());

        for (std::shared_ptr<Resource>& resource : mResources) {
            ResourceEntry resourceEntry = resource->resourceEntry();

            uint32_t binding = mResourceBindings[reinterpret_cast<uintptr_t>(resource.get())];

            resourceEntry.bindGroupEntry.binding = binding;
            resourceEntry.bindGroupLayoutEntry.binding = binding;

            bindGroupLayoutEntries.push_back(resourceEntry.bindGroupLayoutEntry);
            bindGroupEntries.push_back(resourceEntry.bindGroupEntry);
        }

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = wgpu::Default;
        bindGroupLayoutDesc.entryCount = bindGroupLayoutEntries.size();
        bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();

        mBindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

        wgpu::BindGroupDescriptor bindGroupDesc = wgpu::Default;
        bindGroupDesc.layout = *mBindGroupLayout;
        bindGroupDesc.entries = bindGroupEntries.data();
        bindGroupDesc.entryCount = bindGroupEntries.size();

        mBindGroup = device.createBindGroup(bindGroupDesc);

        mDirty = false;
        return ResourceGroupEntry{.wgpuBindGroup = mBindGroup, .wgpuBindGroupLayout = mBindGroupLayout};
    }

    void setBindingCount(uint32_t tBindingCount) {
        mResources.resize(tBindingCount);
    }
    void bindResource(uint32_t tBindingPosition, std::shared_ptr<Resource> tResource) {
        assert(mResources.size() > tBindingPosition);

        mResourceBindings[reinterpret_cast<uintptr_t>(tResource.get())] = tBindingPosition;

        mResources[tBindingPosition] = std::move(tResource);

        mDirty = true;
    }

    ResourceGroup() = default;

private:
    std::vector<std::shared_ptr<Resource>> mResources;
    std::map<uintptr_t, uint32_t> mResourceBindings;

    RAIIWrapper<wgpu::BindGroup> mBindGroup;
    RAIIWrapper<wgpu::BindGroupLayout> mBindGroupLayout;

    bool mDirty = true;
};