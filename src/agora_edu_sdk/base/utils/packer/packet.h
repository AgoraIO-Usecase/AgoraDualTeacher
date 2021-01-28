//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#pragma once

#include "utils/packer/packer.h"

namespace agora {
namespace commons {
struct packet {
  enum {
    SERVER_TYPE_OFFSET = 2,
    URI_OFFSET = 4,
    BODY_OFFSET = 6,
  };
  explicit packet(uint16_t u) : server_type(0), uri(u) {}
  packet(uint16_t server, uint16_t u) : server_type(server), uri(u) {}
  packet(const packet& rhs) : server_type(rhs.server_type), uri(rhs.uri) {}
  virtual ~packet() {}

  virtual void unmarshall(unpacker& p) { p >> server_type >> uri; }
  virtual void marshall(packer& p) const { p << server_type << uri; }
  virtual void pack(packer& p) const {
    marshall(p);
    p.pack();
  }

  friend packer& operator<<(packer& p, const packet& x) {
    x.marshall(p);
    return p;
  }
  friend unpacker& operator>>(unpacker& p, packet& x) {
    x.unmarshall(p);
    return p;
  }

  uint16_t server_type;
  uint16_t uri;
};

#define DECLARE_PACKET_0(name, s, u)   \
  struct name : packet {               \
    enum { SERVER_TYPE = s, URI = u }; \
    name() : packet(s, u) {}           \
  };
#define DECLARE_PACKET_1(name, s, u, type1, name1) \
  struct name : packet {                           \
    enum { SERVER_TYPE = s, URI = u };             \
    type1 name1;                                   \
    name() : packet(s, u), name1() {}              \
    virtual void unmarshall(unpacker& p) {         \
      packet::unmarshall(p);                       \
      p >> name1;                                  \
    }                                              \
    virtual void marshall(packer& p) const {       \
      packet::marshall(p);                         \
      p << name1;                                  \
    }                                              \
  };
#define DECLARE_PACKET_2(name, s, u, type1, name1, type2, name2) \
  struct name : packet {                                         \
    enum { SERVER_TYPE = s, URI = u };                           \
    type1 name1;                                                 \
    type2 name2;                                                 \
    name() : packet(s, u), name1(), name2() {}                   \
    virtual void unmarshall(unpacker& p) {                       \
      packet::unmarshall(p);                                     \
      p >> name1 >> name2;                                       \
    }                                                            \
    virtual void marshall(packer& p) const {                     \
      packet::marshall(p);                                       \
      p << name1 << name2;                                       \
    }                                                            \
  };
#define DECLARE_PACKET_3(name, s, u, type1, name1, type2, name2, type3, name3) \
  struct name : packet {                                                       \
    enum { SERVER_TYPE = s, URI = u };                                         \
    type1 name1;                                                               \
    type2 name2;                                                               \
    type3 name3;                                                               \
    name() : packet(s, u), name1(), name2(), name3() {}                        \
    virtual void unmarshall(unpacker& p) {                                     \
      packet::unmarshall(p);                                                   \
      p >> name1 >> name2 >> name3;                                            \
    }                                                                          \
    virtual void marshall(packer& p) const {                                   \
      packet::marshall(p);                                                     \
      p << name1 << name2 << name3;                                            \
    }                                                                          \
  };
#define DECLARE_PACKET_4_START(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4) \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    name() : packet(s, u), name1(), name2(), name3(), name4() {}                                   \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4;                                                       \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4;                                                       \
    }

#define DECLARE_PACKET_7_START(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                               type5, name5, type6, name6, type7, name7)                           \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    name() : packet(s, u), name1(), name2(), name3(), name4(), name5(), name6(), name7() {}        \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7;                            \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7;                            \
    }
#define DECLARE_PACKET_8_START(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                               type5, name5, type6, name6, type7, name7, type8, name8)             \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    name()                                                                                         \
        : packet(s, u), name1(), name2(), name3(), name4(), name5(), name6(), name7(), name8() {}  \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8;                   \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8;                   \
    }
#define DECLARE_PACKET_9_START(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                               type5, name5, type6, name6, type7, name7, type8, name8, type9,      \
                               name9)                                                              \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9() {}                                                                               \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9;          \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9;          \
    }

#define DECLARE_PACKET_END \
  }                        \
  ;
#define DECLARE_PACKET_4(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_PACKET_4_START(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_PACKET_END

#define DECLARE_PACKET_5(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                         type5, name5)                                                       \
  struct name : packet {                                                                     \
    enum { SERVER_TYPE = s, URI = u };                                                       \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    name() : packet(s, u), name1(), name2(), name3(), name4(), name5() {}                    \
    virtual void unmarshall(unpacker& p) {                                                   \
      packet::unmarshall(p);                                                                 \
      p >> name1 >> name2 >> name3 >> name4 >> name5;                                        \
    }                                                                                        \
    virtual void marshall(packer& p) const {                                                 \
      packet::marshall(p);                                                                   \
      p << name1 << name2 << name3 << name4 << name5;                                        \
    }                                                                                        \
  };

#define DECLARE_PACKET_6(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                         type5, name5, type6, name6)                                         \
  struct name : packet {                                                                     \
    enum { SERVER_TYPE = s, URI = u };                                                       \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    type6 name6;                                                                             \
    name() : packet(s, u), name1(), name2(), name3(), name4(), name5(), name6() {}           \
    virtual void unmarshall(unpacker& p) {                                                   \
      packet::unmarshall(p);                                                                 \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6;                               \
    }                                                                                        \
    virtual void marshall(packer& p) const {                                                 \
      packet::marshall(p);                                                                   \
      p << name1 << name2 << name3 << name4 << name5 << name6;                               \
    }                                                                                        \
  };

#define DECLARE_PACKET_7(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4, \
                         type5, name5, type6, name6, type7, name7)                           \
  struct name : packet {                                                                     \
    enum { SERVER_TYPE = s, URI = u };                                                       \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    type6 name6;                                                                             \
    type7 name7;                                                                             \
    name() : packet(s, u), name1(), name2(), name3(), name4(), name5(), name6(), name7() {}  \
    virtual void unmarshall(unpacker& p) {                                                   \
      packet::unmarshall(p);                                                                 \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7;                      \
    }                                                                                        \
    virtual void marshall(packer& p) const {                                                 \
      packet::marshall(p);                                                                   \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7;                      \
    }                                                                                        \
  };
#define DECLARE_PACKET_8(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                         type5, name5, type6, name6, type7, name7, type8, name8)                  \
  struct name : packet {                                                                          \
    enum { SERVER_TYPE = s, URI = u };                                                            \
    type1 name1;                                                                                  \
    type2 name2;                                                                                  \
    type3 name3;                                                                                  \
    type4 name4;                                                                                  \
    type5 name5;                                                                                  \
    type6 name6;                                                                                  \
    type7 name7;                                                                                  \
    type8 name8;                                                                                  \
    name()                                                                                        \
        : packet(s, u), name1(), name2(), name3(), name4(), name5(), name6(), name7(), name8() {} \
    virtual void unmarshall(unpacker& p) {                                                        \
      packet::unmarshall(p);                                                                      \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8;                  \
    }                                                                                             \
    virtual void marshall(packer& p) const {                                                      \
      packet::marshall(p);                                                                        \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8;                  \
    }                                                                                             \
  };

#define DECLARE_PACKET_9(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,   \
                         type5, name5, type6, name6, type7, name7, type8, name8, type9, name9) \
  struct name : packet {                                                                       \
    enum { SERVER_TYPE = s, URI = u };                                                         \
    type1 name1;                                                                               \
    type2 name2;                                                                               \
    type3 name3;                                                                               \
    type4 name4;                                                                               \
    type5 name5;                                                                               \
    type6 name6;                                                                               \
    type7 name7;                                                                               \
    type8 name8;                                                                               \
    type9 name9;                                                                               \
    name()                                                                                     \
        : packet(s, u),                                                                        \
          name1(),                                                                             \
          name2(),                                                                             \
          name3(),                                                                             \
          name4(),                                                                             \
          name5(),                                                                             \
          name6(),                                                                             \
          name7(),                                                                             \
          name8(),                                                                             \
          name9() {}                                                                           \
    virtual void unmarshall(unpacker& p) {                                                     \
      packet::unmarshall(p);                                                                   \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9;      \
    }                                                                                          \
    virtual void marshall(packer& p) const {                                                   \
      packet::marshall(p);                                                                     \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9;      \
    }                                                                                          \
  };
#define DECLARE_PACKET_10(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,   \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9, \
                          type10, name10)                                                       \
  struct name : packet {                                                                        \
    enum { SERVER_TYPE = s, URI = u };                                                          \
    type1 name1;                                                                                \
    type2 name2;                                                                                \
    type3 name3;                                                                                \
    type4 name4;                                                                                \
    type5 name5;                                                                                \
    type6 name6;                                                                                \
    type7 name7;                                                                                \
    type8 name8;                                                                                \
    type9 name9;                                                                                \
    type10 name10;                                                                              \
    name()                                                                                      \
        : packet(s, u),                                                                         \
          name1(),                                                                              \
          name2(),                                                                              \
          name3(),                                                                              \
          name4(),                                                                              \
          name5(),                                                                              \
          name6(),                                                                              \
          name7(),                                                                              \
          name8(),                                                                              \
          name9(),                                                                              \
          name10() {}                                                                           \
    virtual void unmarshall(unpacker& p) {                                                      \
      packet::unmarshall(p);                                                                    \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>     \
          name10;                                                                               \
    }                                                                                           \
    virtual void marshall(packer& p) const {                                                    \
      packet::marshall(p);                                                                      \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9        \
        << name10;                                                                              \
    }                                                                                           \
  };

#define DECLARE_PACKET_11(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11)                                          \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11;                                                                        \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11;                                                                                 \
    }                                                                                              \
  };

#define DECLARE_PACKET_12(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11, type12, name12)                          \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    type12 name12;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11(),                                                                                \
          name12() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11 >> name12;                                                              \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11 << name12;                                                                       \
    }                                                                                              \
  };

#define DECLARE_PACKET_13(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11, type12, name12, type13, name13)          \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    type12 name12;                                                                                 \
    type13 name13;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11(),                                                                                \
          name12(),                                                                                \
          name13() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11 >> name12 >> name13;                                                    \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11 << name12 << name13;                                                             \
    }                                                                                              \
  };

#define DECLARE_PACKET_14(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11, type12, name12, type13, name13, type14,  \
                          name14)                                                                  \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    type12 name12;                                                                                 \
    type13 name13;                                                                                 \
    type14 name14;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11(),                                                                                \
          name12(),                                                                                \
          name13(),                                                                                \
          name14() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11 >> name12 >> name13 >> name14;                                          \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11 << name12 << name13 << name14;                                                   \
    }                                                                                              \
  };

#define DECLARE_PACKET_15(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11, type12, name12, type13, name13, type14,  \
                          name14, type15, name15)                                                  \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    type12 name12;                                                                                 \
    type13 name13;                                                                                 \
    type14 name14;                                                                                 \
    type15 name15;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11(),                                                                                \
          name12(),                                                                                \
          name13(),                                                                                \
          name14(),                                                                                \
          name15() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11 >> name12 >> name13 >> name14 >> name15;                                \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11 << name12 << name13 << name14 << name15;                                         \
    }                                                                                              \
  };

#define DECLARE_PACKET_16(name, s, u, type1, name1, type2, name2, type3, name3, type4, name4,      \
                          type5, name5, type6, name6, type7, name7, type8, name8, type9, name9,    \
                          type10, name10, type11, name11, type12, name12, type13, name13, type14,  \
                          name14, type15, name15, type16, name16)                                  \
  struct name : packet {                                                                           \
    enum { SERVER_TYPE = s, URI = u };                                                             \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    type10 name10;                                                                                 \
    type11 name11;                                                                                 \
    type12 name12;                                                                                 \
    type13 name13;                                                                                 \
    type14 name14;                                                                                 \
    type15 name15;                                                                                 \
    type16 name16;                                                                                 \
    name()                                                                                         \
        : packet(s, u),                                                                            \
          name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10(),                                                                                \
          name11(),                                                                                \
          name12(),                                                                                \
          name13(),                                                                                \
          name14(),                                                                                \
          name15(),                                                                                \
          name16() {}                                                                              \
    virtual void unmarshall(unpacker& p) {                                                         \
      packet::unmarshall(p);                                                                       \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9 >>        \
          name10 >> name11 >> name12 >> name13 >> name14 >> name15 >> name16;                      \
    }                                                                                              \
    virtual void marshall(packer& p) const {                                                       \
      packet::marshall(p);                                                                         \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9 << name10 \
        << name11 << name12 << name13 << name14 << name15 << name16;                               \
    }                                                                                              \
  };

}  // namespace commons
}  // namespace agora
