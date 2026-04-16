/// MIT License
module;

export module usecase:system_monitor_output_boundary;

import entity;

export namespace usecase {

struct system_monitor_output_boundary {

    virtual ~system_monitor_output_boundary() = default;

    virtual void present_cpu(const entity::cpu& cpu) = 0;
    virtual void present_memory(const entity::memory& memory) = 0;
    virtual void present_gpu(const entity::gpu& gpu) = 0;
    virtual void present_disk(const entity::disk& disk) = 0;
    virtual void present_net(const entity::net& net) = 0;
};

} /// namespace usecase