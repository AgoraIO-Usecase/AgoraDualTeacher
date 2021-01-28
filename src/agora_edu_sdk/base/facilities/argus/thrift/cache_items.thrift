namespace cpp agora.cache

struct DnsItem {
  1: i64 expired,
  2: list<i32> ipList
}
struct DnsItem2 {
  1: i64 expired,
  2: list<string> ipList
}

struct Policy {
  1: i64 expired,
  2: optional string params
}

struct CacheDocument {
  //bssid or ip ==> dns name ==> ip list
  1: optional map<string, map<string, DnsItem> > dnsList,
  2: optional string lastSid,
  3: optional string failedSid,
  4: optional Policy policy,
  5: optional map<string, map<string, DnsItem2> > dnsList2,
  6: optional string installId,
  7: optional i32 netEngine,
  8: optional string agoraUniqueId
}

struct PolicyDocument {
  1: i32 version,
  2: optional string params
}

struct ReportCacheItem {
  1: i64 sent_ts,
  3: string payload,
  4: i32 level,
  5: optional i32 vid,
  6: optional i32 cid,
  7: i32 type,
}

struct ReportCacheDocument {
  // hash ==> ReportCacheItem
  1: optional map<i64, ReportCacheItem> reportCacheList
}
