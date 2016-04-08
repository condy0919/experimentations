#include <map>
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <cstdint>

struct dns_entry {
    std::int32_t ip;
};

class dns_cache {
private:
    std::map<std::string, dns_entry> entries;
    mutable boost::shared_mutex entry_mutex;

public:
    dns_entry find_entry(const std::string& domain) const {
        boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
        auto it = entries.find(domain);
        return it == entries.end() ? dns_entry() : it->second;
    }

    void update(const std::string& domain, const dns_entry& entry) {
        std::lock_guard<boost::shared_mutex> lk(entry_mutex);
        entries[domain] = entry;
    }
};
