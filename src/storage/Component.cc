// See the file "COPYING" in the main distribution directory for copyright.

#include "zeek/storage/Component.h"

#include "zeek/Desc.h"
#include "zeek/storage/Manager.h"

namespace zeek::storage {

Component::Component(const std::string& name, factory_callback arg_factory)
    : plugin::Component(plugin::component::STORAGE_BACKEND, name, 0, storage_mgr->GetTagType()) {
    factory = arg_factory;
}

void Component::Initialize() {
    InitializeTag();
    storage_mgr->RegisterComponent(this);
}

void Component::DoDescribe(ODesc* d) const {
    d->Add("Storage::STORAGE_BACKEND_");
    d->Add(CanonicalName());
}

} // namespace zeek::storage
