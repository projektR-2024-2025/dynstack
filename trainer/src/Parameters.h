#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include <ECF/ECF.h>

class Parameters
{
private:
	template<typename T>
	static void readParam(XMLNode& conf, T& var, const char *parent, const char *name) {
		XMLNode par = conf.getChildNode(parent);
		if (par.isEmpty())
			return;

		using UnderlyingType = typename std::remove_const<typename std::remove_reference<T>::type>::type;

		if constexpr (std::is_same<UnderlyingType, std::string>::value)
			var = !par.getChildNode(name).isEmpty() ? par.getChildNode(name).getText() : var;
		else if constexpr (std::is_same<UnderlyingType, float>::value)
			var = !par.getChildNode(name).isEmpty() ? std::stof(par.getChildNode(name).getText()) : var;
		else
			var = !par.getChildNode(name).isEmpty() ? std::stoi(par.getChildNode(name).getText()) : var;
	}
public:
	// main
	inline static std::string BEST_FILE = "best.txt";
	inline static bool USING_ECF = false;
	inline static bool RUN_BEST = false;
	// Runner
	inline static float KPI_W1 = 0.5;
	inline static float KPI_W2 = -0.4;
	inline static float KPI_W3 = 0.1;
	// Simulator
	inline static bool PRINT_STEPS = false;
	inline static bool INITALIZE_BUFFERS = true;
	inline static int MAX_ARRIVAL_SIZE = 3;
	inline static int MAX_BUFFER_SIZE = 12;
	inline static int MIN_INIT_BUFFER = 3;
	inline static int MAX_INIT_BUFFER = 5;
	inline static int MIN_DUE_TIME = 70;
	inline static int MAX_DUE_TIME = 90;
	inline static float MIN_WAIT_FAC = 0.1;
	inline static float MAX_WAIT_FAC = 0.3;
	inline static int ARRIVAL_PROB = 15; // /100
	inline static int HANDOVER_PROB = 22; // /100
	inline static bool SEED_SIM = true; // za CustomHeuristic seedaj sim prije pokretanja
	inline static int SIMULATOR_SEED = 4444; // pocetni seed
	// SimulatorEvalOp
	inline static int SIM_STEPS = 50;
	// PriorityHeuristic
	inline static int META_ALG = 0; // 0 default; 1,2,3 - kandidati

	static void readParameters(int argc, char** argv) {
		if (argc != 2) {
			std::cout << "Parameters file not provided properly!" << std::endl;
			std::cout << "Using default values!" << std::endl;
			return;
		}

		XMLNode params = XMLNode::parseFile(argv[1]);
		if (params.isEmpty()) {
			std::cout << "Parameters file (" << argv[1] << ") doesn't exist or isn't properly formated!" << std::endl;
			std::cout << "Using default values!" << std::endl;
			return;
		}

		if (!params.getChildNode("ECF").isEmpty())
			USING_ECF = true;

		XMLNode conf = params.getChildNode("Conf");

		if (!conf.isEmpty()) {
			readParam(conf, RUN_BEST, "Main", "RunBest");
			readParam(conf, BEST_FILE, "Main", "BestFile");

			readParam(conf, KPI_W1, "KPI", "KpiW1");
			readParam(conf, KPI_W2, "KPI", "KpiW2");
			readParam(conf, KPI_W3, "KPI", "KpiW3");

			readParam(conf, PRINT_STEPS, "Main", "PrintSteps");
			readParam(conf, INITALIZE_BUFFERS, "Simulator", "InitalizeBuffers");
			readParam(conf, MAX_ARRIVAL_SIZE, "Simulator", "MaxArrivalSize");
			readParam(conf, MAX_BUFFER_SIZE, "Simulator", "MaxBufferSize");
			readParam(conf, MIN_INIT_BUFFER, "Simulator", "MinInitialBuffer");
			readParam(conf, MAX_INIT_BUFFER, "Simulator", "MaxInitialBuffer");
			readParam(conf, MIN_DUE_TIME, "Simulator", "MinDueTime");
			readParam(conf, MAX_DUE_TIME, "Simulator", "MaxDueTime");
			readParam(conf, MIN_WAIT_FAC, "Simulator", "MinWaitFactor");
			readParam(conf, MAX_WAIT_FAC, "Simulator", "MaxWaitFactor");
			readParam(conf, ARRIVAL_PROB, "Simulator", "ArrivalProbability");
			readParam(conf, HANDOVER_PROB, "Simulator", "HandoverProbability");
			readParam(conf, SEED_SIM, "Main", "SeedSimulator");
			readParam(conf, SIMULATOR_SEED, "Main", "SimulatorSeed");

			readParam(conf, SIM_STEPS, "Main", "SimulationSteps");

			readParam(conf, META_ALG, "Main", "MetaAlg");
		}
	}
};

#endif
