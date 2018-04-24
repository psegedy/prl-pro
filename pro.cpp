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

typedef struct node_t {
    int node;
    int f_edge;
    int r_edge;
    int next_f_edge;
    int next_r_edge;
} node_t;


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
    int weight[adj_list.size()] = {};
    int suff_weight[adj_list.size()] = {};
    int succ[adj_list.size()] = {};
    int succ2[adj_list.size()] = {};
    int list_rank[adj_list.size()] = {};
    int my_v[adj_list.size()] = {};
    int position[adj_list.size()] = {};

    // for (int i = 0; i < adj_list.size()-1; i += 2) {
    //     cout << adj_list[i] << "," << adj_list[i+1] << " ";
    // }
    // cout << endl;

    int adj_arr[adj_list.size()];
    int etour[adj_list.size()];
    copy(adj_list.begin(), adj_list.end(), adj_arr);
    // copy(adj_list.begin(), adj_list.end(), adj_arr);

    MPI_Scatter(adj_arr, 1, MPI_INT, &my_val, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Scatter(weight, 1, MPI_INT, &my_weight, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank%2 == 0)
        my_weight = 1;

    if (rank == 0)
    {
        cout << adj_list.size() << endl;
    }

    // euler tour
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

    MPI_Gather(&my_etour, 1, MPI_INT, etour, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&my_weight, 1, MPI_INT, weight, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if (rank == my_etour) {
        my_list_rank = 0;
    }
    else
        my_list_rank = 1;
        // my_list_rank = my_weight;


    if (rank == my_etour)
        my_etour = -1;

    if (my_etour != -1)
        MPI_Send(&rank, 1, MPI_INT, my_etour, 1, MPI_COMM_WORLD);

    if (rank != 0)
        MPI_Recv(&my_pred, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &stat);

    // cout << "rank: " << rank << " pred: " << my_pred << endl;


    for (int k = 0; k < log2(numprocs); k++) {

        if (my_pred != -1) {
            MPI_Send(&my_etour, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD);
            MPI_Send(&my_list_rank, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD);
            // cout << "k: " << k  << " Send: " << my_etour << " to: " << my_pred <<  " rank: " << rank << endl;
        }

        if (my_etour != -1) {
            // cout << "Recv from: " << my_etour << " rank: " << rank <<  endl;
            MPI_Send(&my_pred, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD);
            MPI_Recv(&my_succ, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD, &stat);
            MPI_Recv(&succ_list_rank, 1, MPI_INT, my_etour, 0, MPI_COMM_WORLD, &stat);
            my_etour = my_succ;
            my_list_rank += succ_list_rank;
        }

        if (my_pred != -1) {
            MPI_Recv(&my_pred, 1, MPI_INT, my_pred, 0, MPI_COMM_WORLD, &stat);
        }

    }

    // preorder 
    // int pos = -1;
    // if (my_weight == 1) {
    //     pos = (numprocs-1) - my_list_rank;
    // }
    int preorder_arr[numprocs] = {};
    // MPI_Gather(&pos, 1, MPI_INT, preorder_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // MPI_Barrier(MPI_COMM_WORLD);
    int pos = (numprocs-1) - my_list_rank;
    int preorder = -1;
    cout << "pos: " << pos << endl;
    MPI_Send(&rank, 1, MPI_INT, pos, 2, MPI_COMM_WORLD);
    MPI_Recv(&preorder, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &stat);

    // get node index from edge number
    int res_node = -1;
    if (preorder%2 == 0)
        res_node = preorder / 2 + 1;

    int result[numprocs];
    MPI_Gather(&res_node, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);


    MPI_Gather(&my_list_rank, 1, MPI_INT, list_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&preorder, 1, MPI_INT, preorder_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < adj_list.size()-1; i += 2) {
            cout << adj_list[i] << " " << adj_list[i+1] << " ";
        }
        cout << endl;

        for (int i = 0; i < adj_list.size(); ++i) {
            cout << etour[i] << " ";
        }
        cout << endl;

        for (int i = 0; i < adj_list.size(); ++i) {
            cout << list_rank[i] << " ";
        }
        cout << endl;

        for (int i = 0; i < adj_list.size(); ++i) {
            cout << weight[i] << " ";
        }
        cout << endl;

        for (int i = 0; i < adj_list.size(); ++i) {
                cout << preorder_arr[i] << " ";
        }
        cout << endl;

        cout << "preorder" << endl;
        vector<int> preord;
        cout << "0 ";
        preord.push_back(0);
        for (int i = 0; i < adj_list.size(); ++i) {
            if (weight[i]) {
                cout << preorder_arr[i] << " ";
                preord.push_back(preorder_arr[i]);
            }
        }
        cout << endl;

        for (int i = 0; i < numprocs; ++i)
        {
            cout << result[i] << " ";
        }
        cout << endl;

        cout << 0 << " ";
        for (int i = 0; i < numprocs; i++) {
            if (result[i] != -1)
            cout << result[i] << " ";
        }
        cout << endl;

        cout << argv[1][0];
        for (int i = 0; i < numprocs; i++) {
            if (result[i] != -1)
            cout << argv[1][result[i]];
        }
        cout << endl;

        // for(auto&& i : preord) {
        //     cout << argv[1][i];
        // }
        // cout << endl;

    }

    MPI_Finalize(); 
    return 0;
}
