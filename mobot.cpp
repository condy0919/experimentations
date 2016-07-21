#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <random>
#include <regex>

namespace {
class Quotation {
public:
    Quotation(std::vector<std::string> svec) : svec_(std::move(svec)) {}

    std::string pick() const {
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_int_distribution<int> dis(0, svec_.size() - 1);
        int dice_roll = dis(generator);
        return svec_[dice_roll];
    };

private:
    const std::vector<std::string> svec_;
};

Quotation HaQuotation(
    {"吼啊！", "你问我姿不姿瓷，我是姿瓷的，我就明确地告诉你这一点。",
     " ( ´∀`)σ 你们啊 naive！I am angry！ ヽ(｀Д´)ノ "
     "你们这样子是不行的，我今天是得罪了你们一下。",
     "暴力膜蛤不可取", "续一秒", "吃枣药丸", "天荒地老蛤不老，我为长者续一秒",
     "春眠不觉晓，闷声续三秒", "Exciting！", "水能载舟 亦可赛艇！", "一颗赛艇",
     "我什么话也不说，这是最好的。但是我想我见到你们这样热情啊，一句话不说也不"
     "好。"});

Quotation HaHaQuotation(
    {"Orz %1%", "%1% 好棒", "%1% 好厉害", "你姿瓷 %1% 吗？当然啦！",
     "%1% 千万要记着，不要“见得风，是得雨”。",
     "接到这个消息，%1% 本身也要判断，明白意思吗？",
     "刚才你问我啊，我可以回答你一句“无可奉告”，你们也不高兴，那怎么办？我讲的"
     "意思不是我钦点 %1%",
     "我告诉你们，我是身经百战了，见得多啦！ %1% 的哪一个身体部位我没prpr过？",
     "你们要知道，开源界的 %1%，那比你们不知要高到哪里去.",
     "%1% ,(๑•̀ㅂ•́)و✧我跟他谈笑风生。",
     "所以说 %1% 呀还是要prpr，识得唔识得啊？唉，我也替你们着急啊，真的。",
     "刚才我很想啊，我每次碰到你们，我就讲开源界有一句话叫“prpr %1%”。",
     "你们啊!!不要总想弄个大新闻，(・Д・)ノ说现在已经 %1% 了，就把我批判一番。",
     "%1% 是属于prpr的，到那时候我们会表态的。明白这意思吧？",
     "连任也要按照 %1% "
     "的法律啊，对不对？要按照基本法的...当然我们的决定权也是很重要的。",
     "但是你一定要问我对C先生姿不姿瓷，我们不姿瓷他？他现在是当%1%"
     "，我们怎么能不姿瓷%1%？对不对？",
     "我很抱歉，我今天是作为一个[长者]"
     "跟你们讲。我不是想prpr，但是我见得太多了。我有这个必要好告诉你们一点%1%"
     "的经验。",
     "但是呢%1%呀，都too simple,sometimes naive.懂了没有？",
     "你们啊，我感觉你们开源界还要学习一个。你们非常熟悉%1%"
     "的这一套理论，你们毕竟还too young.明白我的意思吧？",
     "还是按照%1%的...按照prpr法 (・Д・)ノ"});
}

class IRCBot {
public:
    IRCBot(std::string server, int port)
        : IRCBot(std::move(server), std::to_string(port)) {}

    IRCBot(std::string server, std::string port)
        : server_(std::move(server)),
          port_(std::move(port)),
          beat_(service_),
          sock_(service_) {
        connect();
    }

    ~IRCBot() {
        disconnect();
    }

    void start() {
        service_.run();
    }

protected:
    void async_write(const char* buf, size_t len) {
        boost::asio::async_write(
            sock_, boost::asio::buffer(buf, len),
            [=](boost::system::error_code ec, size_t sent) {
                if (ec) {
                    std::cerr << "[error] "
                              << boost::format("sent %d/%d") % sent % len
                              << '\n';
                    throw "[error] exit";
                }
            });
    }

    void async_write(const char* buf) {
        async_write(buf, std::strlen(buf));
    }

    void async_write(std::string s) {
        async_write(s.data(), s.length());
    }

    void async_read(std::function<void(std::string)> callback) {
        beat_.expires_from_now(boost::posix_time::minutes(5));
        beat_.async_wait(&IRCBot::timeout);

        boost::asio::async_read_until(
            sock_, buf_, "\r\n",
            [this, callback](boost::system::error_code ec, size_t sent) {
                if (!ec) {
                    beat_.cancel_one(ec);

                    std::istream is(&buf_);
                    std::string line;

                    std::getline(is, line);
                    if (line.compare(0, 4, "PING") == 0) {
                        async_write("PONG" + line.substr(4) + "\n");
                    } else {
                        callback(line);
                    }
                    std::cout << "[info] " << line << '\n';
                } else {
                    boost::asio::streambuf::const_buffers_type bufs = buf_.data();
                    std::string s(boost::asio::buffers_begin(bufs),
                                  boost::asio::buffers_end(bufs));
                    std::cerr << "[error] "
                              << "read " << s << '\n';
                    throw "[error] exit";
                }
                async_read(callback);
            });
    }

private:
    static void timeout(const boost::system::error_code& ec) {
        if (ec != boost::asio::error::operation_aborted) {
            throw "[timeout] goodbye";
        }
    }

    void connect() {
        using namespace boost::asio;
        ip::tcp::resolver resolver(service_);
        ip::tcp::resolver::query query(server_, port_);
        ip::tcp::resolver::iterator iter = resolver.resolve(query);
        ip::tcp::endpoint ep = *iter;

        std::cout << "[info] connect to " << ep.address().to_string() << ":"
                  << ep.port() << '\n';
        sock_.connect(ep);
    }

    void disconnect() {
        sock_.close();
    }

    boost::asio::io_service service_;
    boost::asio::deadline_timer beat_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::streambuf buf_;
    std::string server_, port_;
};

class MoBot : public IRCBot {
public:
    MoBot() : IRCBot("irc.freenode.net", 6667) {}

    /// NICK [nickname]
    MoBot& nick(std::string nick) {
        return raw_send("NICK " + nick);
    }

    /// user [mode] [unused] [realname]
    MoBot& user(std::string user) {
        return raw_send("USER " + user + " 0 * :MoBot");
    }

    /// privmsg [target] [msg]
    MoBot& privmsg(std::string msg, std::string to) {
        return raw_send("PRIVMSG " + to + " :" + msg);
    }

    /// join [#channel]
    MoBot& join(std::vector<std::string> channels) {
        for (auto&& chan : channels) {
            raw_send("JOIN " + chan);
        }
        return *this;
    }

    /*
     * PRIVMSG [target] :膜一下\s* =>  膜蛤语录
     * PRIVMSG [target] :膜一下\s*(quininer\s+)+ => Orz/好棒/好厉害 \group
     */
    void mainloop() {
        auto fn = [this](std::string s) {
            std::regex pattern(".* PRIVMSG (\\S+) :(膜|肛)一下([\\S\\s]*)");
            std::smatch match;
            if (std::regex_match(s, match, pattern)) {
                if (match.ready()) {
                    std::string target = match[1].str();
                    std::string arg = boost::trim_right_copy(
                        boost::trim_left_copy(match[3].str()));

                    if (match[2].str() == "肛") {
                        if (!arg.empty()) {
                            auto bfmt = boost::format("%1% 啪啪啪") % arg;
                            privmsg(bfmt.str(), target);
                        }
                    } else if (arg.empty()) {
                        privmsg(HaQuotation.pick(), target);
                    } else {
                        auto bfmt = boost::format(HaHaQuotation.pick()) % arg;
                        privmsg(bfmt.str(), target);
                    }
                }
            }
        };
        async_read(fn);
        start();
    }

private:
    MoBot& raw_send(std::string s) {
        async_write(s + "\r\n");
        return *this;
    }
};

int main() {
    MoBot bot;
    bot.nick("MoBot")
       .user("MoBot")
       .join({"#xiyoulinux", "#archlinux-cn", "#linuxba", "##shuati"});
    try {
        bot.mainloop();
    } catch (const char* str) {
        std::cerr << str << '\n';
        std::exit(-1);
    }
    return 0;
}
