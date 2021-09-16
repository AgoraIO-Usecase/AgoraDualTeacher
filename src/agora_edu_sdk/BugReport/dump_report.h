#ifndef __DUMP_REPORT_H__
#define __DUMP_REPORT_H__

#if defined(__GNUC__)
#define EXPORT_API __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif

struct CrashEventPacket {
  long long lts = 0;
  char* sid = 0;
  char* cname = 0;
  long long cid = 0;
  long long uid = 0;
  long long elapse = 0;
  long crashVer = 0;
  long dmpType = 0;
  long long lstLts = 0;
  long long lstCrashAddr = 0;
  long long lstLdBegin = 0;
  long long lstLdEnd = 0;
  char* lstServiceId = 0;
  char* lstSessionId = 0;
  char* lstChannelName = 0;
  char* lstSdkVer = 0;
  long lstNetwork = 0;
  long lstChannelMode = 0;
  long lstChannelProfile = 0;
  long lstClientType = 0;
  long lstClientRole = 0;
  char* lstCrashUid = 0;
  long lstBuildNo = 0;
  bool isDumpFile = 0;
  long os = 0;
  char* deviceid = 0;
  char* installid = 0;
  char* appid = 0;
  long cpuarch = 0;
};

#define GEN_SET(type, property_name) \
  virtual void Set##property_name(const type property_name) = 0

#define GEN_GET(type, property_name) virtual type Get##property_name()const = 0

#define GEN_GET_SET(type, property_name) \
  GEN_GET(type, property_name);          \
  GEN_SET(type, property_name)

class IDumpReportService {
 public:
  virtual void Initialize(const char* log_dir, bool enable_report_argus = true,
                          bool enable_upload_dump = true) = 0;
  virtual void Destroy() = 0;
  virtual void SetCrashEventPacket(CrashEventPacket* pkt) = 0;
  virtual const CrashEventPacket* GetCrashEventPacket() = 0;
  GEN_GET_SET(long long, Lts);
  GEN_GET_SET(char*, Sid);
  GEN_GET_SET(char*, Cname);
  GEN_GET_SET(long long, Cid);
  GEN_GET_SET(long long, Uid);
  GEN_GET_SET(long long, Elapse);
  GEN_GET_SET(long, CrashVer);
  GEN_GET_SET(long, DmpType);
  GEN_GET_SET(long long, LstLts);
  GEN_GET_SET(long long, LstCrashAddr);
  GEN_GET_SET(long long, LstLdBegin);
  GEN_GET_SET(long long, LstLdEnd);
  GEN_GET_SET(char*, LstServiceId);
  GEN_GET_SET(char*, LstSessionId);
  GEN_GET_SET(char*, LstChannelName);
  GEN_GET_SET(char*, LstSdkVer);
  GEN_GET_SET(long, LstNetwork);
  GEN_GET_SET(long, LstChannelMode);
  GEN_GET_SET(long, LstChannelProfile);
  GEN_GET_SET(long, LstClientType);
  GEN_GET_SET(long, LstClientRole);
  GEN_GET_SET(char*, LstCrashUid);
  GEN_GET_SET(long, LstBuildNo);
  GEN_GET_SET(bool, IsDumpFile);
  GEN_GET_SET(long, Os);
  GEN_GET_SET(char*, Deviceid);
  GEN_GET_SET(char*, Installid);
  GEN_GET_SET(char*, Appid);
  GEN_GET_SET(long, Cpuarch);
  GEN_GET_SET(char*, RequestId);
  GEN_GET_SET(long, Vid);
};

#undef GEN_SET
#undef GEN_GET
#undef GEN_GET_SET

#ifdef __cplusplus
extern "C" {
#endif
EXPORT_API IDumpReportService* GetDumpReportService();

#ifdef __cplusplus
}
#endif

#endif