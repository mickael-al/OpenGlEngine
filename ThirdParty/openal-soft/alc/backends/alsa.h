#ifndef BACKENDS_ALSA_H
#define BACKENDS_ALSA_H

#include "backends/base.h"

struct AlsaBackendFactory final : public BackendFactory {
public:
    bool init() override;

    bool querySupport(BackendType type) override;

    std::string probe(BackendType type) override;

    BackendPtr createBackend(ALCdevice *device, BackendType type) override;

    static BackendFactory &getFactory();
};

#endif /* BACKENDS_ALSA_H */
