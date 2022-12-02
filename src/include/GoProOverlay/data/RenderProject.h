#pragma once

#include <GoProOverlay/data/DataSource.h>
#include <GoProOverlay/data/TrackDataObjects.h>

namespace gpo
{

// forward declare
class RenderEngine;
using RenderEnginePtr = std::shared_ptr<RenderEngine>;

class RenderProject
{
public:
	RenderProject();

	~RenderProject();

	DataSourceManager &
	dataSourceManager();

	const DataSourceManager &
	dataSourceManager() const;

	void
	setTrack(
		Track *track);

	Track *
	getTrack();

	bool
	hasTrack() const;

	void
	reprocessDatumTrack();

	void
	setEngine(
		RenderEnginePtr engine);

	RenderEnginePtr
	getEngine();

	static
	bool
	isValidProject(
		const std::string &dir);

	bool
	save(
		const std::string &dirPath);

	bool
	load(
		const std::string &dirPath);

	YAML::Node
	encode() const;

	bool
	decode(
		const YAML::Node& node);

private:
	DataSourceManager dsm_;
	RenderEnginePtr engine_;
	Track *track_;

};

}