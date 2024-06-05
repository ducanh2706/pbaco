#include <bits/stdc++.h>
#include "ant.h"
#include "random.h"

using namespace std;

void Ant::add(int salesman, int vertex){
    tours[salesman].cost += (*distance)[tours[salesman].back()][vertex];
    tours[salesman].push_back(vertex);
}

void Ant::add(_tour &tour, int vertex){
    tour.cost += (*distance)[tour.back()][vertex];
    tour.push_back(vertex);
}

void Ant::end_tour(){
    min_max_cost = min_sum_cost = 0;

    for(auto &tour: tours){
        add(tour, 0);
        min_max_cost = max(min_max_cost, tour.cost);
        min_sum_cost += tour.cost;
    }
}

void Ant::calculate_result(){
    min_max_cost = min_sum_cost = 0;
    for(auto &tour : tours){
        min_max_cost = max(min_max_cost, tour.cost);
        min_sum_cost += tour.cost;
        sqrt_cost += tour.cost * tour.cost;
    }
    sqrt_cost = sqrt(sqrt_cost);
}

double tour_distance(_tour& a, _tour &b, vector<vector<double>> *distance){
    double x = 0;
    for(int i = 1; i < a.size() - 1; i++){
        for(int j = 1; j < b.size() - 1; j++){
            x += (*distance)[a[i]][b[j]];
        }
    }
    return x / double(a.size() - 2) / double(b.size() - 2);
}

Ant trim(Ant ant){
    int salesmen = ant.tours.size();
    vector<vector<int>> dis(salesmen, vector<int>(salesmen));
    for(int i = 0; i < salesmen; i++)
        for(int j = i + 1; j < salesmen; j++)
            dis[i][j] = dis[j][i] = tour_distance(ant.tours[i], ant.tours[j], ant.distance);
            
    int del_count = rng() % salesmen;
    int last_del = ant.longest_tour_index();
    unordered_set<int> del_list = {last_del};

    while(del_count--){
        int next_del = 0;
        double d = 1e9;

        for(int i = 0; i < salesmen; i++){
            if(!del_list.count(i) && dis[last_del][i] < d){
                d = dis[last_del][i];
                del_list.insert(i);
            }
        }
    }

    for(int i = 0; i < ant.tours.size(); i++){
        if(del_list.count(i)){
            ant.tours[i].tour = {DEPOT};
            ant.tours[i].cost = 0;
        }else{
            ant.tours[i].tour.pop_back();
            ant.tours[i].cost = ant.tour_length(ant.tours[i]);
        }
    }

    ant.del = del_list;

    return ant;
}