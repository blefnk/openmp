#include "checkpoint.hpp"
#include <sdk.hpp>

struct CheckpointsPlugin final : public ICheckpointsPlugin, public PlayerEventHandler {
	DefaultEventDispatcher<PlayerCheckpointEventHandler> checkpointDispatcher;
	ICore* core;

	UUID getUUID() override {
		return 0x44a937350d611dde;
	}

	DefaultEventDispatcher<PlayerCheckpointEventHandler>& getCheckpointDispatcher() override {
		return checkpointDispatcher;
	}

	// Set up dummy checkpoint data for when the player connects
	PlayerCheckpointData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerCheckpointData(player);
	}

	struct PlayerEnterCheckpointHandler : public PlayerUpdateEventHandler {
		CheckpointsPlugin& self;
		PlayerEnterCheckpointHandler(CheckpointsPlugin& self) : self(self) {}

		bool onUpdate(IPlayer& player) override {
			PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
			if (cp->inside_)
				return true;

			const float radius = cp->size_ / 2;
			const float maxDistance = radius * radius;
			const Vector3 distance = cp->position_ - player.getPosition();
			if (glm::dot(distance, distance) > maxDistance)
				return true;

			cp->inside_ = true;
			self.checkpointDispatcher.dispatch(
				&PlayerCheckpointEventHandler::onPlayerEnterCheckpoint,
				player
			);
			return true; 
		}
	} playerEnterCheckpointHandler;

	struct PlayerLeaveCheckpointHandler : public PlayerUpdateEventHandler {
		CheckpointsPlugin& self;
		PlayerLeaveCheckpointHandler(CheckpointsPlugin& self) : self(self) {}

		bool onUpdate(IPlayer& player) override {
			PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
			if (!cp->inside_)
				return true;

			const float radius = cp->size_ / 2;
			const float maxDistance = radius * radius;
			const Vector3 distance = cp->position_ - player.getPosition();
			if (glm::dot(distance, distance) <= maxDistance)
				return true;

			cp->inside_ = false;
			self.checkpointDispatcher.dispatch(
				&PlayerCheckpointEventHandler::onPlayerLeaveCheckpoint,
				player
			);

			return true;
		}
	} playerLeaveCheckpointHandler;

	CheckpointsPlugin() :
		playerEnterCheckpointHandler(*this),
		playerLeaveCheckpointHandler(*this)
	{
	}

	void onInit(ICore* c) override {
		core = c;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerEnterCheckpointHandler);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerLeaveCheckpointHandler);
	}

	const char* pluginName() override {
		return "CheckpointPlugin";
	}

	void free() override {
		delete this;
	}

	~CheckpointsPlugin() {
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerEnterCheckpointHandler);
		core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerLeaveCheckpointHandler);
	}

	void setPlayerCheckpoint(const IPlayer& player, const Vector3 position, const float size) override {
		PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
		cp->position_ = position;
		cp->size_ = size;
		cp->enable();
	}

	void disablePlayerCheckpoint(const IPlayer& player) override {
		PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
		cp->disable();
	}
};

PLUGIN_ENTRY_POINT() {
	return new CheckpointsPlugin();
}