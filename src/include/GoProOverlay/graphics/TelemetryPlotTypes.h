#pragma once

namespace gpo::TelemetryPlot
{

	enum X_Component
	{
		eXC_Samples = 0,
		eXC_Time = 1
	};

	enum Y_Component
	{
		eYC_UNKNOWN = 0,

		eYC_TIME = 1,

		eYC_ACCL_X = 2,
		eYC_ACCL_Y = 3,
		eYC_ACCL_Z = 4,
		eYC_GYRO_X = 5,
		eYC_GYRO_Y = 6,
		eYC_GYRO_Z = 7,
		eYC_GRAV_X = 23,
		eYC_GRAV_Y = 24,
		eYC_GRAV_Z = 25,
		eYC_CORI_W = 26,
		eYC_CORI_X = 27,
		eYC_CORI_Y = 28,
		eYC_CORI_Z = 29,
		eYC_GPS_LAT = 30,
		eYC_GPS_LON = 31,
		eYC_GPS_SPEED2D = 8,
		eYC_GPS_SPEED3D = 9,

		eYC_ECU_ENGINE_SPEED = 20,
		eYC_ECU_TPS = 21,
		eYC_ECU_BOOST = 22
	};

	enum AxisSide
	{
		eAS_Side1,// xAxis & yAxis
		eAS_Side2 // xAxis2 & yAxis2
	};

	struct Y_ComponentEnumInfo
	{
		Y_Component yComp;
		const char *name;
		const char *plotTitle;
		const char *axisTitle;
		const char *unit;
	};

}
