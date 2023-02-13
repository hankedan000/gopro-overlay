#pragma once

#include <GoProOverlay/data/DataSource.h>
#include <GoProOverlay/data/TrackDataObjects.h>
#include <yaml-cpp/yaml.h>

namespace gpo
{

// forward declare
class RenderEngine;
using RenderEnginePtr = std::shared_ptr<RenderEngine>;

enum ElementSide_E
{
	eES_Entry = 0,
	eES_Exit = 1
};

struct LapAlignment
{
	unsigned int lap;
	ElementSide_E side;
};

struct SectorAlignment
{
	unsigned int lap;
	unsigned int sector;
	ElementSide_E side;
};

struct CustomAlignment
{
	// map of custom alignment indices keyed by data source names
	std::unordered_map<std::string,size_t> idxBySourceName;
};

enum RenderAlignmentType_E
{
	eRAT_None = 0,
	eRAT_Lap = 1,
	eRAT_Sector = 2,
	eRAT_Custom = 3
};

class RenderAlignmentInfo
{
public:
	void
	release()
	{
		switch (type)
		{
			case RenderAlignmentType_E::eRAT_None:
				break;
			case RenderAlignmentType_E::eRAT_Lap:
				delete alignInfo.lap;
				break;
			case RenderAlignmentType_E::eRAT_Sector:
				delete alignInfo.sector;
				break;
			case RenderAlignmentType_E::eRAT_Custom:
				delete alignInfo.custom;
				break;
		}
		type = RenderAlignmentType_E::eRAT_None;
	}

	void
	initFrom(
		const LapAlignment &from)
	{
		release();
		type = RenderAlignmentType_E::eRAT_Lap;
		alignInfo.lap = new LapAlignment(from);
	}

	void
	initFrom(
		const SectorAlignment &from)
	{
		release();
		type = RenderAlignmentType_E::eRAT_Sector;
		alignInfo.sector = new SectorAlignment(from);
	}

	void
	initFrom(
		const CustomAlignment &from)
	{
		release();
		type = RenderAlignmentType_E::eRAT_Custom;
		alignInfo.custom = new CustomAlignment(from);
	}
	
	// the type of alignment defined in union
	RenderAlignmentType_E type;

	union AlignmentInfo_U
	{
		AlignmentInfo_U()
		 : none(nullptr)
		{}

		~AlignmentInfo_U()
		{}

		void *none;
		LapAlignment *lap;
		SectorAlignment *sector;
		CustomAlignment *custom;
	} alignInfo;
};

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

	const RenderAlignmentInfo &
	getAlignmentInfo() const;

	void
	setAlignmentInfo(
		const RenderAlignmentInfo &renderAlignmentInfo);

	double
	getLeadOutSeconds() const;

	void
	setExportFilePath(
		const std::filesystem::path &path);

	const std::filesystem::path &
	getExportFilePath() const;

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
		const std::string &dir,
		bool noisy = false);

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

	// the last alignment info that was not a 'custom' alignment type.
	// this is used to populate the lap/sector alignment UI fields based on
	// what the user might have used to base the custom alignment off of.
	RenderAlignmentInfo lastNonCustomAlignmentInfo_;

	RenderAlignmentInfo currRenderAlignmentInfo_;

	// user-defined path to the final rendered video file
	std::filesystem::path exportFilePath_;

};

}// gpo

namespace YAML
{
	template<>
	struct convert<gpo::LapAlignment>
	{
		static Node
		encode(
			const gpo::LapAlignment& rhs)
		{
			Node node;
			node["lap"] = rhs.lap;
			node["side"] = (int)rhs.side;

			return node;
		}

		static bool
		decode(
			const Node& node,
			gpo::LapAlignment& rhs)
		{
			YAML_TO_FIELD(node,"lap",rhs.lap);
			rhs.side = (gpo::ElementSide_E)node["side"].as<int>();

			return true;
		}
	};

	template<>
	struct convert<gpo::SectorAlignment>
	{
		static Node
		encode(
			const gpo::SectorAlignment& rhs)
		{
			Node node;
			node["lap"] = rhs.lap;
			node["sector"] = rhs.sector;
			node["side"] = (int)rhs.side;

			return node;
		}

		static bool
		decode(
			const Node& node,
			gpo::SectorAlignment& rhs)
		{
			YAML_TO_FIELD(node,"lap",rhs.lap);
			YAML_TO_FIELD(node,"sector",rhs.sector);
			rhs.side = (gpo::ElementSide_E)node["side"].as<int>();

			return true;
		}
	};

	template<>
	struct convert<gpo::CustomAlignment>
	{
		static Node
		encode(
			const gpo::CustomAlignment& rhs)
		{
			Node node;

			Node idxBySourceNameNode;
			for (const auto &idxBySourceEntry : rhs.idxBySourceName)
			{
				Node newEntry;
				newEntry["sourceName"] = idxBySourceEntry.first;
				newEntry["index"] = idxBySourceEntry.second;
				idxBySourceNameNode.push_back(newEntry);
			}
			node["idxBySourceName"] = idxBySourceNameNode;

			return node;
		}

		static bool
		decode(
			const Node& node,
			gpo::CustomAlignment& rhs)
		{
			rhs.idxBySourceName.clear();
			auto &idxBySourceNameNode = node["idxBySourceName"];
			for (auto &item : idxBySourceNameNode)
			{
				auto sourceName = item["sourceName"].as<std::string>();
				auto index = item["index"].as<size_t>();
				rhs.idxBySourceName.insert({sourceName,index});
			}

			return true;
		}
	};

	template<>
	struct convert<gpo::RenderAlignmentInfo>
	{
		static Node
		encode(
			const gpo::RenderAlignmentInfo& rhs)
		{
			Node node;
			switch (rhs.type)
			{
				case gpo::RenderAlignmentType_E::eRAT_None:
					node["none"] = (size_t)rhs.alignInfo.none;
					break;
				case gpo::RenderAlignmentType_E::eRAT_Lap:
					node["lap"] = *rhs.alignInfo.lap;
					break;
				case gpo::RenderAlignmentType_E::eRAT_Sector:
					node["sector"] = *rhs.alignInfo.sector;
					break;
				case gpo::RenderAlignmentType_E::eRAT_Custom:
					node["custom"] = *rhs.alignInfo.custom;
					break;
			}

			return node;
		}

		static bool
		decode(
			const Node& node,
			gpo::RenderAlignmentInfo& rhs)
		{
			if (node["none"])
			{
				rhs.type = gpo::RenderAlignmentType_E::eRAT_None;
				rhs.alignInfo.none = (void*)node["none"].as<size_t>();
				return true;
			}

			if (node["lap"])
			{
				auto lap = node["lap"].as<gpo::LapAlignment>();
				rhs.initFrom(lap);
				return true;
			}

			if (node["sector"])
			{
				auto sector = node["sector"].as<gpo::SectorAlignment>();
				rhs.initFrom(sector);
				return true;
			}

			if (node["custom"])
			{
				auto custom = node["custom"].as<gpo::CustomAlignment>();
				rhs.initFrom(custom);
				return true;
			}

			return false;
		}
	};
}