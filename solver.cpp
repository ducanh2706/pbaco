#include <bits/stdc++.h>
#include <chrono>
#include "graph.h"
#include "solver.h"
#include "ant.h"
#include "parameters.h"
#include "utils.h"

using namespace std;

// pair<int, int> mTSPSolver::select_city(Ant &ant, vector<bool> &visited, vector<long double> &sum, vector<vector<pair<int, long double>>> &bucket){
//     RouletteWheel wheel;

//     for(int salesman = 0; salesman < salesmen; salesman++){
//         if(!bucket[salesman].size()){
//             sum[salesman] = 0;
//             int current_city = ant.tours[salesman].back();
//             for(int city = 0; city < n; city++){
//                 if(!visited[city]){
//                     long double prob = pheromone[current_city][city] * 1. / powl(graph.distance[current_city][city] * max(1., ant.tours[salesman].cost), BETA);
//                     //cout << pheromone[current_city][city] << endl;
//                     bucket[salesman].push_back({city, prob});
//                     sum[salesman] += prob;
//                 }
//             }
//             //cout << "new " << ' ' << current_city << ' ' << sum[salesman] << ' ' << ant.tours[salesman].cost<< endl;
//         }
//         wheel.add(sum[salesman]);
//     }

//     int sm = wheel.spin();
//     long double bound = rng_real(rng) * sum[sm];
//     long double accum = 0.;

//     for(auto &p : bucket[sm]){
//         if(visited[p.first]) continue;
//         accum += p.second;
//         if(accum > bound){
//             int city = p.first;
//             for(int salesman = 0; salesman < salesmen; salesman++){
//                 int current_city = ant.tours[salesman].back();
//                 long double prob = pheromone[current_city][city] * 1. / powl(graph.distance[current_city][city] * max(1., ant.tours[salesman].cost), BETA);
//                 sum[salesman] -= prob;
//             }
//             bucket[sm].clear();
//             return {sm, city};
//         }
//     }
    
//     return {-1, -1};
// }

pair<int, int> mTSPSolver::select_city(Ant &ant, vector<bool> &visited, vector<long double> &sum, vector<vector<pair<int, long double>>> &bucket){
    RouletteWheel wheel;
    int salesman = ant.shortest_tour_index();
    // double cost = 1e9;

    // for(int i = 0; i < salesmen; i++){
    //     if(ant.del.count(i) && ant.tours[i].cost < cost){
    //         cost = ant.tours[i].cost;
    //         salesman = i;
    //     }
    // }

    int current_city = ant.tours[salesman].back();

    vector<int> candidate;
    for(int city = 0; city < n; city++){
        if(!visited[city]){
            long double prob = pheromone[current_city][city] * 1. / powl(graph.distance[current_city][city], BETA);
            wheel.add(prob);
            candidate.push_back(city);
        }
    }

    assert(candidate.size());

    return {salesman, candidate[wheel.spin()]};
}

Ant mTSPSolver::build_solution(Ant ant){
    vector<bool> visited(n);
    vector<long double> sum(salesmen);
    vector<vector<pair<int, long double>>> bucket(salesmen);

    

    for(auto &tour : ant.tours){
        for(auto &vertex : tour){
            visited[vertex] = true;
        }
    }
    visited[DEPOT] = true;

    int not_visited = count(visited.begin(), visited.end(), 0);

    while(not_visited--){
        //cout << not_visited << endl;
        auto next = select_city(ant, visited, sum, bucket);
        ant.add(next.first, next.second);
       // cout << next.first << ' ' << next.second << endl;
        visited[next.second] = true;
    }

    ant.end_tour();
    ant.local_search();
    ant.calculate_result();

    return ant;
}

vector<Ant> mTSPSolver::build_solutions(){
    vector<Ant> ants;
    const int ANTS = ANTS_MULTIPLIER * n;

    #pragma omp parallel for
    for(int ant = 0; ant < ANTS; ant++){
        Ant a;
        if(iteration == 1)
            a = build_solution(Ant(salesmen, &graph.distance));
        else
            a = build_solution(trim(population.get()));

        #pragma omp critical
        {
            ants.push_back(a);
        }
    }

    return ants;
}

void mTSPSolver::update_pheromone(){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            pheromone[i][j] = pheromone[i][j] * (1 - RHO) + TAU_MIN * RHO;
        }
    }

    for(auto &tour : gbest.tours){
        for(int i = 0; i < tour.size() - 1; i++){
            pheromone[tour[i]][tour[i + 1]] = pheromone[tour[i]][tour[i + 1]] * (1 - RHO) - TAU_MIN * RHO + TAU_MAX * RHO;
            ///assert(pheromone[tour[i]][tour[i + 1]]  <= 1);
        }
    }
}

void mTSPSolver::solve(){

    for(; iteration <= ITERATIONS; iteration++){
        auto start = chrono::high_resolution_clock::now();


        auto ants = build_solutions();
        Ant ibest;

        for(auto &ant : ants){
            population.add(ant);
            if(gbest.min_max_cost - 1e-6 > ant.min_max_cost || (abs(gbest.min_max_cost - ant.min_max_cost) < 1e-6 && gbest.sqrt_cost - 1e-6 > ant.sqrt_cost)){
                gbest = ant;
            }

            if(ibest.min_max_cost - 1e-6 > ant.min_max_cost || (abs(ibest.min_max_cost - ant.min_max_cost) < 1e-6 && ibest.sqrt_cost - 1e-6 > ant.sqrt_cost)){
                ibest = ant;
            }
        }

        update_pheromone();

        population.kill();
        if(iteration % 10 == 0){
            for(auto &tour : gbest.tours){
                for(int &d : tour){
                    cout << d << ' ';
                }
                cout << endl;
            }

            for(auto &p : population.population){
                cout << p.min_max_cost << endl;
            }

            //population.population = {gbest};
        }

            auto end = chrono::high_resolution_clock::now();
            double time_taken = 
      chrono::duration_cast<chrono::nanoseconds>(end - start).count();
 
    time_taken *= 1e-9;

        cout << "Iteration " << iteration << ' ' << gbest.min_max_cost << ' ' << ibest.min_max_cost << ' ' << gbest.min_sum_cost << ' ' <<  time_taken << endl;
    }
}