#ifndef BACKENDS_JACK_H
#define BACKENDS_JACK_H

#include "backends/base.h"

struct JackBackendFactory final : public BackendFactory {
public:
    bool init() override;

    bool querySupport(BackendType type) override;

    std::string probe(BackendType type) override;

    BackendPtr createBackend(ALCdevice *device, BackendType type) override;

    static BackendFactory &getFactory();
};

#endif /* BACKENDS_JACK_H */
