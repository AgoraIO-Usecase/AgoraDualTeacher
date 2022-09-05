#pragma once
#include <string>
struct EnjoyUserInfo {
    EnjoyUserInfo()
        : channel_id("")
        , local_uid(0)
        , local_str_uid("")
        , remote_uid(0)
        , remote_signalId("")
        , msg_type(0)
        , encoder_type(0)
        , p2p(false)
        , host(true)
        , username("")
    {}
    std::string channel_id;
    std::string local_str_uid;
    unsigned int local_uid;
    unsigned int remote_uid;
    std::string remote_signalId;
    std::string username;
    int msg_type;
    int encoder_type;
    bool p2p;
    bool host;
    int gameId;
};

struct GameInfo {
    GameInfo()
        : hostname("")
        , game_id(0)
        , name("")
        , signal_id("")
        , host_uid(0)
        , img_url("")
        , total_seat_count(0)
        , used_seat_count(0)
        , local_img_url("")
    {}

    bool operator==(const GameInfo& rhs) {
        return game_id == rhs.game_id &&
            host_uid == rhs.host_uid &&
            total_seat_count == rhs.total_seat_count &&
            used_seat_count == rhs.used_seat_count &&
            (hostname.compare(rhs.hostname) == 0) &&
            (name.compare(rhs.name) == 0) &&
            (img_url.compare(rhs.img_url) == 0) &&
            (signal_id.compare(rhs.signal_id) == 0);
           
    }
    int game_id;
    std::string hostname;
    unsigned int host_uid;
    std::string name;
    std::string img_url;
    std::string signal_id;
    std::string local_img_url;
    int total_seat_count;
    int used_seat_count;
};

extern struct EnjoyUserInfo userinfo;
