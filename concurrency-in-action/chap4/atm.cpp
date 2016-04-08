#include <string>

struct card_inserted {
    std::string account;
};

class atm {
private:
    messaging::receiver incoming;
    messaging::sender bank;
    messaging::sender interface_hardware;

    void (atm::*state)();

    std::string accound;
    std::string pin;

    void waiting_for_card() {
        interface_hardware.send(display_enter_card());
        incoming.wait().handle<card_inserted>([&](const card_inserted& msg) {
            account = msg.account;
            pin = "";
            interface_hardware.send(display_enter_card());
            state = &atm::getting_pin;
        });
    }

    void getting_pin();

public:
    void run() {
        state = &atm::waiting_for_card;
        try {
            for (;;) {
                this->*state();
            }
        } catch (const messaging::close_queue&) {

        }
    }
};
