// BluetoothIndicator.hpp - Status bar Bluetooth icon + QS toggle tile.
//
// Pure consumer of BluetoothBackend snapshots; the Quick Settings toggle
// flips the adapter through the backend (async Powered write).

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/BluetoothBackend.hpp"
#include <string>

namespace qypr {

class BluetoothIndicator : public StatusIndicator {
public:
    explicit BluetoothIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;
    Color iconColor() const override;

    void onBackendUpdate() override;

    std::unique_ptr<QSTile> createTile() override;

private:
    BluetoothBackend* backend_ = nullptr;
    BluetoothSnapshot lastSnap_;
};

}  // namespace qypr
