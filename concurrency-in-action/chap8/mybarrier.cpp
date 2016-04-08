#include <iostream>
#include <thread>
#include <atomic>

class barrier {
private:
    std::atomic<unsigned> count;
	std::atomic<unsigned> spaces;
	std::atomic<unsigned> generation;

    void done_waiting() {
        --count;
        if (!--spaces) {
            spaces = count.load();
            ++generation;
        }
    }

public:
	explicit barrier(unsigned cnt)
	    : count(cnt), spaces(count), generation(0)
	{}
	
	void wait() {
		const unsigned my_generation = generation.load();
		if (!--spaces) {
            spaces = count.load();
			++generation;
		} else {
			while (generation.load() == my_generation)
				std::this_thread::yield();
		}
	}	
};
