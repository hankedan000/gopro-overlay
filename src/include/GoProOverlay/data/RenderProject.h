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
	clear();

	void
	setTrack(
		Track *track);

	Track *
	getTrack();

	bool
	hasTrack() const;

	void
	setLeadInSeconds(
		double dur_secs);

	double
	getLeadInSeconds() const;

	void
	setLeadOutSeconds(
		double dur_secs);

	double
	getLeadOutSeconds() const;

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

	// amount of time in seconds to start render before the alignment point
	double renderLeadIn_sec_;

	// amount of time in seconds to stop rendering after all sources crossed finish element
	double renderLeadOut_sec_;

};

}