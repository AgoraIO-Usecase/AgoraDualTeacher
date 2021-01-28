//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

namespace agora {
namespace commons {
#define DECLARE_STRUCT_END \
  }                        \
  ;

#define DECLARE_SIMPLE_STRUCT_1_START(name, type1, name1) \
  struct name {                                           \
    type1 name1;                                          \
    name() : name1() {}                                   \
    name(type1 n1) : name1(n1) {}                         \
    name(const name& r) : name1(r.name1) {}               \
    name& operator=(const name& r) {                      \
      if (this == &r) return *this;                       \
      name1 = r.name1;                                    \
      return *this;                                       \
    }
#define DECLARE_SIMPLE_STRUCT_1(name, type1, name1) \
  DECLARE_SIMPLE_STRUCT_1_START(name, type1, name1) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_2_START(name, type1, name1, type2, name2) \
  struct name {                                                         \
    type1 name1;                                                        \
    type2 name2;                                                        \
    name() : name1(), name2() {}                                        \
    name(type1 n1, type2 n2) : name1(n1), name2(n2) {}                  \
    name(const name& r) : name1(r.name1), name2(r.name2) {}             \
    name& operator=(const name& r) {                                    \
      if (this == &r) return *this;                                     \
      name1 = r.name1;                                                  \
      name2 = r.name2;                                                  \
      return *this;                                                     \
    }
#define DECLARE_SIMPLE_STRUCT_2(name, type1, name1, type2, name2) \
  DECLARE_SIMPLE_STRUCT_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3) \
  struct name {                                                                       \
    type1 name1;                                                                      \
    type2 name2;                                                                      \
    type3 name3;                                                                      \
    name() : name1(), name2(), name3() {}                                             \
    name(type1 n1, type2 n2, type3 n3) : name1(n1), name2(n2), name3(n3) {}           \
    name(const name& r) : name1(r.name1), name2(r.name2), name3(r.name3) {}           \
    name& operator=(const name& r) {                                                  \
      if (this == &r) return *this;                                                   \
      name1 = r.name1;                                                                \
      name2 = r.name2;                                                                \
      name3 = r.name3;                                                                \
      return *this;                                                                   \
    }
#define DECLARE_SIMPLE_STRUCT_3(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_SIMPLE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4,     \
                                      name4)                                                     \
  struct name {                                                                                  \
    type1 name1;                                                                                 \
    type2 name2;                                                                                 \
    type3 name3;                                                                                 \
    type4 name4;                                                                                 \
    name() : name1(), name2(), name3(), name4() {}                                               \
    name(type1 n1, type2 n2, type3 n3, type4 n4) : name1(n1), name2(n2), name3(n3), name4(n4) {} \
    name(const name& r) : name1(r.name1), name2(r.name2), name3(r.name3), name4(r.name4) {}      \
    name& operator=(const name& r) {                                                             \
      if (this == &r) return *this;                                                              \
      name1 = r.name1;                                                                           \
      name2 = r.name2;                                                                           \
      name3 = r.name3;                                                                           \
      name4 = r.name4;                                                                           \
      return *this;                                                                              \
    }
#define DECLARE_SIMPLE_STRUCT_4(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_SIMPLE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, \
                                      name4, type5, name5)                                   \
  struct name {                                                                              \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    name() : name1(), name2(), name3(), name4(), name5() {}                                  \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5)                                   \
        : name1(n1), name2(n2), name3(n3), name4(n4), name5(n5) {}                           \
    name(const name& r)                                                                      \
        : name1(r.name1), name2(r.name2), name3(r.name3), name4(r.name4), name5(r.name5) {}  \
    name& operator=(const name& r) {                                                         \
      if (this == &r) return *this;                                                          \
      name1 = r.name1;                                                                       \
      name2 = r.name2;                                                                       \
      name3 = r.name3;                                                                       \
      name4 = r.name4;                                                                       \
      name5 = r.name5;                                                                       \
      return *this;                                                                          \
    }
#define DECLARE_SIMPLE_STRUCT_5(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5)                                                 \
  DECLARE_SIMPLE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5)                                                 \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, \
                                      name4, type5, name5, type6, name6)                     \
  struct name {                                                                              \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    type6 name6;                                                                             \
    name() : name1(), name2(), name3(), name4(), name5(), name6() {}                         \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6)                         \
        : name1(n1), name2(n2), name3(n3), name4(n4), name5(n5), name6(n6) {}                \
    name(const name& r)                                                                      \
        : name1(r.name1),                                                                    \
          name2(r.name2),                                                                    \
          name3(r.name3),                                                                    \
          name4(r.name4),                                                                    \
          name5(r.name5),                                                                    \
          name6(r.name6) {}                                                                  \
    name& operator=(const name& r) {                                                         \
      if (this == &r) return *this;                                                          \
      name1 = r.name1;                                                                       \
      name2 = r.name2;                                                                       \
      name3 = r.name3;                                                                       \
      name4 = r.name4;                                                                       \
      name5 = r.name5;                                                                       \
      name6 = r.name6;                                                                       \
      return *this;                                                                          \
    }
#define DECLARE_SIMPLE_STRUCT_6(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6)                                   \
  DECLARE_SIMPLE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6)                                   \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, \
                                      name4, type5, name5, type6, name6, type7, name7)       \
  struct name {                                                                              \
    type1 name1;                                                                             \
    type2 name2;                                                                             \
    type3 name3;                                                                             \
    type4 name4;                                                                             \
    type5 name5;                                                                             \
    type6 name6;                                                                             \
    type7 name7;                                                                             \
    name() : name1(), name2(), name3(), name4(), name5(), name6(), name7() {}                \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7)               \
        : name1(n1), name2(n2), name3(n3), name4(n4), name5(n5), name6(n6), name7(n7) {}     \
    name(const name& r)                                                                      \
        : name1(r.name1),                                                                    \
          name2(r.name2),                                                                    \
          name3(r.name3),                                                                    \
          name4(r.name4),                                                                    \
          name5(r.name5),                                                                    \
          name6(r.name6),                                                                    \
          name7(r.name7) {}                                                                  \
    name& operator=(const name& r) {                                                         \
      if (this == &r) return *this;                                                          \
      name1 = r.name1;                                                                       \
      name2 = r.name2;                                                                       \
      name3 = r.name3;                                                                       \
      name4 = r.name4;                                                                       \
      name5 = r.name5;                                                                       \
      name6 = r.name6;                                                                       \
      name7 = r.name7;                                                                       \
      return *this;                                                                          \
    }
#define DECLARE_SIMPLE_STRUCT_7(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7)                     \
  DECLARE_SIMPLE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7)                     \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4,  \
                                      name4, type5, name5, type6, name6, type7, name7, type8, \
                                      name8)                                                  \
  struct name {                                                                               \
    type1 name1;                                                                              \
    type2 name2;                                                                              \
    type3 name3;                                                                              \
    type4 name4;                                                                              \
    type5 name5;                                                                              \
    type6 name6;                                                                              \
    type7 name7;                                                                              \
    type8 name8;                                                                              \
    name() : name1(), name2(), name3(), name4(), name5(), name6(), name7(), name8() {}        \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8)      \
        : name1(n1),                                                                          \
          name2(n2),                                                                          \
          name3(n3),                                                                          \
          name4(n4),                                                                          \
          name5(n5),                                                                          \
          name6(n6),                                                                          \
          name7(n7),                                                                          \
          name8(n8) {}                                                                        \
    name(const name& r)                                                                       \
        : name1(r.name1),                                                                     \
          name2(r.name2),                                                                     \
          name3(r.name3),                                                                     \
          name4(r.name4),                                                                     \
          name5(r.name5),                                                                     \
          name6(r.name6),                                                                     \
          name7(r.name7),                                                                     \
          name8(r.name8) {}                                                                   \
    name& operator=(const name& r) {                                                          \
      if (this == &r) return *this;                                                           \
      name1 = r.name1;                                                                        \
      name2 = r.name2;                                                                        \
      name3 = r.name3;                                                                        \
      name4 = r.name4;                                                                        \
      name5 = r.name5;                                                                        \
      name6 = r.name6;                                                                        \
      name7 = r.name7;                                                                        \
      name8 = r.name8;                                                                        \
      return *this;                                                                           \
    }
#define DECLARE_SIMPLE_STRUCT_8(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7, type8, name8)       \
  DECLARE_SIMPLE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7, type8, name8)       \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4,       \
                                      name4, type5, name5, type6, name6, type7, name7, type8,      \
                                      name8, type9, name9)                                         \
  struct name {                                                                                    \
    type1 name1;                                                                                   \
    type2 name2;                                                                                   \
    type3 name3;                                                                                   \
    type4 name4;                                                                                   \
    type5 name5;                                                                                   \
    type6 name6;                                                                                   \
    type7 name7;                                                                                   \
    type8 name8;                                                                                   \
    type9 name9;                                                                                   \
    name() : name1(), name2(), name3(), name4(), name5(), name6(), name7(), name8(), name9() {}    \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9) \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9) {}                                                                             \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9) {}                                                                        \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_9(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                name9)                                                         \
  DECLARE_SIMPLE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                name9)                                                         \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4,      \
                                       name4, type5, name5, type6, name6, type7, name7, type8,     \
                                       name8, type9, name9, type10, name10)                        \
  struct name {                                                                                    \
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
    name()                                                                                         \
        : name1(),                                                                                 \
          name2(),                                                                                 \
          name3(),                                                                                 \
          name4(),                                                                                 \
          name5(),                                                                                 \
          name6(),                                                                                 \
          name7(),                                                                                 \
          name8(),                                                                                 \
          name9(),                                                                                 \
          name10() {}                                                                              \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10)                                                                               \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_10(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10)                                         \
  DECLARE_SIMPLE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10)                                         \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4,      \
                                       name4, type5, name5, type6, name6, type7, name7, type8,     \
                                       name8, type9, name9, type10, name10, type11, name11)        \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11)                                                                   \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_11(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11)                         \
  DECLARE_SIMPLE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11)                         \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_12_START(                                                            \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,      \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12)      \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12)                                                       \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_12(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12)         \
  DECLARE_SIMPLE_STRUCT_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12)         \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4,      \
                                       name4, type5, name5, type6, name6, type7, name7, type8,     \
                                       name8, type9, name9, type10, name10, type11, name11,        \
                                       type12, name12, type13, name13)                             \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13)                                           \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_13(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13)                                                        \
  DECLARE_SIMPLE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13)                                                        \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4,      \
                                       name4, type5, name5, type6, name6, type7, name7, type8,     \
                                       name8, type9, name9, type10, name10, type11, name11,        \
                                       type12, name12, type13, name13, type14, name14)             \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14)                               \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_14(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14)                                        \
  DECLARE_SIMPLE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14)                                        \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_15_START(                                                            \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,      \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,      \
    type13, name13, type14, name14, type15, name15)                                                \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14, type15 n15)                   \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14),                                                                             \
          name15(n15) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14),                                                                        \
          name15(r.name15) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      name15 = r.name15;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_15(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15)                        \
  DECLARE_SIMPLE_STRUCT_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15)                        \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_16_START(                                                            \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,      \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,      \
    type13, name13, type14, name14, type15, name15, type16, name16)                                \
  struct name {                                                                                    \
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
        : name1(),                                                                                 \
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
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14, type15 n15, type16 n16)       \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14),                                                                             \
          name15(n15),                                                                             \
          name16(n16) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14),                                                                        \
          name15(r.name15),                                                                        \
          name16(r.name16) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      name15 = r.name15;                                                                           \
      name16 = r.name16;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_16(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15, type16, name16)        \
  DECLARE_SIMPLE_STRUCT_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15, type16, name16)        \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_17_START(                                                            \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,      \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,      \
    type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)                \
  struct name {                                                                                    \
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
    type17 name17;                                                                                 \
    name()                                                                                         \
        : name1(),                                                                                 \
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
          name16(),                                                                                \
          name17() {}                                                                              \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14, type15 n15, type16 n16,       \
         type17 n17)                                                                               \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14),                                                                             \
          name15(n15),                                                                             \
          name16(n16),                                                                             \
          name17(n17) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14),                                                                        \
          name15(r.name15),                                                                        \
          name16(r.name16),                                                                        \
          name17(r.name17) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      name15 = r.name15;                                                                           \
      name16 = r.name16;                                                                           \
      name17 = r.name17;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_17(                                                               \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,   \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,   \
    type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)             \
  DECLARE_SIMPLE_STRUCT_17_START(                                                               \
      name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6, \
      type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12, \
      type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)           \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4,      \
                                       name4, type5, name5, type6, name6, type7, name7, type8,     \
                                       name8, type9, name9, type10, name10, type11, name11,        \
                                       type12, name12, type13, name13, type14, name14, type15,     \
                                       name15, type16, name16, type17, name17, type18, name18)     \
  struct name {                                                                                    \
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
    type17 name17;                                                                                 \
    type18 name18;                                                                                 \
    name()                                                                                         \
        : name1(),                                                                                 \
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
          name16(),                                                                                \
          name17(),                                                                                \
          name18() {}                                                                              \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14, type15 n15, type16 n16,       \
         type17 n17, type18 n18)                                                                   \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14),                                                                             \
          name15(n15),                                                                             \
          name16(n16),                                                                             \
          name17(n17),                                                                             \
          name18(n18) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14),                                                                        \
          name15(r.name15),                                                                        \
          name16(r.name16),                                                                        \
          name17(r.name17),                                                                        \
          name18(r.name18) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      name15 = r.name15;                                                                           \
      name16 = r.name16;                                                                           \
      name17 = r.name17;                                                                           \
      name18 = r.name18;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_18(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18)                                         \
  DECLARE_SIMPLE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18)                                         \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_19_START(                                                            \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,      \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,      \
    type13, name13, type14, name14, type15, name15, type16, name16, type17, name17, type18,        \
    name18, type19, name19)                                                                        \
  struct name {                                                                                    \
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
    type17 name17;                                                                                 \
    type18 name18;                                                                                 \
    type19 name19;                                                                                 \
    name()                                                                                         \
        : name1(),                                                                                 \
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
          name16(),                                                                                \
          name17(),                                                                                \
          name18(),                                                                                \
          name19() {}                                                                              \
    name(type1 n1, type2 n2, type3 n3, type4 n4, type5 n5, type6 n6, type7 n7, type8 n8, type9 n9, \
         type10 n10, type11 n11, type12 n12, type13 n13, type14 n14, type15 n15, type16 n16,       \
         type17 n17, type18 n18, type19 n19)                                                       \
        : name1(n1),                                                                               \
          name2(n2),                                                                               \
          name3(n3),                                                                               \
          name4(n4),                                                                               \
          name5(n5),                                                                               \
          name6(n6),                                                                               \
          name7(n7),                                                                               \
          name8(n8),                                                                               \
          name9(n9),                                                                               \
          name10(n10),                                                                             \
          name11(n11),                                                                             \
          name12(n12),                                                                             \
          name13(n13),                                                                             \
          name14(n14),                                                                             \
          name15(n15),                                                                             \
          name16(n16),                                                                             \
          name17(n17),                                                                             \
          name18(n18),                                                                             \
          name19(n19) {}                                                                           \
    name(const name& r)                                                                            \
        : name1(r.name1),                                                                          \
          name2(r.name2),                                                                          \
          name3(r.name3),                                                                          \
          name4(r.name4),                                                                          \
          name5(r.name5),                                                                          \
          name6(r.name6),                                                                          \
          name7(r.name7),                                                                          \
          name8(r.name8),                                                                          \
          name9(r.name9),                                                                          \
          name10(r.name10),                                                                        \
          name11(r.name11),                                                                        \
          name12(r.name12),                                                                        \
          name13(r.name13),                                                                        \
          name14(r.name14),                                                                        \
          name15(r.name15),                                                                        \
          name16(r.name16),                                                                        \
          name17(r.name17),                                                                        \
          name18(r.name18),                                                                        \
          name19(r.name19) {}                                                                      \
    name& operator=(const name& r) {                                                               \
      if (this == &r) return *this;                                                                \
      name1 = r.name1;                                                                             \
      name2 = r.name2;                                                                             \
      name3 = r.name3;                                                                             \
      name4 = r.name4;                                                                             \
      name5 = r.name5;                                                                             \
      name6 = r.name6;                                                                             \
      name7 = r.name7;                                                                             \
      name8 = r.name8;                                                                             \
      name9 = r.name9;                                                                             \
      name10 = r.name10;                                                                           \
      name11 = r.name11;                                                                           \
      name12 = r.name12;                                                                           \
      name13 = r.name13;                                                                           \
      name14 = r.name14;                                                                           \
      name15 = r.name15;                                                                           \
      name16 = r.name16;                                                                           \
      name17 = r.name17;                                                                           \
      name18 = r.name18;                                                                           \
      name19 = r.name19;                                                                           \
      return *this;                                                                                \
    }
#define DECLARE_SIMPLE_STRUCT_19(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18, type19, name19)                         \
  DECLARE_SIMPLE_STRUCT_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18, type19, name19)                         \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_1_START(name, type1, name1)       \
  DECLARE_SIMPLE_STRUCT_1_START(name, type1, name1)      \
  friend bool operator<(const name& l, const name& r) {  \
    if (l.name1 != r.name1) {                            \
      return l.name1 < r.name1;                          \
    }                                                    \
    return false;                                        \
  }                                                      \
  friend bool operator==(const name& l, const name& r) { \
    if (l.name1 != r.name1) {                            \
      return false;                                      \
    }                                                    \
    return true;                                         \
  }                                                      \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_1(name, type1, name1) \
  DECLARE_STRUCT_1_START(name, type1, name1) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_2_START(name, type1, name1, type2, name2)  \
  DECLARE_SIMPLE_STRUCT_2_START(name, type1, name1, type2, name2) \
  friend bool operator<(const name& l, const name& r) {           \
    if (l.name1 != r.name1) {                                     \
      return l.name1 < r.name1;                                   \
    }                                                             \
    if (l.name2 != r.name2) {                                     \
      return l.name2 < r.name2;                                   \
    }                                                             \
    return false;                                                 \
  }                                                               \
  friend bool operator==(const name& l, const name& r) {          \
    if (l.name1 != r.name1) {                                     \
      return false;                                               \
    }                                                             \
    if (l.name2 != r.name2) {                                     \
      return false;                                               \
    }                                                             \
    return true;                                                  \
  }                                                               \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_2(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3)  \
  DECLARE_SIMPLE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3) \
  friend bool operator<(const name& l, const name& r) {                         \
    if (l.name1 != r.name1) {                                                   \
      return l.name1 < r.name1;                                                 \
    }                                                                           \
    if (l.name2 != r.name2) {                                                   \
      return l.name2 < r.name2;                                                 \
    }                                                                           \
    if (l.name3 != r.name3) {                                                   \
      return l.name3 < r.name3;                                                 \
    }                                                                           \
    return false;                                                               \
  }                                                                             \
  friend bool operator==(const name& l, const name& r) {                        \
    if (l.name1 != r.name1) {                                                   \
      return false;                                                             \
    }                                                                           \
    if (l.name2 != r.name2) {                                                   \
      return false;                                                             \
    }                                                                           \
    if (l.name3 != r.name3) {                                                   \
      return false;                                                             \
    }                                                                           \
    return true;                                                                \
  }                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_3(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4)  \
  DECLARE_SIMPLE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  friend bool operator<(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                 \
      return l.name1 < r.name1;                                                               \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return l.name2 < r.name2;                                                               \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return l.name3 < r.name3;                                                               \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return l.name4 < r.name4;                                                               \
    }                                                                                         \
    return false;                                                                             \
  }                                                                                           \
  friend bool operator==(const name& l, const name& r) {                                      \
    if (l.name1 != r.name1) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    return true;                                                                              \
  }                                                                                           \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_4(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                               type5, name5)                                                  \
  DECLARE_SIMPLE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5)                                                 \
  friend bool operator<(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                 \
      return l.name1 < r.name1;                                                               \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return l.name2 < r.name2;                                                               \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return l.name3 < r.name3;                                                               \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return l.name4 < r.name4;                                                               \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return l.name5 < r.name5;                                                               \
    }                                                                                         \
    return false;                                                                             \
  }                                                                                           \
  friend bool operator==(const name& l, const name& r) {                                      \
    if (l.name1 != r.name1) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    return true;                                                                              \
  }                                                                                           \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_5(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5)                                                               \
  DECLARE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5)                                                               \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                               type5, name5, type6, name6)                                    \
  DECLARE_SIMPLE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6)                                   \
  friend bool operator<(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                 \
      return l.name1 < r.name1;                                                               \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return l.name2 < r.name2;                                                               \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return l.name3 < r.name3;                                                               \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return l.name4 < r.name4;                                                               \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return l.name5 < r.name5;                                                               \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return l.name6 < r.name6;                                                               \
    }                                                                                         \
    return false;                                                                             \
  }                                                                                           \
  friend bool operator==(const name& l, const name& r) {                                      \
    if (l.name1 != r.name1) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    return true;                                                                              \
  }                                                                                           \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_6(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6)                                                 \
  DECLARE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6)                                                 \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                               type5, name5, type6, name6, type7, name7)                      \
  DECLARE_SIMPLE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7)                     \
  friend bool operator<(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                 \
      return l.name1 < r.name1;                                                               \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return l.name2 < r.name2;                                                               \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return l.name3 < r.name3;                                                               \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return l.name4 < r.name4;                                                               \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return l.name5 < r.name5;                                                               \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return l.name6 < r.name6;                                                               \
    }                                                                                         \
    if (l.name7 != r.name7) {                                                                 \
      return l.name7 < r.name7;                                                               \
    }                                                                                         \
    return false;                                                                             \
  }                                                                                           \
  friend bool operator==(const name& l, const name& r) {                                      \
    if (l.name1 != r.name1) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name7 != r.name7) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    return true;                                                                              \
  }                                                                                           \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_7(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7)                                   \
  DECLARE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7)                                   \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                               type5, name5, type6, name6, type7, name7, type8, name8)        \
  DECLARE_SIMPLE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                type5, name5, type6, name6, type7, name7, type8, name8)       \
  friend bool operator<(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                 \
      return l.name1 < r.name1;                                                               \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return l.name2 < r.name2;                                                               \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return l.name3 < r.name3;                                                               \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return l.name4 < r.name4;                                                               \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return l.name5 < r.name5;                                                               \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return l.name6 < r.name6;                                                               \
    }                                                                                         \
    if (l.name7 != r.name7) {                                                                 \
      return l.name7 < r.name7;                                                               \
    }                                                                                         \
    if (l.name8 != r.name8) {                                                                 \
      return l.name8 < r.name8;                                                               \
    }                                                                                         \
    return false;                                                                             \
  }                                                                                           \
  friend bool operator==(const name& l, const name& r) {                                      \
    if (l.name1 != r.name1) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name2 != r.name2) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name3 != r.name3) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name4 != r.name4) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name5 != r.name5) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name6 != r.name6) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name7 != r.name7) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    if (l.name8 != r.name8) {                                                                 \
      return false;                                                                           \
    }                                                                                         \
    return true;                                                                              \
  }                                                                                           \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_8(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7, type8, name8)                     \
  DECLARE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7, type8, name8)                     \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                               type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                               name9)                                                          \
  DECLARE_SIMPLE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                name9)                                                         \
  friend bool operator<(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                  \
      return l.name1 < r.name1;                                                                \
    }                                                                                          \
    if (l.name2 != r.name2) {                                                                  \
      return l.name2 < r.name2;                                                                \
    }                                                                                          \
    if (l.name3 != r.name3) {                                                                  \
      return l.name3 < r.name3;                                                                \
    }                                                                                          \
    if (l.name4 != r.name4) {                                                                  \
      return l.name4 < r.name4;                                                                \
    }                                                                                          \
    if (l.name5 != r.name5) {                                                                  \
      return l.name5 < r.name5;                                                                \
    }                                                                                          \
    if (l.name6 != r.name6) {                                                                  \
      return l.name6 < r.name6;                                                                \
    }                                                                                          \
    if (l.name7 != r.name7) {                                                                  \
      return l.name7 < r.name7;                                                                \
    }                                                                                          \
    if (l.name8 != r.name8) {                                                                  \
      return l.name8 < r.name8;                                                                \
    }                                                                                          \
    if (l.name9 != r.name9) {                                                                  \
      return l.name9 < r.name9;                                                                \
    }                                                                                          \
    return false;                                                                              \
  }                                                                                            \
  friend bool operator==(const name& l, const name& r) {                                       \
    if (l.name1 != r.name1) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name2 != r.name2) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name3 != r.name3) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name4 != r.name4) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name5 != r.name5) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name6 != r.name6) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name7 != r.name7) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name8 != r.name8) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    if (l.name9 != r.name9) {                                                                  \
      return false;                                                                            \
    }                                                                                          \
    return true;                                                                               \
  }                                                                                            \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_9(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7, type8, name8, type9, name9)       \
  DECLARE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                         name5, type6, name6, type7, name7, type8, name8, type9, name9)       \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10)                                          \
  DECLARE_SIMPLE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10)                                         \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_10(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10)                                                                \
  DECLARE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10)                                                                \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11)                          \
  DECLARE_SIMPLE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11)                         \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_11(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11)                                                \
  DECLARE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11)                                                \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11, type12, name12)          \
  DECLARE_SIMPLE_STRUCT_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12)         \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_12(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12)                                \
  DECLARE_STRUCT_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12)                                \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11, type12, name12, type13,  \
                                name13)                                                         \
  DECLARE_SIMPLE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13)                                                        \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    if (l.name13 != r.name13) {                                                                 \
      return l.name13 < r.name13;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name13 != r.name13) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_13(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12, type13, name13)                \
  DECLARE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12, type13, name13)                \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11, type12, name12, type13,  \
                                name13, type14, name14)                                         \
  DECLARE_SIMPLE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14)                                        \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name13 != r.name13) {                                                                 \
      return l.name13 < r.name13;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name14 != r.name14) {                                                                 \
      return l.name14 < r.name14;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name13 != r.name13) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name14 != r.name14) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_14(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14) \
  DECLARE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14) \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11, type12, name12, type13,  \
                                name13, type14, name14, type15, name15)                         \
  DECLARE_SIMPLE_STRUCT_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15)                        \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name13 != r.name13) {                                                                 \
      return l.name13 < r.name13;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name14 != r.name14) {                                                                 \
      return l.name14 < r.name14;                                                               \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return l.name15 < r.name15;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name13 != r.name13) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name14 != r.name14) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_15(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15)                                                         \
  DECLARE_STRUCT_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15)                                                         \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                name9, type10, name10, type11, name11, type12, name12, type13,  \
                                name13, type14, name14, type15, name15, type16, name16)         \
  DECLARE_SIMPLE_STRUCT_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                 name9, type10, name10, type11, name11, type12, name12, type13, \
                                 name13, type14, name14, type15, name15, type16, name16)        \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    if (l.name13 != r.name13) {                                                                 \
      return l.name13 < r.name13;                                                               \
    }                                                                                           \
    if (l.name14 != r.name14) {                                                                 \
      return l.name14 < r.name14;                                                               \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return l.name15 < r.name15;                                                               \
    }                                                                                           \
    if (l.name16 != r.name16) {                                                                 \
      return l.name16 < r.name16;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name13 != r.name13) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name14 != r.name14) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name16 != r.name16) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_16(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16)                                         \
  DECLARE_STRUCT_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16)                                         \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_17_START(                                                                \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,   \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,   \
    type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)             \
  DECLARE_SIMPLE_STRUCT_17_START(                                                               \
      name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6, \
      type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12, \
      type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)           \
  friend bool operator<(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                   \
      return l.name1 < r.name1;                                                                 \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return l.name2 < r.name2;                                                                 \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return l.name3 < r.name3;                                                                 \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return l.name4 < r.name4;                                                                 \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return l.name5 < r.name5;                                                                 \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return l.name6 < r.name6;                                                                 \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return l.name7 < r.name7;                                                                 \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return l.name8 < r.name8;                                                                 \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return l.name9 < r.name9;                                                                 \
    }                                                                                           \
    return false;                                                                               \
    if (l.name10 != r.name10) {                                                                 \
      return l.name10 < r.name10;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name11 != r.name11) {                                                                 \
      return l.name11 < r.name11;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name12 != r.name12) {                                                                 \
      return l.name12 < r.name12;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name13 != r.name13) {                                                                 \
      return l.name13 < r.name13;                                                               \
    }                                                                                           \
    return false;                                                                               \
    if (l.name14 != r.name14) {                                                                 \
      return l.name14 < r.name14;                                                               \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return l.name15 < r.name15;                                                               \
    }                                                                                           \
    if (l.name16 != r.name16) {                                                                 \
      return l.name16 < r.name16;                                                               \
    }                                                                                           \
    if (l.name17 != r.name17) {                                                                 \
      return l.name17 < r.name17;                                                               \
    }                                                                                           \
    return false;                                                                               \
  }                                                                                             \
  friend bool operator==(const name& l, const name& r) {                                        \
    if (l.name1 != r.name1) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name2 != r.name2) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name3 != r.name3) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name4 != r.name4) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name5 != r.name5) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name6 != r.name6) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name7 != r.name7) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name8 != r.name8) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    if (l.name9 != r.name9) {                                                                   \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name10 != r.name10) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name11 != r.name11) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name12 != r.name12) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name13 != r.name13) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
    if (l.name14 != r.name14) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name15 != r.name15) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name16 != r.name16) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    if (l.name17 != r.name17) {                                                                 \
      return false;                                                                             \
    }                                                                                           \
    return true;                                                                                \
  }                                                                                             \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_17(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17)                         \
  DECLARE_STRUCT_17_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17)                         \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4,    \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,   \
                                name9, type10, name10, type11, name11, type12, name12, type13,   \
                                name13, type14, name14, type15, name15, type16, name16, type17,  \
                                name17, type18, name18)                                          \
  DECLARE_SIMPLE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18)                                         \
  friend bool operator<(const name& l, const name& r) {                                          \
    if (l.name1 != r.name1) {                                                                    \
      return l.name1 < r.name1;                                                                  \
    }                                                                                            \
    if (l.name2 != r.name2) {                                                                    \
      return l.name2 < r.name2;                                                                  \
    }                                                                                            \
    if (l.name3 != r.name3) {                                                                    \
      return l.name3 < r.name3;                                                                  \
    }                                                                                            \
    if (l.name4 != r.name4) {                                                                    \
      return l.name4 < r.name4;                                                                  \
    }                                                                                            \
    if (l.name5 != r.name5) {                                                                    \
      return l.name5 < r.name5;                                                                  \
    }                                                                                            \
    if (l.name6 != r.name6) {                                                                    \
      return l.name6 < r.name6;                                                                  \
    }                                                                                            \
    if (l.name7 != r.name7) {                                                                    \
      return l.name7 < r.name7;                                                                  \
    }                                                                                            \
    if (l.name8 != r.name8) {                                                                    \
      return l.name8 < r.name8;                                                                  \
    }                                                                                            \
    if (l.name9 != r.name9) {                                                                    \
      return l.name9 < r.name9;                                                                  \
    }                                                                                            \
    return false;                                                                                \
    if (l.name10 != r.name10) {                                                                  \
      return l.name10 < r.name10;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name11 != r.name11) {                                                                  \
      return l.name11 < r.name11;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name12 != r.name12) {                                                                  \
      return l.name12 < r.name12;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name13 != r.name13) {                                                                  \
      return l.name13 < r.name13;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name14 != r.name14) {                                                                  \
      return l.name14 < r.name14;                                                                \
    }                                                                                            \
    if (l.name15 != r.name15) {                                                                  \
      return l.name15 < r.name15;                                                                \
    }                                                                                            \
    if (l.name16 != r.name16) {                                                                  \
      return l.name16 < r.name16;                                                                \
    }                                                                                            \
    if (l.name17 != r.name17) {                                                                  \
      return l.name17 < r.name17;                                                                \
    }                                                                                            \
    if (l.name18 != r.name18) {                                                                  \
      return l.name18 < r.name18;                                                                \
    }                                                                                            \
    return false;                                                                                \
  }                                                                                              \
  friend bool operator==(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name2 != r.name2) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name3 != r.name3) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name4 != r.name4) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name5 != r.name5) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name6 != r.name6) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name7 != r.name7) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name8 != r.name8) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name9 != r.name9) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name10 != r.name10) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name11 != r.name11) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name12 != r.name12) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name13 != r.name13) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name14 != r.name14) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name15 != r.name15) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name16 != r.name16) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name17 != r.name17) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name18 != r.name18) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
  }                                                                                              \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_18(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18)         \
  DECLARE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18)         \
  DECLARE_STRUCT_END

#define DECLARE_STRUCT_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4,    \
                                type5, name5, type6, name6, type7, name7, type8, name8, type9,   \
                                name9, type10, name10, type11, name11, type12, name12, type13,   \
                                name13, type14, name14, type15, name15, type16, name16, type17,  \
                                name17, type18, name18, type19, name19)                          \
  DECLARE_SIMPLE_STRUCT_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9, type10, name10, type11, name11, type12, name12, type13,  \
                                 name13, type14, name14, type15, name15, type16, name16, type17, \
                                 name17, type18, name18, type19, name19)                         \
  friend bool operator<(const name& l, const name& r) {                                          \
    if (l.name1 != r.name1) {                                                                    \
      return l.name1 < r.name1;                                                                  \
    }                                                                                            \
    if (l.name2 != r.name2) {                                                                    \
      return l.name2 < r.name2;                                                                  \
    }                                                                                            \
    if (l.name3 != r.name3) {                                                                    \
      return l.name3 < r.name3;                                                                  \
    }                                                                                            \
    if (l.name4 != r.name4) {                                                                    \
      return l.name4 < r.name4;                                                                  \
    }                                                                                            \
    if (l.name5 != r.name5) {                                                                    \
      return l.name5 < r.name5;                                                                  \
    }                                                                                            \
    if (l.name6 != r.name6) {                                                                    \
      return l.name6 < r.name6;                                                                  \
    }                                                                                            \
    if (l.name7 != r.name7) {                                                                    \
      return l.name7 < r.name7;                                                                  \
    }                                                                                            \
    if (l.name8 != r.name8) {                                                                    \
      return l.name8 < r.name8;                                                                  \
    }                                                                                            \
    if (l.name9 != r.name9) {                                                                    \
      return l.name9 < r.name9;                                                                  \
    }                                                                                            \
    return false;                                                                                \
    if (l.name10 != r.name10) {                                                                  \
      return l.name10 < r.name10;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name11 != r.name11) {                                                                  \
      return l.name11 < r.name11;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name12 != r.name12) {                                                                  \
      return l.name12 < r.name12;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name13 != r.name13) {                                                                  \
      return l.name13 < r.name13;                                                                \
    }                                                                                            \
    return false;                                                                                \
    if (l.name14 != r.name14) {                                                                  \
      return l.name14 < r.name14;                                                                \
    }                                                                                            \
    if (l.name15 != r.name15) {                                                                  \
      return l.name15 < r.name15;                                                                \
    }                                                                                            \
    if (l.name16 != r.name16) {                                                                  \
      return l.name16 < r.name16;                                                                \
    }                                                                                            \
    if (l.name17 != r.name17) {                                                                  \
      return l.name17 < r.name17;                                                                \
    }                                                                                            \
    if (l.name18 != r.name18) {                                                                  \
      return l.name18 < r.name18;                                                                \
    }                                                                                            \
    if (l.name19 != r.name19) {                                                                  \
      return l.name19 < r.name19;                                                                \
    }                                                                                            \
    return false;                                                                                \
  }                                                                                              \
  friend bool operator==(const name& l, const name& r) {                                         \
    if (l.name1 != r.name1) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name2 != r.name2) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name3 != r.name3) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name4 != r.name4) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name5 != r.name5) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name6 != r.name6) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name7 != r.name7) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name8 != r.name8) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    if (l.name9 != r.name9) {                                                                    \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name10 != r.name10) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name11 != r.name11) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name12 != r.name12) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name13 != r.name13) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
    if (l.name14 != r.name14) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name15 != r.name15) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name16 != r.name16) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name17 != r.name17) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name18 != r.name18) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    if (l.name19 != r.name19) {                                                                  \
      return false;                                                                              \
    }                                                                                            \
    return true;                                                                                 \
  }                                                                                              \
  friend bool operator!=(const name& l, const name& r) { return !(l == r); }

#define DECLARE_STRUCT_19(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18, type19, \
                          name19)                                                                 \
  DECLARE_STRUCT_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18, type19, \
                          name19)                                                                 \
  DECLARE_STRUCT_END
}  // namespace commons
}  // namespace agora
