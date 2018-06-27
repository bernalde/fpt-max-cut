#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"

#include <iostream>
using namespace std;

const int kDataSetCount = 3;
const string paths[] = {
    "../data/biqmac/ising",
    "../data/biqmac/rudy",
    "../data/custom"
};




void EvaluateMarkedVertices(InputParser& input, const string data_filepath) {
    MaxCutGraph G(data_filepath);

    int k = 0, rule_taken;
    MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
    while ((rule_taken = TryOneWayReduce(G_processing_oneway, k)) != -1) {
        OutputDebugLog("RULE: " + to_string(rule_taken));
        OutputDebugLog("-----------");
    }

    G.SetMarkedVertices(G_processing_oneway.GetMarkedVerticesByOneWayRules());
    const int s_size_oneway = G.GetMarkedVerticesByOneWayRules().size();


    // Try reduce size of S
    G.ReduceMarksetVertexSet();
    const int s_size_oneway_with_reverse = G.GetMarkedVerticesByOneWayRules().size();
    const int s_size_adhoc = G.Algorithm3MarkedComputation_Randomized();


    OutputFilterMarkedVertices(input, data_filepath, G.GetNumNodes(), G.GetRealNumEdges(), s_size_oneway, s_size_oneway_with_reverse, s_size_adhoc);
}

vector<int> tot_used_rules(10, 0);
void EvaluateDataset(InputParser& input, const string data_filepath) {
    cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
    MaxCutGraph G(data_filepath);

    int k = 0;
    int rule_taken;
    OutputDebugLog("----------- START: APPLYING ONE-WAY REDUCTION RULES TO COMPUTE S -----------");
    MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
    while ((rule_taken = TryOneWayReduce(G_processing_oneway, k)) != -1) {
        OutputDebugLog("RULE: " + to_string(rule_taken));
        OutputDebugLog("-----------");
        tot_used_rules[rule_taken]++;
    }

    cout << "|V| = " << G.GetNumNodes() << endl;
    cout << "|E| = " << G.GetRealNumEdges() << endl; 
    cout << "EE = " << G.GetEdwardsErdosBound() << endl;
    cout << "k' = " << k << endl;

    G.SetMarkedVertices(G_processing_oneway.GetMarkedVerticesByOneWayRules());
    auto S = G.GetMarkedVerticesByOneWayRules();
    cout << "|S| = " << S.size() << endl;
    cout << "S: " << " ";
    for (auto node : S) cout << node << " ";
    cout << endl;
    OutputDebugLog("----------- DONE: APPLYING ONE-WAY REDUCTION RULES TO COMPUTE S -----------");

    // Try reduce size of S
    G.ReduceMarksetVertexSet();
    S = G.GetMarkedVerticesByOneWayRules();


    cout << "reduced |S| = " << S.size() << endl;

    sort(S.begin(), S.end());

#ifdef DEBUG
    auto G_minus_S_vertex_set = SetSubstract(G.GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(G, G_minus_S_vertex_set);
    bool debug_iscliquef = G_minus_S.IsCliqueForest();
    OutputDebugLog("Is clique forest G - S: " + to_string(debug_iscliquef));
    assert(debug_iscliquef);
#endif

    if (input.cmdOptionExists("-cc-brute")) { // temp flag since this is very slow
        int mx_sol = G.ComputeOptimalColoringBruteforce(S);
        cout << "mx_sol = " + to_string(mx_sol) << endl;
        cout << "Coloring: ";
        for (int color : G.GetMaxCutColoring()) cout << color << " ";
        cout << endl;

        auto allv = G.GetAllExistingNodes();
        auto sss = G.MaxCutExtension(allv, G.GetMaxCutColoring());
        cout << (get<0>(sss)) << endl;
    } else if (input.cmdOptionExists("-cc-brute-with-prunning")) {
        sort(S.begin(), S.end());
        int mx_sol = G.ComputeOptimalColoring(S);
        cout << "mx_sol = " + to_string(mx_sol) << endl;
    } else {
        int res;
        while ((res = ExhaustiveTwoWayReduce(G, S)) > -1) {
            cout << "Kernelization rule: " << res << " was applied." << endl;
            cout << "New G. Stats: " << "|V| = " << G.GetRealNumNodes() << " , |E| = " << G.GetRealNumEdges() << " , EE = " << G.GetEdwardsErdosBound() << endl;
        }

        if (input.cmdOptionExists("-print-kernalized-graph")) {
            const string output_filepath = input.getCmdOption("-print-kernalized-graph");
            ofstream out(output_filepath);
            G.PrintGraph(out);
            out.close();
            OutputDebugLog("Kernalized graph output is done.");
        }
    }
}

void EvaluateDatasetNew(InputParser& input, const string data_filepath) {
    (void) input;
    cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
    MaxCutGraph G(data_filepath);

    unordered_map<string, bool> visited;
    for (int sz = 70; sz <= 90; ++sz) {
        vector<int> previous;
        for (int r = 0; r < 200; ++r) {
            auto clique = G.GetAClique(sz, 2000);
            sort(clique.begin(), clique.end());
            string key = "";
            for (auto node : clique) key = key + to_string(node) + ",";
            if (visited[key]) continue;
            visited[key] = true;

            int isz = SetIntersection(clique, previous).size();
            cout << "Intersect with previous(" << previous.size() <<") size: " << isz << " (total: " << previous.size() + clique.size() - isz << ")" << endl;
            previous = clique;
            cout << "SZ: " << clique.size() << " = ";
            for (auto node : clique)
                cout << node << " ";
            cout << endl;

            vector<int> S = G.GetAllExistingNodes();
            S = SetSubstract(S, clique);

            cout << "|S| = " << S.size() << endl;
            G.SetMarkedVertices(S);
/*            G.ReduceMarksetVertexSet();
            S = G.GetMarkedVerticesByOneWayRules();
            cout << "|S| = " << S.size() << endl;*/
            

            MaxCutGraph wG = G;
            int res;
            while ((res = ExhaustiveTwoWayReduce(wG, S)) > -1) {
                cout << "Kernelization rule: " << res << " was applied." << endl;
                cout << "New G. Stats: " << "|V| = " << G.GetRealNumNodes() << " , |E| = " << wG.GetRealNumEdges() << " , EE = " << wG.GetEdwardsErdosBound() << endl;
                assert(false);
            }
        }
    }
}

void EvaluateDatasetCliqueDecomposition(InputParser& input, const string data_filepath) {
    (void) input;
    cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
    MaxCutGraph G(data_filepath);

    auto cdecomposition = G.DecomposeIntoCliques();
    cout << cdecomposition.size() << endl;

    vector<int> cnt(G.GetNumNodes(), 0); // how many times node i contained in a clique
    for (auto component : cdecomposition) {
        for (auto node : component)
            cnt[node]++;
    }

    int mxcnt = 0;
    vector<int> cntrev(G.GetNumNodes(), 0); // how many times is a node contained in only i cliques.
    vector<vector<int>> cntrev_content(G.GetNumNodes()); // all nodes contained in only i cliques.
    for (int i = 0; i < G.GetNumNodes(); ++i) {
        cntrev[cnt[i]]++;
        cntrev_content[cnt[i]].push_back(i);
        mxcnt = max(mxcnt, cnt[i]);
    }

    int fac = 1;
    for (int i = 1; i <= mxcnt; ++i) {
        MaxCutGraph newG(G, cntrev_content[i]);
        auto connected_components = newG.GetAllConnectedComponents();
        cout << "There are " << cntrev[i] << " nodes shared among " << i << " cliques. They are spread out into " << connected_components.size() << " connected components." << endl;

        for (auto component : connected_components)
            fac *= component.size();
    }

    cout << "Estimated calculation steps: " << fac << endl;
}

int main(int argc, char **argv){
    srand((unsigned)time(0));
    ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);
    InitOutputFiles(input);

    vector<string> all_sets_to_evaluate;
    if (input.cmdOptionExists("-f")) {
        const string data_filepath = input.getCmdOption("-f");
        all_sets_to_evaluate.push_back(data_filepath);
    } else {
        // get all datasets from data/
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }
    }

    
    for (string data_filepath : all_sets_to_evaluate) {
      //  EvaluateDataset(input, data_filepath);
        EvaluateDatasetCliqueDecomposition(input, data_filepath);
        ///EvaluateMarkedVertices(input, data_filepath);
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;
}