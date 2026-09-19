#include <android/log.h>
#include <sys/system_properties.h>
#include <map>
#include <sstream>
#include <vector>
#include <dlfcn.h>
#include <unistd.h>

#include "zygisk.hpp"
#include "shadowhook.h"

using namespace std::ranges;
using std::string;
using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define CONFIG_FILE_PATH "/data/adb/modules/gpay_country_spoofer/config.conf"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "GpayCountrySpoofer", __VA_ARGS__)
#define MAIN_PROCESS "com.google.android.gms"

typedef void (*T_Callback)(void *, const char *, const char *, uint32_t);

static std::map<void *, T_Callback> callbacks;

static std::map<string, string> configuration;

static void modify_callback(void *cookie, const char *name, const char *value, uint32_t serial) {
    if (cookie == nullptr || name == nullptr || value == nullptr || !callbacks.contains(cookie)) return;

    const char *oldValue = value;
    const int verboseLogs = std::stoi(configuration.at("VERBOSE_LOGS"));
    std::string prop(name);

    if (prop == "gsm.sim.operator.numeric" || prop == "gsm.operator.numeric") {
        string mcc = configuration.at("GMS_MCC");
        string mnc = configuration.at("GMS_MNC");
        value = (mcc += mnc).c_str();
    } else if (prop == "gsm.sim.operator.iso-country" || prop == "gsm.operator.iso-country") {
        value = configuration.at("GMS_ISO").c_str();
    } else if (prop == "gsm.sim.operator.alpha" || prop == "gsm.operator.alpha") {
        value = configuration.at("GMS_OPERATOR_NAME").c_str();
    }

    if (oldValue == value) {
        if (verboseLogs == 1) LOGD("[%s]: %s (unchanged)", name, oldValue);
    } else {
        LOGD("[%s]: %s -> %s", name, oldValue, value);
    }
    return callbacks[cookie](cookie, name, value, serial);
}

static void (*o_system_property_read_callback)(const prop_info *, T_Callback, void *);

static void my_system_property_read_callback(const prop_info *pi, T_Callback callback, void *cookie) {
    if (pi == nullptr || callback == nullptr || cookie == nullptr) {
        return o_system_property_read_callback(pi, callback, cookie);
    }
    callbacks[cookie] = callback;
    return o_system_property_read_callback(pi, modify_callback, cookie);
}

extern "C" [[maybe_unused]] int sh_linker_init(void) {
    return 0;
}

static void doHook() {
    shadowhook_init(SHADOWHOOK_MODE_UNIQUE, false);

    void *sym_addr = dlsym(RTLD_DEFAULT, "__system_property_read_callback");
    if (sym_addr == nullptr) {
        LOGD("Couldn't find '__system_property_read_callback' via dlsym");
        return;
    }

    void *handle = shadowhook_hook_sym_addr(
            sym_addr,
            reinterpret_cast<void *>(my_system_property_read_callback),
            reinterpret_cast<void **>(&o_system_property_read_callback)
    );

    if (handle == nullptr) {
        LOGD("Couldn't hook '__system_property_read_callback'. Error: %d", shadowhook_get_errno());
        return;
    }
    LOGD("Found '__system_property_read_callback' handle at %p", handle);
}

constexpr std::string_view trim(std::string_view line) {
    while (std::isspace(static_cast<unsigned char>(line.front()))) {
        line.remove_prefix(1);
    }
    while (std::isspace(static_cast<unsigned char>(line.back()))) {
        line.remove_suffix(1);
    }
    return line;
}

std::map<string, string> parse_config(const std::string& config_str) {
    std::map<string, string> config;
    std::stringstream file(config_str);

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.starts_with('#')) {
            continue;
        }
        std::string_view prop = trim(line);
        if (auto pos = prop.find('='); pos != std::string_view::npos) {
            auto key = trim(prop.substr(0, pos));
            auto value = trim(prop.substr(pos + 1));

            if (!key.empty()) {
                config.emplace(key, value);
            }
        }
    }
    return config;
}

class GpayCountrySpoofer : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->zygiskApi = api;
        this->jniEnv = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        bool isMainProcess = false;

        auto rawProcess = jniEnv->GetStringUTFChars(args->nice_name, nullptr);
        auto rawDir = jniEnv->GetStringUTFChars(args->app_data_dir, nullptr);

        // Prevent crash on apps with no data dir
        if (rawDir == nullptr) {
            jniEnv->ReleaseStringUTFChars(args->nice_name, rawProcess);
            zygiskApi->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        pkgName = rawProcess;
        std::string_view dir(rawDir);

        isGms = dir.ends_with("/com.google.android.gms");
        isMainProcess = pkgName == MAIN_PROCESS;

        jniEnv->ReleaseStringUTFChars(args->nice_name, rawProcess);
        jniEnv->ReleaseStringUTFChars(args->app_data_dir, rawDir);

        // Applying hooks only to the main process Google Play Services
        if (!isGms || !isMainProcess) {
            zygiskApi->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        std::vector<char> configVector;
        long configSize = 0;
        int fd = zygiskApi->connectCompanion();

        read(fd, &configSize, sizeof(long));
        if (configSize < 1) {
            close(fd);
            LOGD("Failed to load the spoofing configuration");
            zygiskApi->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        configVector.resize(configSize);
        read(fd, configVector.data(), configSize);
        close(fd);

        std::string configString(configVector.cbegin(), configVector.cend());
        configuration = parse_config(configString);

        configVector.clear();
        configString.clear();
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (isGms && pkgName == MAIN_PROCESS && !configuration.empty()) {
            LOGD("Processing: %s", pkgName.c_str());
            doHook();
        }
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override {
        zygiskApi->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api *zygiskApi = nullptr;
    JNIEnv *jniEnv = nullptr;
    bool isGms = false;
    std::string pkgName;
};

static void companion(int fd) {
    long configSize = 0;
    std::vector<char> configVector;

    FILE *config = fopen(CONFIG_FILE_PATH, "r");
    if (config) {
        fseek(config, 0, SEEK_END);
        configSize = ftell(config);
        fseek(config, 0, SEEK_SET);

        configVector.resize(configSize);
        fread(configVector.data(), 1, configSize, config);

        fclose(config);
    }

    write(fd, &configSize, sizeof(long));
    write(fd, configVector.data(), configSize);
    configVector.clear();
}

REGISTER_ZYGISK_MODULE(GpayCountrySpoofer)
REGISTER_ZYGISK_COMPANION(companion)