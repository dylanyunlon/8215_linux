#include "icluster.h"
#include "clusterservice.h"

ICluster *ICluster::getInstance()
{
    return ClusterService::getInstance();
}
