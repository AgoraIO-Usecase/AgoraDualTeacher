namespace cpp agora.rtc.sei

struct Canvas {
  1: i32 width,
  2: i32 height,
  3: optional i32 bgColor
}
struct Region {
  1: i32 uid,
  2: i32 x,
  3: i32 y,
  4: i32 width,
  5: i32 height,
  6: i32 renderMode,
  7: optional i32 zOrder,
  8: optional i32 alpha
}

struct VideoCompositingLayout {
  1: optional Canvas canvas,
  2: list<Region> regions,
  3: i64 ts,
  4: optional binary appData
}
