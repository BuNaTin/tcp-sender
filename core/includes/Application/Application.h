#pragma once

#include <atomic>
#include <string>
#include <memory>

#include <Application/Args.h>
#include <Config/IniConf.h>
#include <Application/types.h>

namespace tcp_sender {

class Application {
    // interface
public:
    virtual bool start() noexcept = 0;

    virtual std::atomic_flag &getSigHandler() noexcept = 0;

    // constructors
public:
    class Builder;

    virtual ~Application() {}
};

class Application::Builder {
public:
    Builder &setConfig(const std::string &file_name);
    Builder &setArgs(const Args &args);

    std::unique_ptr<Application> build();

private:
    const Args* m_args;
    std::unique_ptr<IniConf> m_config;
    std::size_t m_sig_max;
    std::string m_address;
    short m_port;
};

} // namespace Mib3
