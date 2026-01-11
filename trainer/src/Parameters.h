#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <vector>
#include <string>
#include <ECF/ECF.h>
#include <unordered_map>

class Parameters
{
private:
	template<typename T>
	static void readParam(XMLNode& conf, T& var, const char *name) {
		using UnderlyingType = typename std::remove_const<typename std::remove_reference<T>::type>::type;

		if constexpr (std::is_same<UnderlyingType, std::string>::value)
			var = !conf.getChildNode(name).isEmpty() ? conf.getChildNode(name).getText() : var;
		else if constexpr (std::is_same<UnderlyingType, float>::value)
			var = !conf.getChildNode(name).isEmpty() ? std::stof(conf.getChildNode(name).getText()) : var;
		else
			var = !conf.getChildNode(name).isEmpty() ? std::stoi(conf.getChildNode(name).getText()) : var;
	}
public:
	// main
	inline static std::string BEST_FILE = "best.txt";
	// Runner
	inline static float KPI_W1 = 0.5;
	inline static float KPI_W2 = -0.4;
	inline static float KPI_W3 = 0.1;
	// Simulator
	inline static bool PRINT_STEPS = false;
	inline static bool INITALIZE_BUFFERS = true;
	inline static int MAX_ARRIVAL_SIZE = 3;
	inline static int MAX_BUFFER_SIZE = 8;
	inline static int MIN_INIT_BUFFER = 2;
	inline static int MAX_INIT_BUFFER = 4;
	inline static int MIN_WAIT_TIME = 1;
	inline static int MAX_WAIT_TIME = 15;
	inline static int ARRIVAL_PROB = 10; // /100
	inline static int HANDOVER_PROB = 20; // /100
	// SimulatorEvalOp
	inline static int SIM_STEPS = 50;

	static void readParameters(const std::string& filename) {
		XMLNode conf = XMLNode::parseFile(filename.c_str()).getChildNode("Conf");

		if (!conf.isEmpty()) {
			readParam(conf, BEST_FILE, "BestFile");

			readParam(conf, KPI_W1, "KpiW1");
			readParam(conf, KPI_W2, "KpiW2");
			readParam(conf, KPI_W3, "KpiW3");

			readParam(conf, PRINT_STEPS, "PrintSteps");
			readParam(conf, INITALIZE_BUFFERS, "InitalizeBuffers");
			readParam(conf, MAX_ARRIVAL_SIZE, "MaxArrivalSize");
			readParam(conf, MAX_BUFFER_SIZE, "MaxBufferSize");
			readParam(conf, MIN_INIT_BUFFER, "MinInitialBuffer");
			readParam(conf, MAX_INIT_BUFFER, "MaxInitialBuffer");
			readParam(conf, MIN_WAIT_TIME, "MinWaitTime");
			readParam(conf, MAX_WAIT_TIME, "MaxWaitTime");
			readParam(conf, ARRIVAL_PROB, "ArrivalProbability");
			readParam(conf, HANDOVER_PROB, "HandoverProbability");

			readParam(conf, SIM_STEPS, "SimulationSteps");
		}
	}
};

#endif
