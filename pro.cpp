// Paralell preorder
// PRL project at FIT VUT
// Author: Patrik Segedy <xseged00@vutbr.cz>

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <vector>
// #include <chrono>

using namespace std;

int main(int argc, char **argv) {
    int numprocs;               // number of processors
    int rank;                   // rank
    int my_val = -1;            // value of single processor
    int my_etour = -1;
    int my_weight = 0;
    int my_succ = -1;
    int my_list_rank = 0;
    int succ_list_rank = 0;
    int my_pred = -1;
    MPI_Status stat;         // struct- obsahuje kod- source, tag, error

    //MPI INIT
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
    vector<int> adj_list;
    for (int i = 0; i < strlen(argv[1])-1 ; i++) {
        adj_list.push_back(2*(i));
        adj_list.push_back(2*(i)+1);
    }

    // Scatter adj_list to all processors
    int adj_arr[adj_list.size()];
    copy(adj_list.begin(), adj_list.end(), adj_arr);
    MPI_Scatter(adj_arr, 1, MPI_INT, &my_val, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank%2 == 0)
        my_weight = 1;

    // EULER TOUR
    if (my_val%2 == 0) { // forward edge
        if (my_val == 0) {
            if (4 >= adj_list.size())
                my_etour = 1;
            else
                my_etour = 4;
        }
        else if (4*(my_val/2 + 1) >= adj_list.size())
            my_etour = my_val+1;
        else
            my_etour = 4*(my_val/2 + 1);
    }
    else {    // reverse edge
        if (((my_val-1)/2)%2) // right subtree
            if ((((my_val-1)/2) - 2) < 0)
                my_etour = 0;
            else
                my_etour = ((my_val-1)/2) - 2;
        else { // left subtree
            if (my_val + 1 >= adj_list.size())
                my_etour = ((my_val-1)/2) - 1;
            else
                my_etour = my_val+1;
        }
    }

    // add root
    if (numprocs >= 4 && my_val == 3) {
        my_etour = 3;
    }
    if (numprocs < 4 && my_val == 1) {
        my_etour = 1;
    }

    // SUFFIX SUM
    if (rank == my_etour) {
        my_list_rank = 0;
    }
    else
        my_list_rank = 1;

    // set -1 to last edge (to root)
    if (rank == my_etour)
        my_etour = -1;

    // get predecessor
    if (my_etour != -1)
        MPI_Send(&rank, 1, MPI_INT, my_etour, 1, MPI_COMM_WORLD);

    if (rank != 0)
        MPI_Recv(&my_pred, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &stat);

    // compute suffix sum / list rank
    for (int k = 0; k < log2(numprocs); k++) {

        if (my_etour != -1)
            MPI_Send(&my_pred, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD);

        if (my_pred != -1) {
            MPI_Send(&my_etour, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD);
            MPI_Send(&my_list_rank, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD);
        }

        if (my_etour != -1) {
            MPI_Recv(&my_succ, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD, &stat);
            MPI_Recv(&succ_list_rank, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD, &stat);
            my_etour = my_succ;
            my_list_rank += succ_list_rank;
        }

        if (my_pred != -1) {
            MPI_Recv(&my_pred, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD, &stat);
        }

    }


    int preorder_arr[numprocs] = {};
    // order is reversed after suffix sum
    int pos = (numprocs-1) - my_list_rank;

    // get preorder
    // CPU 0 -> first edge in preorder
    // CPU 1 -> second edge in preorder 
    // ... etc
    int preorder = -1;
    MPI_Send(&rank, 1, MPI_INT, pos, 2, MPI_COMM_WORLD);
    MPI_Recv(&preorder, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &stat);

    // get node index from edge number
    int res_node = -1;
    if (preorder%2 == 0)
        res_node = preorder / 2 + 1;

    int result[numprocs];
    MPI_Gather(&res_node, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // print preorder
        cout << argv[1][0];
        for (int i = 0; i < numprocs; i++) {
            if (result[i] != -1)
            cout << argv[1][result[i]];
        }
        cout << endl;
    }

    MPI_Finalize(); 
    return 0;
}
