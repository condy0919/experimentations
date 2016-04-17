#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cassert>
#include <unordered_map>


class Tailer {
public:
    Tailer() {
        if ((notifyfd = ::inotify_init()) == -1) {
            throw "cannot initialize inotify fd";
        }

        if ((epollfd = ::epoll_create(10)) == -1) {
            throw "cannot initialize epoll fd";
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = notifyfd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, notifyfd, &ev) == -1) {
            throw "epoll ctl failed";
        }
    }

    ~Tailer() noexcept {
        for (auto&& pr : files) {
            ::close(filemap[pr.second]);
        }
    }

    void add_watch(const std::string& filename) {
        int wd = ::inotify_add_watch(notifyfd, filename.data(), IN_MODIFY);
        files[filename] = wd;

        int fd = ::open(filename.data(), O_RDONLY);
        if (fd == -1) {
            throw "open file error";
        }
        ::lseek(fd, 0, SEEK_END);
        filemap[wd] = fd;
    }

    void rm_watch(const std::string& filename) {
        auto iter = files.find(filename);
        if (iter == files.end())
            return;

        int wd = iter->second;
        if (::inotify_rm_watch(notifyfd, wd) == -1) {
            throw "rm watch failed";
        }
        ::close(filemap[wd]);
    }

    void monitor() {
        struct epoll_event evs[10];
        alignas(struct inotify_event) char buf[1024];
        while (true) {
            int sz = ::epoll_wait(epollfd, evs, 10, -1);
            for (int i = 0; i < sz; ++i) {
                if (!(evs[i].events & EPOLLIN))
                    continue;

                int fd = evs[i].data.fd;
                int len = ::read(fd, buf, sizeof(buf));
                int idx = 0;
                while (idx < len) {
                    struct inotify_event* pevent =
                        reinterpret_cast<struct inotify_event*>(buf + idx);
                    int event_size =
                        offsetof(struct inotify_event, name) + pevent->len;

                    process(pevent);

                    idx += event_size;
                }
            }
        }
    }

private:
    void process(struct inotify_event* pevent) {
        static char buf[4];
        int fd = filemap[pevent->wd];
        int len;
        while ((len = ::read(fd, buf, sizeof(buf))) != 0) {
            ::write(STDOUT_FILENO, buf, len);
        }
    }

private:
    int epollfd;
    int notifyfd;
    std::unordered_map<std::string, int> files; // filename to fd
    int filemap[1024]; // watch descriptor to file descriptor
};

int main(int argc, char* argv[]) {
    assert(argc == 2);
    const char* file = argv[1];

    Tailer t;
    t.add_watch(file);
    t.monitor();

    return 0;
}
