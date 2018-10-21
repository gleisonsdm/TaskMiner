#include "CostModel.h"

void CostModel::setRuntimeCost(uint32_t rCost)
{
	CostModel::runtimeCost = rCost;
}

void CostModel::setNWorkers(uint32_t nWorkers)
{
	CostModel::nWorkers = nWorkers;
}

uint32_t CostModel::runtimeCost = 500;
uint32_t CostModel::nWorkers = 2;
uint32_t CostModel::singleTaskCost = 0;