#include "GoProOverlay/data/RenderProject.h"

#include "GoProOverlay/graphics/RenderEngine.h"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

const std::string PROJECT_FILENAME = "project.yaml";
const std::string TRACK_FILENAME = "track.yaml";

namespace gpo
{
	RenderProject::RenderProject()
	 : ModifiableObject("RenderProject",true,true)
	 , dsm_()
	 , engine_(std::make_shared<RenderEngine>())
	 , track_(nullptr)
	 , renderLeadIn_sec_(0.0)
	 , renderLeadOut_sec_(0.0)
	 , lastNonCustomAlignmentInfo_()
	 , currRenderAlignmentInfo_()
	 , audioExportApproach_(AudioExportApproach_E::eAEA_SingleSource)
	 , exportFilePath_("")
	{
		engine_->addObserver((ModifiableObjectObserver*)this);
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

	const DataSourceManager &
	RenderProject::dataSourceManager() const
	{
		return dsm_;
	}

	void
	RenderProject::clear()
	{
		dsm_.clear();
		engine_->clear();
		setTrack(nullptr);
		setSavePath("");
		clearNeedsApply();
		clearNeedsSave();
	}

	void
	RenderProject::setTrack(
		Track *track)
	{
		internalSetTrack(track);
		markObjectModified();
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

	void
	RenderProject::setLeadInSeconds(
		double dur_secs)
	{
		if (renderLeadIn_sec_ != dur_secs)
		{
			renderLeadIn_sec_ = dur_secs;
			markObjectModified();
		}
	}

	double
	RenderProject::getLeadInSeconds() const
	{
		return renderLeadIn_sec_;
	}

	void
	RenderProject::setLeadOutSeconds(
		double dur_secs)
	{
		if (renderLeadOut_sec_ != dur_secs)
		{
			renderLeadOut_sec_ = dur_secs;
			markObjectModified();
		}
	}

	double
	RenderProject::getLeadOutSeconds() const
	{
		return renderLeadOut_sec_;
	}

	void
	RenderProject::setAlignmentInfo(
		const RenderAlignmentInfo &renderAlignmentInfo)
	{
		currRenderAlignmentInfo_ = renderAlignmentInfo;
		if (renderAlignmentInfo.type != RenderAlignmentType_E::eRAT_Custom)
		{
			lastNonCustomAlignmentInfo_ = renderAlignmentInfo;
		}
		markObjectModified();
	}

	const RenderAlignmentInfo &
	RenderProject::getAlignmentInfo() const
	{
		return currRenderAlignmentInfo_;
	}

	void
	RenderProject::setAudioExportApproach(
		const AudioExportApproach_E &approach)
	{
		if (audioExportApproach_ != approach)
		{
			audioExportApproach_ = approach;
			markObjectModified();
		}
	}

	const AudioExportApproach_E &
	RenderProject::getAudioExportApproach() const
	{
		return audioExportApproach_;
	}

	void
	RenderProject::setExportFilePath(
		const std::filesystem::path &path)
	{
		exportFilePath_ = path;
		markObjectModified();
	}

	const std::filesystem::path &
	RenderProject::getExportFilePath() const
	{
		return exportFilePath_;
	}

	void
	RenderProject::reprocessDatumTrack()
	{
		#pragma omp parallel for
		for (size_t i=0; i<dsm_.sourceCount(); i++)
		{
			dsm_.getSource(i)->reprocessDatumTrack();
		}
	}

	void
	RenderProject::setEngine(
		RenderEnginePtr engine)
	{
		if (engine != nullptr)
		{
			if (engine_)
			{
				engine_->removeObserver((ModifiableObjectObserver*)this);
			}
			engine_ = engine;
			engine_->addObserver((ModifiableObjectObserver*)this);
			markObjectModified();
		}
	}

	RenderEnginePtr
	RenderProject::getEngine()
	{
		return engine_;
	}

	bool
	RenderProject::isValidProject(
		const std::string &dir,
		bool noisy)
	{
		const std::filesystem::path projectDir(dir);
		if ( ! std::filesystem::exists(projectDir))
		{
			if (noisy)
			{
				spdlog::warn("directory doesn't exists. '{}'",projectDir.c_str());
			}
			return false;
		}
		else if (std::filesystem::is_directory(projectDir))
		{
			const std::filesystem::path projectFilePath = projectDir / PROJECT_FILENAME;
			bool projectFileExists = std::filesystem::exists(projectFilePath);
			if ( ! projectFileExists && noisy)
			{
				spdlog::warn("directory '{}' doesn't contain a '{}' file.",projectDir.c_str(),PROJECT_FILENAME);
			}
			return projectFileExists;
		}
		return false;
	}

	bool
	RenderProject::load(
		const std::string &dirPath)
	{
		if ( ! isValidProject(dirPath,true))// true -> noisy
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
			bool trackOkay = true;
			YAML::Node trackNode = YAML::LoadFile(trackPath);
			if ( ! trackNode.IsNull())
			{
				gpo::Track *newTrack = new gpo::Track();
				if (newTrack->decode(trackNode))
				{
					newTrack->setSavePath(trackPath);
					// call save to clear modification flags from loading
					// kind of a hack...
					newTrack->saveModifications(true);
					internalSetTrack(newTrack);
				}
				else
				{
					delete newTrack;
					trackOkay = false;
				}
			}
			else
			{
				trackOkay = false;
			}

			if ( ! trackOkay)
			{
				spdlog::error("failed to decode track data from '{}'",trackPath.c_str());
				okay = false;
			}
		}

		engine_->removeObserver(this);// ignore change events while restoring alignment position
		engine_->getSeeker()->seekToAlignmentInfo(currRenderAlignmentInfo_);
		engine_->getSeeker()->setAlignmentHere();
		engine_->addObserver(this);

		// call save to clear modification flags from loading
		// kind of a hack...
		engine_->saveModifications(true);

		setSavePath(projectRoot);
		return okay;
	}

	YAML::Node
	RenderProject::encode() const
	{
		YAML::Node node;

		node["dsm"] = dsm_.encode();
		node["engine"] = engine_->encode();

		node["renderLeadIn_sec"] = renderLeadIn_sec_;
		node["renderLeadOut_sec"] = renderLeadOut_sec_;

		node["lastNonCustomAlignmentInfo"] = lastNonCustomAlignmentInfo_;
		node["currRenderAlignmentInfo"] = currRenderAlignmentInfo_;

		node["audioExportApproach"] = (int)audioExportApproach_;

		node["exportFilePath"] = exportFilePath_.c_str();

		return node;
	}

	bool
	RenderProject::decode(
		const YAML::Node& node)
	{
		bool okay = true;

		okay = okay && dsm_.decode(node["dsm"]);
		okay = okay && engine_->decode(node["engine"],dsm_);

		YAML_TO_FIELD_W_DEFAULT(node,"renderLeadIn_sec",renderLeadIn_sec_,0.0);
		YAML_TO_FIELD_W_DEFAULT(node,"renderLeadOut_sec",renderLeadOut_sec_,0.0);

		auto defaultAlignInfo = RenderAlignmentInfo();
		YAML_TO_FIELD_W_DEFAULT(node,"lastNonCustomAlignmentInfo",lastNonCustomAlignmentInfo_,defaultAlignInfo);
		YAML_TO_FIELD_W_DEFAULT(node,"currRenderAlignmentInfo",currRenderAlignmentInfo_,defaultAlignInfo);

		audioExportApproach_ = AudioExportApproach_E::eAEA_SingleSource;
		if (node["audioExportApproach"])
		{
			int approachInt;
			YAML_TO_FIELD(node,"audioExportApproach",approachInt);
			audioExportApproach_ = (AudioExportApproach_E)approachInt;
		}

		std::string strExportFilePath;
		YAML_TO_FIELD_W_DEFAULT(node,"exportFilePath",strExportFilePath,"");
		exportFilePath_ = strExportFilePath;

		return okay;
	}

	bool
	RenderProject::subclassApplyModifications(
        bool unnecessaryIsOkay)
	{
		return false;
	}

	bool
	RenderProject::subclassSaveModifications(
        bool unnecessaryIsOkay)
	{
		const std::filesystem::path &projectRoot = getSavePath();
		if ( ! std::filesystem::exists(projectRoot))
		{
			// make project directory if it doesn't exist already
			if ( ! std::filesystem::create_directories(projectRoot))
			{
				spdlog::error("failed to create project root directory '{}'",projectRoot.c_str());
				return false;
			}
		}
		else if ( ! std::filesystem::is_directory(projectRoot))
		{
			// path exists, but it's not a directory, so bail out
			spdlog::error("projectRoot must be a directory. '{}'",projectRoot.c_str());
			return false;
		}

		const std::filesystem::path projectPath = projectRoot / PROJECT_FILENAME;

		YAML::Node yProject = encode();
		std::ofstream projOFS(projectPath);
		projOFS << yProject;
		projOFS.close();

		if (track_)
		{
			YAML::Node trackNode = track_->encode();
			std::filesystem::path trackPath = projectRoot / TRACK_FILENAME;
			std::ofstream trackOFS(trackPath);
			trackOFS << trackNode;
			trackOFS.close();
			track_->setSavePath(trackPath);
			track_->saveModifications(unnecessaryIsOkay);
		}

		engine_->saveModifications(unnecessaryIsOkay);

		return true;
	}

	void
	RenderProject::internalSetTrack(
		Track *track)
	{
		if (track_)
		{
			track_->removeObserver(this);
			delete track_;
		}
		track_ = track;
		if (track_)
		{
			track_->addObserver(this);
		}

		#pragma omp parallel for
		for (size_t i=0; i<dsm_.sourceCount(); i++)
		{
			dsm_.getSource(i)->setDatumTrack(track,true);// true - process immediately
		}

		for (size_t e=0; e<engine_->entityCount(); e++)
		{
			const auto &entity = engine_->getEntity(e);
			DataSourceRequirements dsr = entity->renderObject()->dataSourceRequirements();
			if (dsr.numTracks == DSR_ONE_OR_MORE || dsr.numTracks > 0)
			{
				entity->renderObject()->setTrack(track_);
			}
		}
	}

	void
	RenderProject::onModified(
		ModifiableObject *modifiable)
	{
		if (modifiable == engine_.get())
		{
			markObjectModified(
				modifiable->hasApplyableModifications(),
				modifiable->hasSavableModifications());
		}
		else if (modifiable == track_)
		{
			markObjectModified(
				modifiable->hasApplyableModifications(),
				modifiable->hasSavableModifications());
		}
	}
}