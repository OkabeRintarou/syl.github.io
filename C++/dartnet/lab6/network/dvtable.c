
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../common/constants.h"
#include "../topology/topology.h"
#include "dvtable.h"

//This function creates a dvtable(distance vector table) dynamically.
//A distance vector table contains the n+1 entries, where n is the number of the neighbors of this node, and the rest one is for this node itself. 
//Each entry in distance vector table is a dv_t structure which contains a source node ID and an array of N dv_entry_t structures where N is the number of all the nodes in the overlay.
//Each dv_entry_t contains a destination node address the the cost from the source node to this destination node.
//The dvtable is initialized in this function.
//The link costs from this node to its neighbors are initialized using direct link cost retrived from topology.dat. 
//Other link costs are initialized to INFINITE_COST.
//The dynamically created dvtable is returned.
dv_t* dvtable_create()
{
    int neighborNum = topology_getNbrNum();
    int overlayNodesNum = topology_getNodeNum();
    int myNodeId = topology_getMyNodeID();
    int* neighborArray = topology_getNbrArray();
    int* nodeArray = topology_getNodeArray();

    dv_t* dv = (dv_t*)malloc(sizeof(dv_t) * (neighborNum + 1));
    int i,j;
    for(i = 0; i < neighborNum;i++){
        dv[i].nodeID = neighborArray[i];
        dv[i].dvEntry = (dv_entry_t*)malloc(sizeof(dv_entry_t) * overlayNodesNum);
        for(j = 0; j < overlayNodesNum;j++){
            dv[i].dvEntry[j].nodeID = nodeArray[j];
            dv[i].dvEntry[j].cost = topology_getCost(dv[i].nodeID,nodeArray[j]);
        }
    }

    dv[neighborNum].nodeID = myNodeId;
    dv[neighborNum].dvEntry = (dv_entry_t*)malloc(sizeof(dv_entry_t) * overlayNodesNum);
    for(j = 0; j < overlayNodesNum;j++){
        dv[neighborNum].dvEntry[j].nodeID = nodeArray[j];
        dv[neighborNum].dvEntry[j].cost = topology_getCost(myNodeId,nodeArray[j]);
    }

    free(neighborArray);
    free(nodeArray);

    return dv;
}

//This function destroys a dvtable. 
//It frees all the dynamically allocated memory for the dvtable.
void dvtable_destroy(dv_t* dvtable)
{
    int neighborNum = topology_getNbrNum();
    int i;
    for(i = 0; i < neighborNum + 1;++i){
        free(dvtable[i].dvEntry);
    }
    free(dvtable);
}

//This function sets the link cost between two nodes in dvtable.
//If those two nodes are found in the table and the link cost is set, return 1.
//Otherwise, return -1.
int dvtable_setcost(dv_t* dvtable,int fromNodeID,int toNodeID, unsigned int cost)
{
    int neighborNum = topology_getNbrNum();
    int overlayNodesNum = topology_getNodeNum();
    int i,j;
    int flag = -1;

    if(fromNodeID == toNodeID)return -1;
    for(i = 0; i < neighborNum + 1;i++){
        for(j = 0; j < overlayNodesNum;j++){
            if((fromNodeID == dvtable[i].nodeID && toNodeID == dvtable[i].dvEntry[j].nodeID) ||
                    (fromNodeID == dvtable[i].dvEntry[j].nodeID && fromNodeID == dvtable[i].nodeID)){
                dvtable[i].dvEntry[j].cost = cost;
                flag = 1;
            }
        }
    }

    return flag;
}

//This function returns the link cost between two nodes in dvtable
//If those two nodes are found in dvtable, return the link cost. 
//otherwise, return INFINITE_COST.
unsigned int dvtable_getcost(dv_t* dvtable, int fromNodeID, int toNodeID)
{
    int neighborNum = topology_getNbrNum();
    int overlayNodesNum = topology_getNodeNum();
    int i,j;

    if(fromNodeID == toNodeID)return 0;
    for(i = 0; i < neighborNum + 1;i++){
        for(j = 0; j < overlayNodesNum;j++){
            if((fromNodeID == dvtable[i].nodeID && toNodeID == dvtable[i].dvEntry[j].nodeID) ||
                    (fromNodeID == dvtable[i].dvEntry[j].nodeID && fromNodeID == dvtable[i].nodeID)){
                return dvtable[i].dvEntry[j].cost;
            }   
        }
    }

    return INFINITE_COST;
}

//This function prints out the contents of a dvtable.
void dvtable_print(dv_t* dvtable)
{
    int neighborNum = topology_getNbrNum();
    int overlayNodesNum = topology_getNodeNum();
    int i,j;

    printf("      ");
    for(i = 0; i < overlayNodesNum;i++){
        printf("%3d  ",dvtable[0].dvEntry[i].nodeID);
    }
    printf("\n");

    for(i = 0; i < neighborNum + 1;i++){
        printf("%3d  ",dvtable[i].nodeID);
        for(j = 0; j < overlayNodesNum;j++){   
            printf("%3d  ",dvtable[i].dvEntry[j].cost);
           // printf("%d -> %d,cost: %d\n",dvtable[i].nodeID,dvtable[i].dvEntry[j].nodeID,dvtable[i].dvEntry[j].cost);
        }
        printf("\n");
    }
    return;
}
