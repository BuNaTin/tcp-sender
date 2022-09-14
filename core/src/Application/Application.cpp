#include <Application/Application.h>

#include <algorithm>
#include <functional>
#include <future>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <utils/strFromHex.hpp>

namespace {

std::string getPath(const std::string &file) {
    std::size_t pos = file.find_last_of('/') + 1;
    return file.substr(0, std::min(pos, file.size()));
}

std::size_t size_t_max() {
    return std::numeric_limits<std::size_t>::max();
}

std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }
} // namespace

namespace tcp_sender {

class ApplicationImpl final : public Application {
    // interface
public:
    bool start() noexcept override;

    std::atomic_flag &getSigHandler() noexcept override {
        return m_sig_handler;
    }

private:
    void hadSigint() noexcept {
        ++m_sig_cnt;
        if (m_sig_cnt > m_sig_max) {
            std::terminate();
        }
        m_sig_handler.clear(std::memory_order_release);
    }

    // constructors
public:
    ApplicationImpl(std::unique_ptr<IniConf> &&conf,
                    const std::size_t sig_max,
                    const int sock_fd,
                    sockaddr_in serv_addr);
    virtual ~ApplicationImpl() = default;

    // data
private:
    std::vector<std::string> m_data;
    std::atomic_flag m_sig_handler = ATOMIC_FLAG_INIT;
    std::size_t m_sig_cnt = 0;
    std::size_t m_sig_max = size_t_max();
    const int m_socket_fd;
    sockaddr_in m_serv_addr;
};

using ABuilder = Application::Builder;

ABuilder &ABuilder::setConfig(const std::string &config_file) {
    m_config = std::make_unique<IniConf>(config_file);
    return *this;
}

ABuilder &ABuilder::setArgs(const Args &args) {
    m_args = &args;
    return *this;
}

std::unique_ptr<Application> Application::Builder::build() {

    using AN = Args::Name;
    if (m_config) {
        // setup settings from config
        if (!*m_config) {
            return nullptr;
        }
        m_sig_max =
                (*m_config)(
                        AN::NAMESPACE_DEFAULT)(AN::CATCH_SIGINT_COUNT)
                        .as<std::size_t>();
        m_address = (*m_config)(AN::NAMESPACE_DEFAULT)(AN::ADDRESS)
                            .as<std::string>();

        m_port = (*m_config)(AN::NAMESPACE_DEFAULT)(AN::PORT)
                         .as<short>();
    }
    if (m_args) {
        // setup setting from args
        if (m_args->has(AN::CATCH_SIGINT_COUNT)) {
            m_sig_max = std::stoi(m_args->get(AN::CATCH_SIGINT_COUNT));
        }
        if (m_args->has(AN::ADDRESS)) {
            m_address = std::stoi(m_args->get(AN::ADDRESS));
        }
        if (m_args->has(AN::PORT)) {
            m_port = std::stoi(m_args->get(AN::PORT));
        }
    } else {
        return nullptr;
    }

    // check settings before building app
    if (m_sig_max == size_t_max()) {
        return nullptr;
    }

    // CREATE SOCKET
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return nullptr;
    }
    server = gethostbyname(m_address.c_str());
    if (server == NULL) {
        std::cerr << "ERROR, no such host" << std::endl;
        return nullptr;
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(m_port);

    if (connect(sockfd,
                (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR connecting" << std::endl;
        return nullptr;
    }

    return std::make_unique<ApplicationImpl>(
            std::move(m_config), m_sig_max, sockfd, serv_addr);
}

ApplicationImpl::ApplicationImpl(std::unique_ptr<IniConf> &&conf,
                                 const std::size_t sig_max,
                                 const int sock_fd,
                                 sockaddr_in serv_addr)
        : m_sig_max(sig_max),
          m_socket_fd(sock_fd),
          m_serv_addr(serv_addr) {
    shutdown_handler = [this](int) {
        hadSigint();
    };
    signal(SIGINT, signal_handler);
    m_sig_handler.test_and_set(std::memory_order_acquire);

    std::vector<std::string> names =
            (*conf)("DEFAULT")("packages").as<std::string[]>();

    auto getData = [this](const std::string &name) {
        std::ifstream in(name);
        if (!in) {
            return;
        }
        std::string buffer;
        std::string data;

        while (in >> buffer) {
            data += utils::strFromHex(buffer);
        }

        m_data.push_back(data);
    };

    std::for_each(names.begin(), names.end(), getData);
}

bool ApplicationImpl::start() noexcept {
    std::cout << "Start work" << std::endl;

    auto sendhex = [this](const std::string &hex) -> bool {
        std::cout << "Send [" << hex << "]\n";
        if (sendto(m_socket_fd,
                   hex.c_str(),
                   hex.size(),
                   MSG_EOR,
                   (struct sockaddr *)&m_serv_addr,
                   sizeof(m_serv_addr)) < 0) {
            return false;
        }
        return true;
    };

    auto it = std::find_if(m_data.begin(), m_data.end(), sendhex);
    if (it != m_data.end()) {
        return false;
    }

    std::cout << "End work" << std::endl;
    return true;
}

} // namespace tcp_sender