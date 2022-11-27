#pragma once

#include <GoProOverlay/data/DataSource.h>
#include <GoProOverlay/data/TrackDataObjects.h>

namespace gpo
{

class RenderProject
{
public:
	RenderProject();

	~RenderProject();

	DataSourceManager &
	dataSourceManager();

	void
	setTrack(
		Track *track);

	Track *
	getTrack();

	bool
	hasTrack() const;

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
	Track *track_;

};

}