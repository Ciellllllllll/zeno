#include <zeno/zeno_native_backend.h>

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    config.flags = 0;
    config.reserved = 0;

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 1;
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_OK ? 0 : 2;
}
