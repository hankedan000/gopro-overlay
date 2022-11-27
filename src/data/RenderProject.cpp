#include "GoProOverlay/data/RenderProject.h"

#include <filesystem>
#include <fstream>

const std::string PROJECT_FILENAME = "project.yaml";
const std::string TRACK_FILENAME = "track.yaml";

namespace gpo
{
	RenderProject::RenderProject()
	 : dsm_()
	 , track_(nullptr)
	{
	}

	RenderProject::~RenderProject()
	{
		if (track_)
		{
			delete track_;
			track_ = nullptr;
		}
	}

	DataSourceManager &
	RenderProject::dataSourceManager()
	{
		return dsm_;
	}

	void
	RenderProject::setTrack(
		Track *track)
	{
		if (track_)
		{
			delete track_;
		}

		track_ = track;
		for (size_t i=0; i<dsm_.sourceCount(); i++)
		{
			dsm_.getSource(i)->setDatumTrack(track);
		}
	}

	Track *
	RenderProject::getTrack()
	{
		return track_;
	}

	bool
	RenderProject::hasTrack() const
	{
		return track_ != nullptr;
	}

	bool
	RenderProject::isValidProject(
		const std::string &dir)
	{
		const std::filesystem::path path(dir);
		if ( ! std::filesystem::exists(path))
		{
			return false;
		}
		else if (std::filesystem::is_directory(path))
		{
			const std::filesystem::path projPath = path / PROJECT_FILENAME;
			return std::filesystem::exists(projPath);
		}
		return false;
	}

	bool
	RenderProject::save(
		const std::string &dirPath)
	{
		const std::filesystem::path projectRoot(dirPath);
		if ( ! std::filesystem::exists(projectRoot))
		{
			// make project directory if it doesn't exist already
			if ( ! std::filesystem::create_directories(projectRoot))
			{
				return false;
			}
		}
		else if ( ! std::filesystem::is_directory(projectRoot))
		{
			// path exists, but it's not a directory, so bail out
			return false;
		}

		const std::filesystem::path projectPath = projectRoot / PROJECT_FILENAME;
		const std::filesystem::path trackPath = projectRoot / TRACK_FILENAME;

		YAML::Node yProject = encode();
		std::ofstream projOFS(projectPath);
		projOFS << yProject;
		projOFS.close();

		if (track_)
		{
			YAML::Node trackNode = track_->encode();
			std::ofstream trackOFS(trackPath);
			trackOFS << trackNode;
			trackOFS.close();
		}

		return true;
	}

	bool
	RenderProject::load(
		const std::string &dirPath)
	{
		if ( ! isValidProject(dirPath))
		{
			return false;
		}

		const std::filesystem::path projectRoot(dirPath);
		const std::filesystem::path projectPath = projectRoot / PROJECT_FILENAME;
		const std::filesystem::path trackPath = projectRoot / TRACK_FILENAME;

		bool okay = true;

		YAML::Node projectNode = YAML::LoadFile(projectPath);
		okay = okay && decode(projectNode);

		if (std::filesystem::exists(trackPath))
		{
			YAML::Node trackNode = YAML::LoadFile(trackPath);
			if ( ! trackNode.IsNull())
			{
				gpo::Track *newTrack = new gpo::Track();
				if (newTrack->decode(trackNode))
				{
					setTrack(newTrack);
				}
				else
				{
					delete newTrack;
					okay = false;
				}
			}
			else
			{
				okay = false;
			}
		}

		return okay;
	}

	YAML::Node
	RenderProject::encode() const
	{
		YAML::Node node;

		node["dsm"] = dsm_.encode();

		return node;
	}

	bool
	RenderProject::decode(
		const YAML::Node& node)
	{
		bool okay = true;

		okay = okay && dsm_.decode(node["dsm"]);

		return okay;
	}
}