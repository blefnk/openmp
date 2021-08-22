#include <sdk.hpp>
#include "legacy_network_impl.hpp"

struct RakNetLegacyNetworkComponent : INetworkComponent {
	RakNetLegacyNetwork legacyNetwork;

	void onLoad(ICore* core) override {
		legacyNetwork.init(core);
	}

	INetwork* getNetwork() override {
		return &legacyNetwork;
	}

	StringView componentName() const override {
		return "RakNetLegacyNetwork";
	}

	SemanticVersion componentVersion() const override {
		return SemanticVersion(0, 0, 0, BUILD_NUMBER);
	}

	UUID getUUID() override {
		return 0xea9799fd79cf8442;
	}
};

COMPONENT_ENTRY_POINT() {
	return new RakNetLegacyNetworkComponent();
}