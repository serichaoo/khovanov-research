#ifndef DIFFERENTIAL_MAPS
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include "matrices.hpp"

using namespace std;

using ll = long long;
using ull = unsigned long long;


class PD{
    public:
        vector<vector<int>> crossings; // crossing[i].size() should always be 4
        int size(){
            return crossings.size();
        }
        PD(int n = 0){
            crossings.resize(n);
            for (int i = 0; i < n; i++){
                crossings[i].resize(4);
            }
        }
};

void setMerge(set<int>& a, set<int>& b){ // puts b into a
    for (int x : b){
        a.insert(x);
    }
}

template<typename T> set<T> setComplement(set<T> a, set<T> b){ // a setminus b
    for (T x : b){
        a.erase(x);
    }
    return a;
}

void insertVector(vector<ll> &basis, ll mask) {
    for (ll i = 0; i < basis.size(); i++) {
        if (!(mask & (1ll << i))) continue;
        if (!basis[i]) {
            basis[i] = mask;
        }
        mask ^= basis[i];
    }
}

bool isLinearlyIndependent(const vector<ll> &basis, ll mask){
    for (ll i = 0; i < basis.size(); i++){
        if (!(mask & (1ll << i))) continue;
        if (!basis[i]) return 1;
        mask ^= basis[i];
    }
    return 0;
}

set<set<int>> resolutionCircles(PD diagram, ll resolution){
    vector<set<int>> circles;
    for (int i = 0; i < diagram.size(); i++){
        vector<pair<int, int>> strandPairings(2);
        if (!(resolution & (1ll << i))){
            // pair crossing[0] with crossing[1], crossing[2] with crossing[3]
            strandPairings[0] = {diagram.crossings[i][0], diagram.crossings[i][1]};
            strandPairings[1] = {diagram.crossings[i][2], diagram.crossings[i][3]};
        }
        else{
            // pair crossing[0] with crossing[3], crossing[1] with crossing[2]
            strandPairings[0] = {diagram.crossings[i][0], diagram.crossings[i][3]};
            strandPairings[1] = {diagram.crossings[i][1], diagram.crossings[i][2]};
        }
        for (int j = 0; j < 2; j++){
            bool found = 0;
            vector<set<int>> toMerge; // merge circles that have overlapping strands
            for (int i = 0; (ull) i < circles.size(); i++){
                set<int> circle = circles[i];
                if (circle.count(strandPairings[j].first) || circle.count(strandPairings[j].second)){
                    found = 1;
                    toMerge.push_back(circles[i]);
                    circles.erase(circles.begin() + i);
                    i--;
                }
            }
            while (toMerge.size() > 1){
                setMerge(toMerge[0], toMerge.back());
                toMerge.pop_back();
            }
            if (toMerge.size()){
                toMerge[0].insert(strandPairings[j].first);
                toMerge[0].insert(strandPairings[j].second);
                circles.push_back(toMerge[0]);
            }
            if (!found){
                set<int> newCircle{strandPairings[j].first, strandPairings[j].second};
                circles.push_back(newCircle);
            }
        }
    }
    set<set<int>> ret;
    for (auto s : circles) ret.insert(s);
    return ret;
}

PD readPlanarDiagram(int n){ // reads planar diagram, given n crossings
// space-separated
    PD D(n);
    for (int i = 0; i < n; i++){
        for (int j = 0; j < 4; j++){
            cin >> D.crossings[i][j];
        }
    }
    return D;
}

PD createPlanarDiagram(vector<vector<int>> &crossings){ // crossings[i].size() should be 4
    PD D(crossings.size());
    for (int i = 0; i < crossings.size(); i++){
        for (int j = 0; j < 4; j++){
            D.crossings[i][j] = crossings[i][j];
        }
    }
    return D;
}

vector<Matrix> regularDifferentialMaps(PD D){
// returns n matrices, each mapping from k 1-resolutions to k+1 1-resolutions for 0 <= k < n
// works with the unreduced Khovanov homology to obtain differentials
// complexity: O(n * 4^n)

    // int n;
    // cout << "Input the number of crossings (no more than 60):" << endl;
    // cin >> n;
    // cout << "Input " << 4 * n << " space-separated numbers in planar diagram notation." << endl;

    // PD D(n);
    // for (int i = 0; i < n; i++){
    //     for (int j = 0; j < 4; j++){
    //         cin >> D.crossings[i][j];
    //     }
    // }

    // testing if resolutionCircles produces correct circles
    // set<set<int>> res = resolutionCircles(D, 2);
    // cout << res.size() << endl;

    // construct resolution cube
    int n = D.size();
    vector<set<set<int>>> resolutionCube(1ll << n);
    for (ll i = 0; i < (1ll << n); i++){
        resolutionCube[i] = resolutionCircles(D, i);
        // cerr << i << ": " << resolutionCube[i].size() << endl;
    }

    vector<ll> ordering((1ll << n));
    vector<ll> circleStartingIndex((1ll << n));
    // ordering[k] takes in a resolution k (binary string) and gives the
    // zero-indexed order for all elements
    // that have the same number of bits as k

    // circleStartingIndex[k] takes in a resolution k (binary string)
    // and gives the starting index for basis elements of a
    // particular circle compared to all circles with the same number of bits
    vector<ll> bitCount(n+1, 0);
    vector<ll> basisStartCount(n+1, 0);
    for (ll i = 0; i < (1ll << n); i++){
        ordering[i] = bitCount[__builtin_popcountll(i)]++;
        circleStartingIndex[i] = basisStartCount[__builtin_popcountll(i)];
        basisStartCount[__builtin_popcountll(i)] += (1ll << resolutionCube[i].size());
    }
    


    vector<Matrix> differentialMap(n);
    // differentialMap[i] gives the differential map from i 1-resolutions to
    // i+1 1-resolutions
    // differentialMap[i] stores the matrix as a (horizontal) vector of column vectors
    
    // resize differentialMap matrices
    for (int i = 0; i < n; i++){
        differentialMap[i] = Matrix(basisStartCount[i], basisStartCount[i+1]);
    }

    vector<map<set<int>, ll>> resolutionCircleIndices(1ll << n);
    // get the circle indices of everything in a resolution

    vector<vector<set<int>>> resolutionCirclesVector(1ll << n);
    // store the circles of a resolution in order in a vector

    for (ll resolution = 0; resolution < (1ll << n); resolution++){
        ll count = 0;
        for (auto x : resolutionCube[resolution]){
            resolutionCircleIndices[resolution][x] = count++;
            resolutionCirclesVector[resolution].push_back(x);
        }
    }

    for (ll resolution = 0; resolution < (1ll << n); resolution++){
        for (int j = 0; j < n; j++){
            if ((resolution & (1ll << j)) != 0) continue; // jth bit already set
            // jth bit not yet set
            ll newResolution = resolution | (1ll << j);
            set<set<int>> oldCircles = resolutionCube[resolution];
            set<set<int>> newCircles = resolutionCube[newResolution];

            set<set<int>> oldDiff = setComplement<set<int>>(oldCircles, newCircles);
            set<set<int>> newDiff = setComplement<set<int>>(newCircles, oldCircles);

            for (ll oldCirclesSubset = 0; oldCirclesSubset < (1ll << oldCircles.size()); oldCirclesSubset++){
                ll oldIndex = circleStartingIndex[resolution] + oldCirclesSubset;
                // (-) <-> 0, (+) <-> 1
                if (oldDiff.size() == 2){ // exactly 2 circles in the old resolution not in the new resolution
                    // must be a merge
                    // (-) x (-) -> (-); (-) x (+) = (+) x (-) -> (+), (+) x (+) -> (-)
                    bool circleOneStatus = ((oldCirclesSubset 
                    & (1ll << resolutionCircleIndices[resolution][*(oldDiff.begin())])) != 0);
                    bool circleTwoStatus = ((oldCirclesSubset 
                    & (1ll << resolutionCircleIndices[resolution][*(++oldDiff.begin())])) != 0);

                    bool newCircleStatus = circleOneStatus ^ circleTwoStatus;
                    // rule based on Audoux's notation

                    ll newCircleIndex = circleStartingIndex[newResolution];
                    // update index for the new merged circle
                    if (newCircleStatus) newCircleIndex += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                    for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                        if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                            if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                newCircleIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                            }
                        }
                    }

                    differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                }
                else if (oldDiff.size() == 1){ // must be a split
                    // (+) -> (+)(+) + (-)(-); (-) -> (-)(+) + (+)(-)
                    // based on Audoux's notation

                    ll newCircleIndex1 = circleStartingIndex[newResolution];
                    ll newCircleIndex2 = circleStartingIndex[newResolution];
                    for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                        if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                            if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                newCircleIndex1 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                                newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                            }
                        }
                    }

                    // implementing (+) ->
                    if (oldCirclesSubset & (1ll << resolutionCircleIndices[resolution][*oldDiff.begin()])){ // (+) ->
                        // newCircleIndex1: (-)(-), newCircleIndex2: (+)(+)
                        newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                        newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*(++newDiff.begin())]);

                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex1] = 1;
                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex2] = 1;
                    }
                    else{ // (-) ->
                        // newCircleIndex1: (+)(-), newCircleIndex2: (-)(+)
                        newCircleIndex1 += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                        newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*(++newDiff.begin())]);

                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex1] = 1;
                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex2] = 1;
                    }
                }
            }
        }
    }

    return differentialMap;
}

vector<Matrix> reducedDifferentialMaps(PD D){
    // returns the reduced differential map of a planar diagram D
    // assumes that strand 1 is the marked strand
    
    // int n; cin >> n;
    // PD D = readPlanarDiagram(n);

    int n = D.size();

    // construct resolution cube
    // int n = D.size();
    vector<set<set<int>>> resolutionCube(1ll << n);
    for (ll i = 0; i < (1ll << n); i++){
        resolutionCube[i] = resolutionCircles(D, i);
        // cerr << i << ": " << resolutionCube[i].size() << endl;
    }

    vector<ll> ordering((1ll << n));
    vector<ll> circleStartingIndex((1ll << n));
    // ordering[k] takes in a resolution k (binary string) and gives the
    // zero-indexed order for all elements
    // that have the same number of bits as k

    // circleStartingIndex[k] takes in a resolution k (binary string)
    // and gives the starting index for basis elements of a
    // particular circle compared to all circles with the same number of bits
    vector<ll> bitCount(n+1, 0);
    vector<ll> basisStartCount(n+1, 0);
    for (ll i = 0; i < (1ll << n); i++){
        ordering[i] = bitCount[__builtin_popcountll(i)]++;
        circleStartingIndex[i] = basisStartCount[__builtin_popcountll(i)];
        basisStartCount[__builtin_popcountll(i)] += (1ll << (resolutionCube[i].size() - 1));
        // the -1 comes from forcing strand 1 to be labelled as X, no choice -> halves dimension
    }

    vector<Matrix> differentialMap(n);
    // differentialMap[i] gives the differential map from i 1-resolutions to
    // i+1 1-resolutions
    // differentialMap[i] stores the matrix as a (horizontal) vector of column vectors
    
    // resize differentialMap matrices
    for (int i = 0; i < n; i++){
        differentialMap[i] = Matrix(basisStartCount[i], basisStartCount[i+1]);
    }

    vector<map<set<int>, ll>> resolutionCircleIndices(1ll << n);
    // get the circle indices of everything in a resolution

    vector<vector<set<int>>> resolutionCirclesVector(1ll << n);
    // store the circles of a resolution in order in a vector

    for (ll resolution = 0; resolution < (1ll << n); resolution++){
        ll count = -1;
        for (auto x : resolutionCube[resolution]){
            resolutionCircleIndices[resolution][x] = count++;
            resolutionCirclesVector[resolution].push_back(x);
        }
    }

    for (ll resolution = 0; resolution < (1ll << n); resolution++){
        for (int j = 0; j < n; j++){
            if ((resolution & (1ll << j)) != 0) continue; // jth bit already set
            // jth bit not yet set
            ll newResolution = resolution | (1ll << j);
            set<set<int>> oldCircles = resolutionCube[resolution];
            set<set<int>> newCircles = resolutionCube[newResolution];

            set<set<int>> oldDiff = setComplement<set<int>>(oldCircles, newCircles);
            set<set<int>> newDiff = setComplement<set<int>>(newCircles, oldCircles);

            bool containsX = 0;
            for (auto x : oldDiff){
                if (x.count(1)) containsX = 1;
            }

            if (!containsX){
                for (ll oldCirclesSubset = 0; oldCirclesSubset < (1ll << (oldCircles.size()-1)); oldCirclesSubset++){
                    ll oldIndex = circleStartingIndex[resolution] + oldCirclesSubset;
                    // (-) <-> 0, (+) <-> 1
                    if (oldDiff.size() == 2){ // exactly 2 circles in the old resolution not in the new resolution
                        // must be a merge
                        // (-) x (-) -> (-); (-) x (+) = (+) x (-) -> (+), (+) x (+) -> (-)
                        bool circleOneStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(oldDiff.begin())])) != 0);
                        bool circleTwoStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(++oldDiff.begin())])) != 0);

                        bool newCircleStatus = circleOneStatus ^ circleTwoStatus;
                        // rule based on Audoux's notation

                        ll newCircleIndex = circleStartingIndex[newResolution];
                        // update index for the new merged circle
                        if (newCircleStatus) newCircleIndex += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                        for (ll oldCirclesIndex = 0; oldCirclesIndex < oldCircles.size() - 1ll; oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex + 1])){
                                    newCircleIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                                }
                            }
                        }

                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                    }
                    else if (oldDiff.size() == 1){ // must be a split
                        // (+) -> (+)(+) + (-)(-); (-) -> (-)(+) + (+)(-)
                        // based on Audoux's notation

                        ll newCircleIndex1 = circleStartingIndex[newResolution];
                        ll newCircleIndex2 = circleStartingIndex[newResolution];
                        for (ll oldCirclesIndex = 0; oldCirclesIndex < oldCircles.size()-1ll; oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex + 1])){
                                    newCircleIndex1 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                                    newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                                }
                            }
                        }

                        // implementing (+) ->
                        if (oldCirclesSubset & (1ll << resolutionCircleIndices[resolution][*oldDiff.begin()])){ // (+) ->
                            // newCircleIndex1: (-)(-), newCircleIndex2: (+)(+)
                            newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                            newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*(++newDiff.begin())]);

                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex1] = 1;
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex2] = 1;
                        }
                        else{ // (-) ->
                            // newCircleIndex1: (+)(-), newCircleIndex2: (-)(+)
                            newCircleIndex1 += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                            newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*(++newDiff.begin())]);

                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex1] = 1;
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex2] = 1;
                        }
                    }
                }
            }
            else { // contains X in the merge/split
                for (ll oldCirclesSubset = 0; oldCirclesSubset < (1ll << (oldCircles.size()-1)); oldCirclesSubset++){
                    ll oldIndex = circleStartingIndex[resolution] + oldCirclesSubset;
                    if (oldDiff.size() == 2){ // must be a merge
                        ll newCircleIndex = circleStartingIndex[newResolution];
                        for (ll oldCirclesIndex = 0; oldCirclesIndex < oldCircles.size() - 1ll; oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                if (!oldDiff.count(resolutionCirclesVector[resolution][oldCirclesIndex + 1])){
                                    newCircleIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                                }
                            }
                        }
                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                    }
                    else if (oldDiff.size() == 1){ // must be a split
                        ll newCircleIndex1 = circleStartingIndex[newResolution];
                        ll newCircleIndex2 = circleStartingIndex[newResolution];
                        for (ll oldCirclesIndex = 0; oldCirclesIndex < oldCircles.size() - 1ll; oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                newCircleIndex1 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                                newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex + 1]]);
                            }
                        }
                        newCircleIndex2 += (1ll << resolutionCircleIndices[newResolution][*(++newDiff.begin())]);
                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex1] = 1;
                        differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex2] = 1;
                    }
                }
            }
        }
    }

    return differentialMap;
}

PD getPlanarDiagram(){
    // reads planar diagram notation from input.txt and returns the differential maps
    freopen("input.txt", "r", stdin);
    vector<vector<int>> input;
    int x;
    while (cin >> x){
        if (!input.size() || input[input.size()-1].size() == 4){
            input.push_back({x});
        }
        else{
            input[input.size()-1].push_back(x);
        }
    }

    PD D = createPlanarDiagram(input);
    return D;
}

vector<Matrix> getMaps(PD D, bool reducedHomology){
    vector<Matrix> maps;
    if (reducedHomology) maps = reducedDifferentialMaps(D);
    else maps = regularDifferentialMaps(D);

    return maps;
}


// annular stuff below

namespace annular{
    bool containsPuncture(const set<int> &circle, const vector<ll> &basis1, const vector<ll> &basis2){
        ll mask = 0;
        for (auto x : circle){
            mask += (1ll << (x-1));
        }
        if (isLinearlyIndependent(basis2, mask)){
            cerr << "Issue with faces and/or crossings. Please check input." << endl;
            exit(1);
        }
        return isLinearlyIndependent(basis1, mask);
    }

    vector<bool> mapVVtoA(bool first, bool second){
        if (first != second) return {0, 1}; // means assign V-
        return {};
    }

    bool mapVAtoV(bool first, bool second){ 
        return first;
    }

    vector<bool> annularMerge(const set<int> &circle1, bool circle1Status, const set<int> &circle2, bool circle2Status, const vector<ll> &basis1, const vector<ll> &basis2){
        bool circle1HasPuncture = containsPuncture(circle1, basis1, basis2);
        bool circle2HasPuncture = containsPuncture(circle2, basis1, basis2);
        if (circle1HasPuncture && circle2HasPuncture){
            return mapVVtoA(circle1Status, circle2Status);
        }
        if (!circle1HasPuncture && !circle2HasPuncture){ // regular Audoux map
            return {circle1Status != circle2Status};
        }
        if (circle2HasPuncture) swap(circle1Status, circle2Status);
        return {mapVAtoV(circle1Status, circle2Status)};
    }

    vector<pair<bool, bool>> mapAtoVV(bool first){ // returns a sum
        if (first) return {{1, 0}, {0, 1}};
        return {};
    }

    vector<pair<bool, bool>> mapVtoVA(bool first){
        return {{first, 0}, {first, 1}};
    }

    vector<pair<bool, bool>> annularSplit(const set<int> &circle, bool circleStatus, const set<int> &res1, const set<int> &res2, const vector<ll> &basis1, const vector<ll> &basis2){
        bool origHasPuncture = containsPuncture(circle, basis1, basis2);
        bool res1HasPuncture = containsPuncture(res1, basis1, basis2);
        bool res2HasPuncture = containsPuncture(res2, basis1, basis2);
        if (!origHasPuncture && !res1HasPuncture && !res2HasPuncture){ // regular split
            if (circleStatus){ // (+) -> (-)(-) + (+)(+)
                return {{0, 0}, {1, 1}};
            }
            else{ // (-) -> (-)(+) + (+)(-)
                return {{0, 1}, {1, 0}};
            }
        } 
        if (origHasPuncture){
            if (res1HasPuncture){
                return mapVtoVA(circleStatus);
            }
            else if (res2HasPuncture){
                auto ret = mapVtoVA(circleStatus);
                swap(ret[0].first, ret[0].second);
                swap(ret[1].first, ret[1].second);
                return ret;
            }
            assert(0); // split configuration invalid
        }
        else{
            if (res1HasPuncture && res2HasPuncture){
                return {{1, 0}, {0, 1}};
            }
            assert(0); // split configuration invalid
        }
        assert(0); // split configuration invalid
        return {};
    }

    vector<Matrix> differentialMap(PD D, vector<vector<int>> faces){
        // freopen("input.txt", "r", stdin);
        // int n; cin >> n;
        // assume edges are always 1-indexed
        // PD D = readPlanarDiagram(n);
        int n = D.size();
        // int f; cin >> f;

        vector<ll> basis1(63), basis2(63);
        ll specialMask;
        for (int i = 0; i < faces.size(); i++){
            ll mask = 0;
            for (int j = 0; j < faces[i].size(); j++){
                ll x = faces[i][j];
                mask += (1ll << (x-1));
            }
            if (!i) specialMask = mask;
            else insertVector(basis1, mask);
        }
        basis2 = basis1;
        insertVector(basis2, specialMask);

        vector<set<set<int>>> resolutionCube(1ll << n);
        for (ll i = 0; i < (1ll << n); i++){
            resolutionCube[i] = resolutionCircles(D, i);
            // test puncture detection
            // for (auto circle : resolutionCube[i]){
            //     if (containsPuncture(circle, basis1, basis2)){
            //         for (auto x : circle) cerr << x << ' ';
            //         cerr << endl;
            //     }
            // }
        }
        
        vector<ll> ordering((1ll << n));
        vector<ll> circleStartingIndex((1ll << n));

        vector<ll> bitCount(n+1, 0);
        vector<ll> basisStartCount(n+1, 0);
        for (ll i = 0; i < (1ll << n); i++){
            ordering[i] = bitCount[__builtin_popcountll(i)]++;
            circleStartingIndex[i] = basisStartCount[__builtin_popcountll(i)];
            basisStartCount[__builtin_popcountll(i)] += (1ll << resolutionCube[i].size());
        }


        vector<Matrix> differentialMap(n);
        for (int i = 0; i < n; i++){
            differentialMap[i] = Matrix(basisStartCount[i], basisStartCount[i+1]);
        }

        vector<map<set<int>, ll>> resolutionCircleIndices(1ll << n);
        vector<vector<set<int>>> resolutionCirclesVector(1ll << n);

        for (ll resolution = 0; resolution < (1ll << n); resolution++){
            ll count = 0;
            for (auto x : resolutionCube[resolution]){
                resolutionCircleIndices[resolution][x] = count++;
                resolutionCirclesVector[resolution].push_back(x);
            }
        }


        for (ll resolution = 0; resolution < (1ll << n); resolution++){
            for (int j = 0; j < n; j++){
                if ((resolution & (1ll << j)) != 0) continue; // jth bit already set
                // jth bit not yet set
                ll newResolution = resolution | (1ll << j);
                set<set<int>> oldCircles = resolutionCube[resolution];
                set<set<int>> newCircles = resolutionCube[newResolution];

                set<set<int>> oldDiff = setComplement<set<int>>(oldCircles, newCircles);
                set<set<int>> newDiff = setComplement<set<int>>(newCircles, oldCircles);

                for (ll oldCirclesSubset = 0; oldCirclesSubset < (1ll << oldCircles.size()); oldCirclesSubset++){
                    ll oldIndex = circleStartingIndex[resolution] + oldCirclesSubset;
                    // (-) <-> 0, (+) <-> 1
                    if (oldDiff.size() == 2){ // exactly 2 circles in the old resolution not in the new resolution
                        // must be a merge

                        bool circleOneStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(oldDiff.begin())])) != 0);
                        bool circleTwoStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(++oldDiff.begin())])) != 0);

                        vector<bool> newCircleStatuses = annularMerge(*oldDiff.begin(), circleOneStatus,
                        *(++oldDiff.begin()), circleTwoStatus, basis1, basis2);

                        ll newCircleStartIndex = circleStartingIndex[newResolution];
                        // update index for the new merged circle
                        for (bool newCircleStatus : newCircleStatuses){
                            ll newCircleIndex = newCircleStartIndex;
                            if (newCircleStatus) newCircleIndex += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                            for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                                if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                    if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                        newCircleIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                                    }
                                }
                            }
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                        }
                    }
                    else if (oldDiff.size() == 1){ // must be a split
                        ll newCircleStartIndex = circleStartingIndex[newResolution];
                        for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                    newCircleStartIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                                }
                            }
                        }

                        bool oldCircleStatus = oldCirclesSubset & (1ll << resolutionCircleIndices[resolution][*oldDiff.begin()]);
                        vector<pair<bool, bool>> newCircleStatuses = annularSplit(*oldDiff.begin(), oldCircleStatus,
                        *newDiff.begin(), *(++newDiff.begin()), basis1, basis2);

                        int newCircle1Index = resolutionCircleIndices[newResolution][*newDiff.begin()];
                        int newCircle2Index = resolutionCircleIndices[newResolution][*(++newDiff.begin())];
                        for (pair<bool, bool> newCircleStatus : newCircleStatuses){
                            ll newCircleIndex = newCircleStartIndex;
                            if (newCircleStatus.first) newCircleIndex += (1ll << newCircle1Index);
                            if (newCircleStatus.second) newCircleIndex += (1ll << newCircle2Index);
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                        }
                    }
                }
            }
        }

        return differentialMap;
    }
    vector<Matrix> differentialMapSubcomplex(PD D, vector<vector<int>> faces, int annularGrading){
        int n = D.size();
        // int f; cin >> f;

        vector<ll> basis1(63), basis2(63);
        ll specialMask;
        for (int i = 0; i < faces.size(); i++){
            ll mask = 0;
            for (int j = 0; j < faces[i].size(); j++){
                ll x = faces[i][j];
                mask += (1ll << (x-1));
            }
            if (!i) specialMask = mask;
            else insertVector(basis1, mask);
        }
        basis2 = basis1;
        insertVector(basis2, specialMask);

        vector<set<set<int>>> resolutionCube(1ll << n);
        for (ll i = 0; i < (1ll << n); i++){
            resolutionCube[i] = resolutionCircles(D, i);
        }
        
        vector<ll> ordering((1ll << n));
        vector<ll> circleStartingIndex((1ll << n));

        vector<ll> bitCount(n+1, 0);
        vector<ll> basisStartCount(n+1, 0);
        for (ll i = 0; i < (1ll << n); i++){
            ordering[i] = bitCount[__builtin_popcountll(i)]++;
            // if (__builtin_popcountll(i) == 3){
            //     cerr << "";
            // }
            circleStartingIndex[i] = basisStartCount[__builtin_popcountll(i)];
            basisStartCount[__builtin_popcountll(i)] += (1ll << resolutionCube[i].size());
        }


        vector<vector<vector<bool>>> differentialMap(n);
        for (int i = 0; i < n; i++){
            differentialMap[i] = vector<vector<bool>>(basisStartCount[i], vector<bool>(basisStartCount[i+1]));
        }

        vector<map<set<int>, ll>> resolutionCircleIndices(1ll << n);
        vector<vector<set<int>>> resolutionCirclesVector(1ll << n);

        for (ll resolution = 0; resolution < (1ll << n); resolution++){
            ll count = 0;
            for (auto x : resolutionCube[resolution]){
                resolutionCircleIndices[resolution][x] = count++;
                resolutionCirclesVector[resolution].push_back(x);
            }
        }

        vector<vector<int>> elementsToKeep(n+1); // for each degree, stores the column vectors to keep
        for (ll resolution = 0; resolution < (1ll << n); resolution++){
            set<set<int>> circles = resolutionCube[resolution];
            for (ll circlesSubset = 0; circlesSubset < (1ll << circles.size()); circlesSubset++){
                ll index = circleStartingIndex[resolution] + circlesSubset;
                ll count = 0;
                ll circleIndex = 0;
                for (auto circle : circles){
                    if (containsPuncture(circle, basis1, basis2)){
                        if ((1 << circleIndex) & circlesSubset) count++;
                        else count--;
                    }
                    circleIndex++;
                }
                if (count == annularGrading) elementsToKeep[__builtin_popcountll(resolution)].push_back(index);
            }
        }

        for (ll resolution = 0; resolution < (1ll << n); resolution++){
            for (int j = 0; j < n; j++){
                if ((resolution & (1ll << j)) != 0) continue; // jth bit already set
                // jth bit not yet set
                ll newResolution = resolution | (1ll << j);
                set<set<int>> oldCircles = resolutionCube[resolution];
                set<set<int>> newCircles = resolutionCube[newResolution];

                set<set<int>> oldDiff = setComplement<set<int>>(oldCircles, newCircles);
                set<set<int>> newDiff = setComplement<set<int>>(newCircles, oldCircles);

                for (ll oldCirclesSubset = 0; oldCirclesSubset < (1ll << oldCircles.size()); oldCirclesSubset++){
                    ll oldIndex = circleStartingIndex[resolution] + oldCirclesSubset;
                    // (-) <-> 0, (+) <-> 1
                    if (oldDiff.size() == 2){ // exactly 2 circles in the old resolution not in the new resolution
                        // must be a merge

                        bool circleOneStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(oldDiff.begin())])) != 0);
                        bool circleTwoStatus = ((oldCirclesSubset 
                        & (1ll << resolutionCircleIndices[resolution][*(++oldDiff.begin())])) != 0);

                        vector<bool> newCircleStatuses = annularMerge(*oldDiff.begin(), circleOneStatus,
                        *(++oldDiff.begin()), circleTwoStatus, basis1, basis2);

                        ll newCircleStartIndex = circleStartingIndex[newResolution];
                        // update index for the new merged circle
                        for (bool newCircleStatus : newCircleStatuses){
                            ll newCircleIndex = newCircleStartIndex;
                            if (newCircleStatus) newCircleIndex += (1ll << resolutionCircleIndices[newResolution][*newDiff.begin()]);
                            for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                                if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                    if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                        newCircleIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                                    }
                                }
                            }
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                        }
                    }
                    else if (oldDiff.size() == 1){ // must be a split
                        ll newCircleStartIndex = circleStartingIndex[newResolution];
                        for (ll oldCirclesIndex = 0; (ull) oldCirclesIndex < oldCircles.size(); oldCirclesIndex++){
                            if (oldCirclesSubset & (1ll << oldCirclesIndex)){
                                if (newCircles.count(resolutionCirclesVector[resolution][oldCirclesIndex])){
                                    newCircleStartIndex += (1ll << resolutionCircleIndices[newResolution][resolutionCirclesVector[resolution][oldCirclesIndex]]);
                                }
                            }
                        }

                        bool oldCircleStatus = oldCirclesSubset & (1ll << resolutionCircleIndices[resolution][*oldDiff.begin()]);
                        vector<pair<bool, bool>> newCircleStatuses = annularSplit(*oldDiff.begin(), oldCircleStatus,
                        *newDiff.begin(), *(++newDiff.begin()), basis1, basis2);

                        int newCircle1Index = resolutionCircleIndices[newResolution][*newDiff.begin()];
                        int newCircle2Index = resolutionCircleIndices[newResolution][*(++newDiff.begin())];
                        for (pair<bool, bool> newCircleStatus : newCircleStatuses){
                            ll newCircleIndex = newCircleStartIndex;
                            if (newCircleStatus.first) newCircleIndex += (1ll << newCircle1Index);
                            if (newCircleStatus.second) newCircleIndex += (1ll << newCircle2Index);
                            differentialMap[__builtin_popcountll(resolution)][oldIndex][newCircleIndex] = 1;
                        }
                    }
                }
            }
        }

        for (int i=0; i<n; i++){
            sort(elementsToKeep[i].begin(), elementsToKeep[i].end());
        }

        vector<Matrix> finalDiffMap(n);
        for (int i=0; i<n; i++){
            finalDiffMap[i] = Matrix(elementsToKeep[i].size(), elementsToKeep[i+1].size());
        }
        for (int i=0; i<n; i++){
            for (int columnIndex=0; columnIndex<finalDiffMap[i].size(); columnIndex++){
                for (int rowIndex=0; rowIndex<finalDiffMap[i][0].size(); rowIndex++){
                    finalDiffMap[i][columnIndex][rowIndex] = differentialMap[i][elementsToKeep[i][columnIndex]][elementsToKeep[i+1][rowIndex]];
                }
            }
        }

        return finalDiffMap;
    }

    vector<Matrix> planarDiagramToMaps(bool restrictAnnularGrading){
        // reads planar diagram notation and faces from input.txt and returns the differential maps
        freopen("input.txt", "r", stdin);
        int n, f, r; cin >> n >> f;
        if (restrictAnnularGrading) cin >> r;
        // assume edges are always 1-indexed
        PD D = readPlanarDiagram(n);
        vector<vector<int>> faces(f);
        for(int i=0; i<f; i++){
            ll numEdges;
            cin >> numEdges;
            for(int j=0; j<numEdges; j++){
                ll x; cin >> x;
                faces[i].push_back(x);
            }
        }
        if (restrictAnnularGrading) return annular::differentialMapSubcomplex(D, faces, r);
        return annular::differentialMap(D, faces);
    }
}

#endif
#define DIFFERENTIAL_MAPS